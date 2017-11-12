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
  unsigned int result[3];
  unsigned char check;
  unsigned char debug_flag = 1;

  if (argc < 2) {
    puts("MPU番号を入力してください");
    exit(1);
  }
  MPU_NUM = (unsigned char)atoi(argv[1]);

  sync();

  while(1){
    puts("演算開始");
    result[MPU_NUM - 1] = fibonacci(400000000);  // そこそこ処理時間のかかる演算
    puts("演算終了");

    if((MPU_NUM == 3) && (debug_flag == 1)){
      result[MPU_NUM - 1] = 123;
      debug_flag          = 0;
    }

    sync_data(result);

    printf("MPU1: %d, MPU2: %d,MPU3: %d\n", result[0], result[1], result[2]);
  
    check = compare_result(result);

    if(!check){
      puts("全一致を確認！");
      break;
    }

    printf("MPU%d: 不一致を確認\n", check);
    puts("再演算を開始");
  }

  return 0;
}
