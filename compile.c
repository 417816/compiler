// compile.c ... C'プログラムを読んでPコードに変換
//
// コンパイル： gcc compile.c libcprime.a -o compile または make
//
// 使い方： ./compile [-v] [C'_PROGRAM_FILE]
//          -v で詳細表示，[ ] 内は省略可能

#include "cprime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//===== 記号表 =====

/*----- 公開する定数 -----*/

enum IdentType { VARI, FUNC, ARGU };
char *identtype[] =  { "VAR", "FUN", "ARG" };
enum IdentRenge { GLOBAL, LOCAL };
char *identrenge[] = { "GLOBAL", "LOCAL" };

typedef struct ident_info {
  char name[100];
  int address;
  int type;
  int renge;
} IDENT_INFO;

IDENT_INFO ident[100];
int xst_address;
int ident_num = 0;
int xst_num = 0;
//int ident_lnum = 3;
//int sym_num = 0;

//----- 登録 -----
static void registration(char *name, int address, int type, int renge){
  strcpy(ident[ident_num].name, name);
  ident[ident_num].address = address; 
  ident[ident_num].type = type;
  ident[ident_num].renge = renge;
  ident_num++;
}

//----- 一覧表示 -----
static void display(void){
  int i = 0;
  printf("name, address, type, renge\n");
  while (strcmp(ident[i].name, "\0") != 0) {
	printf("%s, %d, %s, %s\n", ident[i].name, ident[i].address, identtype[ident[i].type], identrenge[ident[i].renge]);
	i++;
  } 
}
//----- 検索 -----
static IDENT_INFO search(char *name){
  int i;
  for (i = ident_num - 1; i >= 0 ; i--) {
	if (strcmp(ident[i].name, name) == 0) {
	  return ident[i];
	}
  }
  printf("Cannot find %s.", name);
  exit(1);
}

//----- 無効化 -----
static void Invalidation(void){
  int i, n = ident_num;
  for (i = 0; i < n ; i++) {
	if (ident[i].renge == LOCAL) {
	  strcpy(ident[i].name, "\0");
	  ident_num--;
	}
  }
}

//===== 補助関数：誤り処理，段付け表示 =====

static void exit_parser(const char *message) {
  fprintf(stderr, "*** ERROR : %s ***", message);
  fprintf(stderr, " line=%d, column=%d\n", token_row(), token_col());
  exit(EXIT_FAILURE);   // 強制終了
} 

static void show_message(const char *str) {
  if (is_verbose()) {
    printf("#\t\t\t\t%s\n", str);
  }
} 

//========== 補助関数：字句取得，字句照合，構文解析初期化 ==========

static void read_token(void) {
  lookahead_token();
  if (is_verbose()) {
    printf("# "); print_token(); putchar('\n');
  }
}


static void match_token(int type, const char *str) {
  if (token_type() != type) {
    exit_parser(str);
  }
  read_token();
}

static void init_parse(void) {
  init_lookahead_token();
  read_token();   // 構文解析の各関数は字句を常に一つ先読みする
}

//========== 補助関数：コード生成 ==========

static void emit_instruction(int op, int m, int n, const char *str) { 
  int successful = append_instruction(op, m, n);
  if (is_verbose()) {
    printf("#\t\t%s\n", str);
  }
  if (! successful) {
    exit_parser("code buffer is full");
  }
} 

//========== 補助関数：構文解析 ==========

static void expression(void);
// expression() は，先に定義される atom() から呼ばれる

//----- 原子式 atom の解析 -----

static void atom(void) {
  show_message("begin atom");
  if (token_type() == NUMBER) {
    emit_instruction(LDC, 0, token_num(), "LDC (NUMBER)");
    read_token();
  }
  else if (token_type() == LPAREN) {
    read_token();
    expression();
    match_token(RPAREN, "')' not found");
  }
  else if (token_type() == IDENT) {
	int is_input = (strcmp(token_id(), "input") == 0);
	char ident_id[100] = "\0";
	strcpy(ident_id, token_id());
	read_token();
	int arg_num = 0;
	if(token_type() == LPAREN){
	  read_token();
	  if (token_type() != RPAREN) { 
		if (is_input == 0) { emit_instruction(MST, 0, 0, "MST"); }
		expression();
		arg_num++;
		while (token_type() == COMMA) {
		  read_token();
		  expression();
		  arg_num++;
		}
	  } 
	  match_token(RPAREN, "')' not found");	
	  if (is_input) { emit_instruction(RLN, 0, 0, "RLN"); }
	  else{ emit_instruction(CAL, arg_num, search(ident_id).address,"CAL"); }
	}
	else emit_instruction(LOD, search(ident_id).renge, search(ident_id).address, "LOD");
  }
  else {
    exit_parser("atomic expression not found");
  }
  show_message("end atom");
}

