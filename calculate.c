// calculate.c ... 算術式の列を読んでPコードに変換
//
// コンパイル： gcc calculate.c libcprime.a -o calculate または make
//
// 使い方： ./calculate [-v] [TEXT_FILE]
//          -v で詳細表示，[ ] 内は省略可能

#include "cprime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//========== 補助関数：誤り処理と段付け表示 ==========

// エラーメッセージ str を表示して実行を中断
static void error(const char *str) { 
  fputs(str, stderr);
  exit(EXIT_FAILURE);
} 

// 入れ子の深さ level に応じて字下げしてから，文字列 str を表示
static void show_message(int level, const char *str) { 
  if (is_verbose()) {
    putchar('#'); print_characters('\t', level + 4); puts(str);
  }
} 

//========== 補助関数：表示機能を字句取得とコード生成に付加 ==========

// 次の字句を読んで保持し，詳細表示の場合それを表示
static void read_token(void) { 
  lookahead_token();
  if (is_verbose()) {
    printf("# "); print_token(); putchar('\n');
  }
} 

// 命令列の最後に (op, m, n) を追加し，詳細表示の場合は文字列 str を表示
static void emit_instruction(int op, int m, int n, const char *str) { 
  int successful;

  //*** 課題 1d で次の１行を除去してコード生成機能を有効にする ***

  successful = append_instruction(op, m, n);
  if (is_verbose()) {
    printf("#\t\t%s\n", str);
  }
  if (! successful) {
    error("*** code buffer is full ***\n");
  }
} 

//========== 主関数：構文解析とコード生成 ==========

// 単独の算術式 exp の構文 と 算術式の列 seq の構文
//
// exp → NUMBER | IDENT | LPAREN exp [ PLUS | TIMES ] exp RPAREN
// seq → exp* END_OF_INPUT

// 単独の算術式 exp に対するコンパイル（構文解析とコード生成）
static void compile_exp(void) { 
  static int nesting_depth = 0;   // 式の入れ子の深さ
  show_message(nesting_depth, "begin exp");
  nesting_depth++;
  
  if (token_type() == NUMBER) {
    emit_instruction(LDC, 0, token_num(), "LDC (NUMBER)");
    read_token();
  }
  else if (token_type() == IDENT) {
    int ident_value = 1;   // この課題では識別子の値は常に１
    emit_instruction(LDC, 0, ident_value, "LDC (IDENT)");
    read_token();
  }
  else if (token_type() == LPAREN) {
	read_token();
	compile_exp();	
	int aop_type;
	if (token_type() == PLUS) {
	  aop_type = 0;
	  read_token();
	} else if (token_type() == TIMES) {
	  aop_type = 2;
	  read_token();
	} else {
	  error("*** unexpected token ***\n");
	}
	compile_exp();
	emit_instruction(AOP, 0, aop_type, "AOP");
	if (token_type() != RPAREN) {
	  error("*** unexpected token ***\n");
	}
	read_token();	
  }
  else {
    error("*** unexpected token ***\n");
  }
  
  nesting_depth--;
  show_message(nesting_depth, "end exp");
} 

// 算術式の列 seq に対するコンパイル（構文解析とコード生成）
static void compile_seq(void) { 
  while (token_type() != END_OF_INPUT) { 
	compile_exp();
	emit_instruction(WLN, 0, 0, "WLN");
  }
  emit_instruction(STP, 0, 0, "STP");
  read_token();
} 

//========== プログラム本体 ==========

// コンパイラ本体
int main(int argc, char *argv[]) { 
  int is_verb = FALSE;   // 詳細表示：通常はしない
  int visible_addr;      // 目的コードの番地の表示の有無

  // オプション処理
  if (argc > 1 && strcmp(argv[1], "-v") == 0) {       // -v あり
    argc--; argv[1] = argv[0]; argv++;   // argv[1] の -v を削除
    is_verb = TRUE;
  }

  // 前処理
  open_file_or_exit(argc, argv);
  set_verbosity(is_verb);
  init_lookahead_token();
  read_token();   // 字句を常に一つ先読み

  // コンパイル処理
  compile_seq();
  visible_addr = FALSE;
  //*** 課題 1d で次の文を有効にして生成した目的コードを出力 ***
  print_code(visible_addr);

  // 後処理
  close_file();

  return EXIT_SUCCESS;
} 


