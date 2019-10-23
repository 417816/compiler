/**
 **  cprime.h ... CPrime コンパイラ作成用ライブラリ
 **/

#ifndef _CPRIME_H_INCLUDED_
#define _CPRIME_H_INCLUDED_

/*========== 汎用関数群 ==========*/

#include <stdio.h>

/*----- 公開する定数 -----*/

enum Boolean {
  TRUE  = (0 == 0),
  FALSE = (0 == 1)
};

enum ErrorStatus {
  SUCCESS = TRUE,
  FAILURE = FALSE
};

enum Array {
  EOA = -1   /* 配列の末尾 (番兵) */
};

/*----- 公開する関数 -----*/

void open_file_or_exit(int argc, char *argv[]);
/* コマンドでファイルの指定があればそれを開き，なければ標準入力を使う
 * ファイルを開けなければ強制終了
 */
FILE *get_file_pointer(void);
/* 使用中のファイルへのポインタを返す
 */
void close_file(void);
/* 使用中のファイルを閉じる
 */

void set_verbosity(int is_verb);
/* 詳細表示の有無を is_verb の TRUE/FALSE で設定（標準では FALSE）
 */
int is_verbose(void);
/* 詳細表示の有無を TRUE/FALSE で返す
 */

void print_characters(char c, int n);
/* 文字 c を n 回表示
 */

/*========== 字句解析（予約語・数・識別子・特殊記号 の判別） ==========*/

/*----- 公開するデータ型 -----*/

typedef struct Token Token;

/*----- 公開する定数 -----*/

enum TokenType {
  NUMBER,    IDENT,
  PLUS,      MINUS,     TIMES,     DIV,       MOD,
  GT,        GEQ,       LT,        LEQ,       EQ,        NEQ,
  BECOMES,
  LPAREN,    RPAREN,    LBRACE,    RBRACE,
  COMMA,     SEMICOLON,
  IF,        ELSE,      WHILE,     RETURN,
  VAR,       FUN,
  VOID,      INT,
  END_OF_INPUT,
  ILLEGAL_TOKEN   /* エラー処理用 */
};

/*----- 公開する関数 -----*/

int token_type(void);
/* 現在の字句の種類を返す
 */
int token_num(void);
/* 現在の字句が表す数値を返す
 */
char *token_id(void);
/* 現在の字句が表す識別子を返す
 * ポインタが指す識別子は更新され得ることに注意
 */
int token_row(void);
/* 現在の字句が原始ファイルの先頭から何行目かを返す
 */
int token_col(void);
/* 現在の字句が原始ファイルの行頭から何文字かを返す
 */

void print_token(void);
/* 現在の字句を表示
 */

void init_lookahead_token(void);
/* 字句解析の初期化（lookahead_token() を最初に呼ぶ前に実行）
 */
int lookahead_token(void);
/* 次の字句を読み，現在の字句を更新し，字句の種類を返す
 */

/*========== P コードの 読み込み・生成・格納 ==========*/

/*----- 公開するデータ型 -----*/

struct Instruction {
  int type;
  int arg1;
  int arg2;
};

typedef struct Instruction Instruction;

/*----- 公開する定数 -----*/

enum InstructionType {
  ERR = -1,   /* エラー処理用 */
  LDC, LOD, LDA, ILD,
  STR, ISR,
  AOP, COP,
  WLN, RLN,
  STP, UJP, FJP,
  XST, MST, CAL, RET
};

enum {
  UNKNOWN_ADDRESS = -1
};

/*----- 公開する関数 -----*/

int get_code_address(void);
/* 格納中の最後の命令の番地を返す
 */
int append_instruction(int type, int arg1, int arg2);
/* 命令 (type, arg1, arg2) を最後に追加し
 * エラーの有無を SUCCESS/FAILURE で返す
 */
int rewrite_instruction(int code_addr, int jump_addr);
/* code_addr 番地に格納中の命令 (type, arg1, arg2) の arg2 を jump_addr に変え
 * エラーの有無を SUCCESS/FAILURE で返す
 */
Instruction *get_instruction(int addr);
/* addr 番地に格納中の命令へのポインタを返す（無効な番地なら NULL を返す）
 */

void print_address(int addr);
/* 番地 addr を表示
 */
void print_instruction(const Instruction *insp);
/* 命令 *insp を表示
 */
void print_code(int visible_addr);
/* 格納中の命令列を表示
 * visible_addr が TRUE なら番地も表示
 */

void read_code_or_exit(void);
/* 使用中のファイルを読んで命令列を格納
 * エラーがあれば強制終了
 */

#endif /* _CPRIME_H_INCLUDED_ */
