# Makefile

# プログラム名とオブジェクトファイル名
PROGRAM = rps
OBJS = rps.o can_lib.o sample.o

# コンパイラの定義
CC = gcc
CFLAGS = -Wall -O2 -std=c11 -pthread

# 拡張子の定義
.SUFFIXES: .c .o

# プライマリターゲット
$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS)

# サフィックスルール
.c.o:
	$(CC) $(CFLAGS) -c $<

# 依存ヘッダファイルの設定
sample.o: can_lib.h
sample.o: rps.h
rps.o: can_lib.h
rps.o: rps.h
can_lib.o: can_lib.h

# ファイル削除ターゲット
.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)
