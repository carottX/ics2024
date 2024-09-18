/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char str[128];
  word_t val;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].str[0] = '\0';
    wp_pool[i].val = 0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char* exp){
  if(free_ == NULL) {
    printf("No free watchpoints available!\n");
    assert(0);
  }
  free_ -> next = head;
  head = free_;
  free_ = free_ -> next;
  strcpy(head->str, exp);
  bool suc = true;
  head->val = expr(exp, &suc);
  if(!suc){
    panic("Wrong Expression!");
  }
  return head;
}

void free_wp(WP* wp){
  WP* now;
  WP* pre = NULL;
  for(now = head; now != NULL; now = now->next){
    if (now == wp){
      if (pre != NULL) {
        pre -> next = now -> next;
        now -> next = free_;
        return;
      }
      else{
        head = head -> next;
        now -> next = free_;
        return;
      }
    }
    pre = now;
  }
  printf("No such watchpoints!\n");
  assert(0);
}