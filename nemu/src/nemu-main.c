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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

word_t expr(char* e, bool* suc);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif
  // FILE* fp = fopen("/home/carottx/ics2024/nemu/tools/gen-expr/input.txt","r");
  // if(fp==NULL) puts("NO");
  // static char ex[2000];
  // static int ans;
  // while(fscanf(fp,"%d%s",&ans, ex) != EOF){
  //   printf("Error! expression = %s\n", ex);
  //   printf("ans=%u\n",ans);
  //   bool suc = true;
  //   word_t result = expr(ex, &suc);
  //   if (!suc || result != ans){
  //     printf("Error! expression = %s\n", ex);
  //     printf("ans=%u result=%u\n",ans,result);
  //   }
  // }
  // fclose(fp);

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