//----- 因子 factor の解析 -----

static void factor(void) {
  int op;
  show_message("begin factor");
  op = token_type();
  if (op == PLUS || op == MINUS) {
	if (op == MINUS) {
	  emit_instruction(LDC, 0, 0, "LDC");
	}
    read_token();
  }
  atom();
  if (op == MINUS) emit_instruction(AOP, 0, op-2, "AOP");
  show_message("end factor");
}

//----- 項 term の解析 -----

static void term(void) {
  int op;
  show_message("begin term");
  factor();
  op = token_type();
  while (op == TIMES || op == DIV || op == MOD) {
    read_token();
    factor();
	emit_instruction(AOP, 0, op-2, "AOP");
    op = token_type();
  }
  show_message("end term");
}

//----- 算術式 arithmetic の解析 -----

static void arithmetic(void) {
  int op;
  show_message("begin arithmetic");
  term();
  op = token_type();
  while (op == PLUS || op == MINUS) {
    read_token();
    term();
	emit_instruction(AOP, 0, op-2, "AOP");
    op = token_type();
  }
  show_message("end arithmetic");
}

//----- 比較 comparison の解析 -----

static void comparison(void) {
  int op;
  show_message("begin comparison");
  arithmetic();
  op = token_type();
  while (op == GT || op == GEQ || op == LT || op == LEQ || op == EQ || op == NEQ ) {
    read_token();
    arithmetic();
	emit_instruction(COP, 0, op-7, "COP");
	op = token_type();
  }
  show_message("end comparison");
}

//----- 式 expression の解析 -----

static void expression(void) {
  int op;
  show_message("begin expression");
  comparison();
  op = token_type();
  while (op == EQ || op == NEQ) {
    read_token();
    comparison();
    op = token_type();
	//emit_instruction(COP, 0, , "COP");
  }
  show_message("end expression");
}

//----- 文 statement の解析 -----

static void statement(void) {
  show_message("begin statement"); 
  if (token_type() == IDENT) {
    int is_output = (strcmp(token_id(), "output") == 0);
	char ident_id[100] = "\0";
	strcpy(ident_id, token_id());
    read_token();
	int arg_num = 0;
	if (token_type() == LPAREN){
	  read_token();
	  if (token_type() != RPAREN) {
		if (is_output == 0) { emit_instruction(MST, 0, 0, "MST"); }
		expression();
		arg_num++;
		while (token_type() == COMMA) {
		  read_token();
		  expression();
		  arg_num++;
		}
	  } 
      match_token(RPAREN, "')' not found");
	  if (is_output) { emit_instruction(WLN, 0, 0, "WLN"); }
	  else{ emit_instruction(CAL, arg_num, search(ident_id).address,"CAL"); }
	}
	else {
	  match_token(BECOMES, "'=' not found");
	  expression();
	  emit_instruction(STR, search(ident_id).renge, search(ident_id).address, "STR");
      //exit_parser("'output' not found");
    }
    match_token(SEMICOLON, "';' not found");
  }
  else if (token_type() == IF) {
	read_token();
	match_token(LPAREN, "'(' not found");
	expression();
	match_token(RPAREN, "')' not found");
	emit_instruction(FJP, 0, -1, "FJP");
	int fjp_address = get_code_address();
	statement();
	rewrite_instruction(fjp_address, get_code_address() + 1);
	if (token_type() == ELSE) {
	  emit_instruction(UJP, 0, -1, "UJP");
	  rewrite_instruction(fjp_address, get_code_address() + 1);
	  int ujp_address = get_code_address();
	  read_token();
	  statement();
	  rewrite_instruction(ujp_address, get_code_address() + 1);
	}
  }
  else if (token_type() == WHILE) {
	int ujp_jp_address = get_code_address() + 1;
	read_token();
	match_token(LPAREN, "'(' not found");
	expression();
	match_token(RPAREN, "')' not found");
	emit_instruction(FJP, 0, -1, "FJP");
	int fjp_address = get_code_address();
	statement();
	emit_instruction(UJP, 0, ujp_jp_address, "UJP");
	rewrite_instruction(fjp_address, get_code_address() + 1);
  }
  else if (token_type() == LBRACE) {
  	read_token();
	while (token_type() != RBRACE) {
	  statement();
	}
	read_token();
  }
  else if (token_type() == RETURN) {
	read_token();
	if (token_type() != SEMICOLON) { 
	  expression(); 
	  emit_instruction(STR, LOCAL, 0, "STR");
	}
	match_token(SEMICOLON, "';' not found");
	emit_instruction(RET, 0, xst_address - 1, "RET");
  }
  show_message("end statement");
}

