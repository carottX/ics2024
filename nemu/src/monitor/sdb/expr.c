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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_EQ,
	TK_PAR_L, TK_PAR_R,
	TK_ADD, TK_SUB, TK_MUL, TK_DIV,
	TK_NUM, TK_NOTYPE = 256
  /* TODO: Add more token types */

};

#define is_op(x) (x==TK_ADD || x==TK_SUB || x==TK_MUL || x==TK_DIV)

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},	        // plus
	{"\\*", TK_MUL},
	{"\\(", TK_PAR_L},
	{"\\)", TK_PAR_R},
	{"-", TK_SUB},
	{"/", TK_DIV},
	{"[0-9]+", TK_NUM},
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[128] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

void print(Token t){
  char *ss[] = {"==","(",")","+","-","*","/",""};
  if (t.type == TK_NOTYPE) return;
  if (t.type == TK_NUM) printf("%s",t.str);
  else printf("%s",ss[t.type]);
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case TK_NUM:
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token++].type = rules[i].token_type;
						break;
					default: 
						tokens[nr_token].type = rules[i].token_type;
						nr_token ++ ;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int find_main_op(int start, int end){
  int lowest = -1, prec = -1;
  int left_par = 0;

  for(int i = start; i<=end; ++i){
    if(tokens[i].type == TK_PAR_L) left_par ++;
    else if(tokens[i].type == TK_PAR_R) left_par--;
    if(!is_op(tokens[i].type)) continue;
    if(left_par) continue;
    if (lowest == -1){
      
      lowest = i;
      prec = (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV);
    }
    else if(prec >= (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV)){
      lowest = i;
      prec = (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV);
    }
  }
  //printf("prec=%d\n",prec);
  assert(lowest != -1);
  return lowest;
}

bool check_parentheses(int start, int end){
  int left_par = 0;
  for(int i=start;i<=end;++i){
    if(tokens[i].type == TK_PAR_L) left_par ++;
    else if(tokens[i].type == TK_PAR_R) left_par--;
    //printf("LEFT_PAR=%d\n",left_par);
    if(left_par<0) assert(0); // Doenst match!
  }
  assert(left_par==0);
  if(tokens[start].type != TK_PAR_L) return false;
  for(int i=start;i<=end;++i){
    if(tokens[i].type == TK_PAR_L) left_par ++;
    else if(tokens[i].type == TK_PAR_R) left_par--;
    if(i!=end && !left_par) return false;
  }
  return true;
}

word_t eval(int start, int end){
  printf("CHECKING:\n");
  for(int i=start;i<=end;++i) {
    print(tokens[i]);
  }
  printf("\n-----------------\n");
  if(start > end) {
    Log("Invalid expression.");
    assert(0);
  }
  else if (start == end){
    assert(tokens[start].type == TK_NUM);
    return strtol(tokens[start].str, NULL, 10);
  }
  else if (check_parentheses(start, end)){
    return eval(start+1, end-1);
  }
  else{
    int mid = find_main_op(start, end);
    int val1 = eval(start, mid-1);
    int val2 = eval(mid+1, end);
    switch (tokens[mid].type){
      case TK_ADD: return val1+val2;
      case TK_SUB: return val1-val2;
      case TK_MUL: return val1*val2;
      case TK_DIV: 
        if (val2 == 0){
          printf("DIV0 ERROR!\n");
          for(int i=0; i < nr_token; ++i){
            print(tokens[i]);
          }
          puts("");
          assert(0);
        }
        return val1/val2;
      default: assert(0); // Invalid oprand!
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  /*
	char *ss[] = {"TK_EQ","TK_PAR_L","TK_PAR_R","TK_ADD","TK_SUB","TK_MUL","TK_DIV","TK_NUM"};
	for(int i=0; i< nr_token; ++i){
    if (tokens[i].type == TK_NOTYPE) printf("NOTYPE\n");
		else printf("Token_id = %s\n", ss[tokens[i].type]);
	}*/
  return eval(0,nr_token-1);
}
