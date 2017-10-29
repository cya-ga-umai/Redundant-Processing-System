#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "can_lib.h"
#include "rps.h"

unsigned int fibonacci(unsigned long n);

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
  int sock;
  unsigned int result;

  if (argc < 2) {
    puts("MPU番号を入力してください");
    exit(1);
  }
  MPU_NUM = (unsigned char)atoi(argv[1]);

  // CANの初期化
  sock = can_init();

  sync(sock);

  puts("演算開始");
  result = fibonacci(400000000);  // そこそこ処理時間のかかる演算
  puts("演算終了");

  close(sock);

  return 0;
}