//----- 変数宣言 declaration の解析 -----

static void declaration(int ident_address, int ident_renge) {
  show_message("begin declaration");
  int dec_num = 0;
  if (token_type() == IDENT) {
	registration(token_id(), ident_address, VARI, ident_renge);
	ident_address++;
	dec_num++;
	read_token();
  } 
  else exit_parser("identifier not found");
  while (token_type() == COMMA) {
	read_token();
	if (token_type() == IDENT) {
	  registration(token_id(), ident_address, VARI, ident_renge);
	  ident_address++;
	  dec_num++;
	  read_token();
	} 
	else exit_parser("identifier not found");
  }
  if (ident_renge == GLOBAL) { xst_num = dec_num; }
  rewrite_instruction(xst_address, dec_num);
  match_token(SEMICOLON, "';' not found");
  show_message("end declaration");
}

//----- 関数定義 definition の解析 -----

static void definition(void) {
  show_message("begin definition");
  emit_instruction(XST, 0, 0, "XST");
  xst_address = get_code_address();
  int def_type = token_type();
  if (def_type != VOID && def_type != INT) {
	exit_parser("'void' or 'int' not found");
  }
  read_token();
  if (token_type() == IDENT) {
	registration(token_id(), get_code_address(), FUNC, GLOBAL);
	read_token();
  }
  else{
	exit_parser("identifier not found");
  }
  match_token(LPAREN, "'(' not found");
  int ident_address = 3; 
  if (token_type() == IDENT) {
	registration(token_id(), ident_address, ARGU, LOCAL);
	ident_address++;
	read_token();
  } 
  while (token_type() == COMMA) {
	read_token();
	if (token_type() == IDENT) {
	  registration(token_id(), ident_address, ARGU, LOCAL);
	  ident_address++;
	  read_token();
	} 
	else printf("identifier not found");
  }
  match_token(RPAREN, "')' not found");
  match_token(LBRACE, "'{' not found");
  if (token_type() == VAR) {
	read_token();
	declaration(ident_address, LOCAL);
  }
  while (token_type() != RBRACE) {
    statement();
  }
  if (def_type == VOID) { emit_instruction(RET, 0, 0, "RET"); }
  else if (def_type == INT) { emit_instruction(RET, 0, 1, "RET"); }
  read_token();

  //記号表の一覧表示
  // display();
  // printf("\n");
  Invalidation();
  show_message("end definition");
}

//----- プログラム program の解析 -----

static void program(void) {
  show_message("begin program");
  if (token_type() == VAR) {
	read_token();
	declaration(0, GLOBAL);
  }
  int ujp_address;
  emit_instruction(XST, 0, xst_num, "XST");
  emit_instruction(UJP, 0, -1, "UJP");
  ujp_address = get_code_address();
  while (token_type() != END_OF_INPUT) { 
	if (token_type() == FUN) {
	  read_token();
	  definition();
	}
	else {
	  exit_parser("'fun' not found");
	}
  }
  read_token();
  emit_instruction(MST, 0, 0, "MST");
  rewrite_instruction(ujp_address, get_code_address());
  emit_instruction(CAL, 0, search("main").address,"CAL");
  emit_instruction(XST, 0, - xst_num, "XST");
  show_message("end program");
  emit_instruction(STP, 0, 0, "STP");
}

//========== 主関数： C'プログラム全体の構文解析 ==========

int main (int argc, char *argv[]) {
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
  init_parse();

  // コンパイル処理
  program();
  visible_addr = FALSE;

  //記号表の一覧表示
  if (is_verb == TRUE) { display(); }

 //生成した目的コードを出力 ***
  print_code(visible_addr);

  // 後処理
  close_file();

  return EXIT_SUCCESS;
} 
