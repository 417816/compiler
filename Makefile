#
# Makefile ... コンパイルに使うファイルの依存関係
#

# make によるコンパイル
#
# make         ... 更新したファイルだけをコンパイル
# make clean   ... 作られたファイルを削除

# コンパイラの指定

CC      = gcc
CFLAGS  = -Wall -W              # 基本的な警告を表示
#CFLAGS += -ansi -pedantic      # 行頭の '#' を消せば ISO 規格準拠の警告を表示
#CFLAGS += -O2                  # 行頭の '#' を消せば データ流解析で誤り検査

# ファイルを作る規則

all: calculate compile

calculate: calculate.c libcprime.a
	$(CC) $(CFLAGS) $^ -o $@

compile: compile.c libcprime.a
	$(CC) $(CFLAGS) $^ -o $@

# ファイルを消す規則

clean:
	rm -f calculate compile
