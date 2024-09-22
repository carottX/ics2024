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

uint8_t* guest_to_host(paddr_t gaddr);

enum {
  TK_EQ,
	TK_PAR_L, TK_PAR_R,
	TK_ADD, TK_SUB, TK_MUL, TK_DIV,
	TK_NUM, TK_NEQ, TK_REG, TK_HEX, TK_AND,
  TK_DEREF, TK_NEG,
  TK_MOD, TK_BITAND, TK_NOT_MORE, TK_LESS,
  TK_NOT, TK_BITOR,
  TK_NOTYPE = 256
  /* TODO: Add more token types */

};

#define is_op(x) (x==TK_ADD || x==TK_SUB || x==TK_MUL || x==TK_DIV || x == TK_AND || x == TK_NEQ || x == TK_EQ || x == TK_MOD || x==TK_BITAND || x==TK_LESS || x==TK_NOT_MORE || x==TK_BITOR)

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE}, 
  {"\\*", TK_MUL},   
  {"\\+", TK_ADD},	        // plus
  {"\\-", TK_SUB},
  {"%", TK_MOD},
  {"&", TK_BITAND},
  {"<=", TK_NOT_MORE},
  {"<", TK_LESS},
  {"==", TK_EQ},
  {"!=", TK_NEQ},
  {"!", TK_NOT},
  {"&&", TK_AND},
	{"/", TK_DIV},
  {"\\|", TK_BITOR},
  {"(\\$\\w+)|(\\$\\$0)", TK_REG},
  {"0x[0-9]+", TK_HEX},
  {"[0-9]+", TK_NUM},
  {"\\(", TK_PAR_L},
	{"\\)", TK_PAR_R}
        // equal
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
  char str[128];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

void print(Token t){
  char *ss[] = {"==","(",")","+","-","*","/","", "!=","","","&&","*","-"};
  if (t.type == TK_NOTYPE) return;
  if (t.type == TK_NUM || t.type == TK_HEX || t.type == TK_REG) printf("%s",t.str);
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
            tokens[nr_token].str[substr_len] = '\0';
						tokens[nr_token++].type = rules[i].token_type;
						break;
          case TK_REG:
            strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
            tokens[nr_token].str[substr_len-1] = '\0';
						tokens[nr_token++].type = rules[i].token_type;
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
						tokens[nr_token++].type = rules[i].token_type;
            break;
          case TK_NOTYPE:
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
      if(e[position]>='a' && e[position]<='z') {
        if(e[position+1] >= 'a' && e[position+1]<='z') assert(0);
        if(e[position+1] >= '0' && e[position+1]<='9') assert(0);
        if(position && e[position-1] == '0' ) {
          if(position>=2 && is_op(e[position-2])) assert(0);
          assert(0);
        }
        assert(0);
      }
      return false;
    }
  }

  return true;
}

inline int get_prec(int type){
  if(type == TK_MUL || type == TK_DIV || type == TK_MOD) return 5;
  if(type == TK_ADD || type == TK_SUB) return 4;
  if(type == TK_AND) return 3;
  if(type == TK_BITAND || type == TK_BITOR) return 2;
  if(type == TK_EQ || type == TK_NEQ || type == TK_LESS || type == TK_NOT_MORE) return 1; 
  assert(0);
  return 0;
}

int find_main_op(int start, int end, bool* success){
  int lowest = -1, prec = -1;
  int left_par = 0;

  for(int i = start; i<=end; ++i){
    if(tokens[i].type == TK_PAR_L) left_par ++;
    else if(tokens[i].type == TK_PAR_R) left_par--;
    if(!is_op(tokens[i].type)) continue;
    if(left_par) continue;
    if (lowest == -1){
      lowest = i;
      prec = get_prec(tokens[i].type);
    }
    else if(prec >= get_prec(tokens[i].type)){
      lowest = i;
      prec = get_prec(tokens[i].type);
    }
  }
  *success = (lowest!=-1);
  // if(success) printf("Find main operator success!\n");
  return lowest;
}

bool check_parentheses(int start, int end){
  int left_par = 0;
  for(int i=start;i<=end;++i){
    if(tokens[i].type == TK_PAR_L) left_par ++;
    else if(tokens[i].type == TK_PAR_R) left_par--;
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
  // printf("CHECKING: start=%d end=%d\n",start,end);
  // for(int i=start;i<=end;++i) {
  //   printf("i=%d,",i);print(tokens[i]);
  // }
  // printf("\n-----------------\n");
  if(tokens[start].type == TK_NOTYPE) return eval(start+1, end);
  if(tokens[end].type == TK_NOTYPE) return eval(start, end-1);
  if(start > end) {
    Log("Invalid expression.");
    assert(0);
  }
  else if (start == end){
    assert(tokens[start].type == TK_NUM || tokens[start].type == TK_REG || tokens[start].type == TK_HEX);
    if(tokens[start].type == TK_NUM) return strtol(tokens[start].str, NULL, 10);
    else if(tokens[start].type == TK_HEX) return strtol(tokens[start].str, NULL, 16);
    else {
      bool suc = true;
      int ret_val = isa_reg_str2val(tokens[start].str, &suc);
      if (!suc) {
        printf("Invalid register name!\n");
        assert(0);
      }
      return ret_val;
    }
  }
  else if (check_parentheses(start, end)){
    return eval(start+1, end-1);
  }
  else{
    bool success = true;
    int mid = find_main_op(start, end, &success);
    if (success){
      word_t val1 = eval(start, mid-1);
      word_t val2 = eval(mid+1, end);
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
        case TK_EQ: return val1 == val2;
        case TK_NEQ: return val1 != val2;
        case TK_AND: return val1 && val2;
        case TK_MOD: return val1 % val2;
        case TK_LESS: return val1 < val2;
        case TK_NOT_MORE: return val1 <= val2;
        case TK_BITAND: return val1&val2;
        case TK_BITOR: return val1 | val2;
        // case 
        default: assert(0); // Invalid oprand!
      }
    }
    else{
      assert(tokens[start].type == TK_DEREF || tokens[start].type == TK_NEG || tokens[start].type == TK_NOT);
      if(tokens[start].type == TK_DEREF){
        word_t val = eval(start+1, end);
        if (val < 0x80000000) {
          printf("Invalid memory address: %u\n", val);
          return 0;
        }
        return 	*(uint32_t*) guest_to_host(val);
      }
      else if(tokens[start].type == TK_NOT){
        word_t val = eval(start+1, end);
        return !val;
      }
      else{
        word_t val = eval(start+1, end);
        return -val;
      }
    }
  }
}

word_t expr(char *e, bool *success) {
  char* new_e = (char *)malloc( strlen(e) * sizeof(char)  + 5);
  int size = 0;
  for(int i=0;e[i];++i){
    if(e[i]!=' ') new_e[size++] = e[i];
  }
  new_e[size] = '\0';
  if (!make_token(new_e)) {
    free(new_e);

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
  for(int i=0; i < nr_token; ++i){
    if(tokens[i].type == TK_MUL && (i==0 || is_op(tokens[i-1].type) || tokens[i-1].type == TK_NEG || tokens[i-1].type == TK_DEREF || tokens[i-1].type == TK_PAR_L)){
      tokens[i].type = TK_DEREF;
    }
    else if(tokens[i].type == TK_SUB && (i==0 || is_op(tokens[i-1].type) || tokens[i-1].type == TK_NEG || tokens[i-1].type == TK_DEREF || tokens[i-1].type == TK_PAR_L)){
      tokens[i].type = TK_NEG;
    }
  }
  *success = true;
  free(new_e);
  return eval(0,nr_token-1);
}

