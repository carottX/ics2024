/***************************************************************************************
Copyright* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void WP_display();
struct WP* new_wp(char* exp);
void free_wp(int id);
uint8_t* guest_to_host(paddr_t gaddr);

#define RING_MAX_SIZE 20

struct iringbuf{
  char buf[RING_MAX_SIZE][128];
  int read, write;
}rbuf;

bool ring_is_full(){
  return (rbuf.write+1)%RING_MAX_SIZE == rbuf.read;
}
void add_inst(const char* s){
  strcpy(rbuf.buf[rbuf.write], s);
  if(ring_is_full()) rbuf.read = (rbuf.read+1)%RING_MAX_SIZE;
  rbuf.write = (rbuf.write+1)%RING_MAX_SIZE;
}

void output_ring(){
  #ifdef CONFIG_ITRACE
  printf("Past %d instructions:\n", RING_MAX_SIZE);
  for(int i=rbuf.read; i!=rbuf.write; i=(i+1)%RING_MAX_SIZE){
    if((i+1)%RING_MAX_SIZE == rbuf.write) printf("--> ");
    else printf("    ");
    printf("%s\n",rbuf.buf[i]);
  }
  #endif
}

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
//  cpu_exec(0);
  nemu_state.state = NEMU_END;
  return -1;
}

static int cmd_si(char *args){
	char* arg = strtok(NULL, " ");
	if (arg == NULL){
		cpu_exec(1);		
 	}	
	else{   
		int cmd_cnt = strtol(arg, NULL, 10);
		cpu_exec(cmd_cnt);
	}		
	return 0;
}

static int cmd_info(char *args){
	char* arg = strtok(NULL, " ");
	if (arg == NULL){
		printf("Please specify which to print!\n");
	}
	else{
		if (arg[0] == 'r'){
			isa_reg_display();
		}
		else{
			WP_display();
		}
	} 
	return 0;
}

static int cmd_x(char *args){
	char* arg1 = strtok(NULL, " ");
	if (arg1 == NULL) {
		printf("Missing Arguments\n");
		return 0;
	}
	char* arg2 = strtok(NULL, " ");
	if (arg2 == NULL) {
		printf("Missing Arguments\n");
		return 0;
	}
	int byte_cnt = strtol(arg1, NULL, 10);
	paddr_t addr_id = strtol(arg2, NULL, 16);
	if (addr_id < 0x80000000) {
		printf("Invalid memory address.\n");
		return 0;
	}
	 for(int i=0;i<byte_cnt;++i){ 
		word_t dt = *(uint32_t*) guest_to_host(addr_id+i);
		printf("%-10x:%x\n",addr_id+i,dt);
	}
	return 0;
}

static int cmd_p(char *args){
  bool success = false;
  printf("Result=%u\n",expr(args, &success));
  return !success;
}

static int cmd_w(char *args){
  new_wp(args);
  return 0;
}

static int cmd_d(char *args){
  char* arg1 = strtok(NULL, " ");
	if (arg1 == NULL) {
		printf("Missing Arguments\n");
		return 0;
	}
  int num = strtol(arg1, NULL, 10);
  free_wp(num);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single execute of the program", cmd_si},
	{ "info", "Print the status of registers or watchpoints", cmd_info},
	{ "x", "Query about consective N bytes of memory", cmd_x},
  { "p", "Calculate expression.", cmd_p},
  { "w", "Add watchpoints", cmd_w},
  { "d", "Delete watchpoints", cmd_d}
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  rbuf.read = 0;
  rbuf.write = 0;
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
