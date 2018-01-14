#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "can_lib.h"
#include "rps.h"

void         log_print(const char *txt, ...);
unsigned int fibonacci(unsigned long n);

void log_print(const char *txt, ...) {
  time_t     timer;
  struct tm *date;
  char       str[256];
  FILE *     fp;

  fp = fopen("msg.log", "a");
  if (fp == NULL) {
    puts("ファイルオープンに失敗");
    exit(1);
  }
  timer = time(NULL);
  date  = localtime(&timer);
  strftime(str, sizeof(str), "[%Y/%m/%d %H:%M:%S] ", date);

  strcat(str, txt);
  fputs(str, fp);
  fputs("\n", fp);
  puts(str);
  fclose(fp);
}

unsigned int fibonacci(unsigned long n) {
  unsigned int x, xn, xn2;
  x   = 0;
  xn  = 1;
  xn2 = 1;

  if (n == 0) return 0;
  if (n == 1) return 1;

  for (unsigned long i = 1; i < n; i++) {
    xn2 = (x + xn) % 100;
    x   = xn;
    xn  = xn2;
  }
  return xn2;
}

int main(int argc, char **argv) {
  unsigned int  result[3];
  unsigned char check;
  unsigned char debug_flag = 1;
  char          str[256];

  if (argc < 2) {
    puts("MPU番号を入力してください");
    exit(1);
  }
  MPU_NUM = (unsigned char)atoi(argv[1]);

  log_print("同期開始");
  sync();
  log_print("同期完了！");

  log_print("演算開始");

  while (1) {
    result[MPU_NUM - 1] = fibonacci(400000000);  // そこそこ処理時間のかかる演算
    log_print("演算終了");

    if ((MPU_NUM == 3) && (debug_flag == 1)) {
      result[MPU_NUM - 1] = 123;
      debug_flag          = 0;
    }

    log_print("演算結果の同期開始");
    sync_data(result);
    log_print("演算結果の同期完了！");

    sprintf(str, "MPU1: %d, MPU2: %d,MPU3: %d", result[0], result[1],
            result[2]);
    log_print(str);

    check = compare_result(result);

    if (!check) {
      log_print("全一致を確認！");
      break;
    }

    sprintf(str, "MPU%dの不一致を確認", check);
    log_print(str);
    log_print("再演算を開始");
  }

  log_print("終了");
  return 0;
}
