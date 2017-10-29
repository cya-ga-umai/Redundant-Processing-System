#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "can_lib.h"
#include "rps.h"

int main(int argc, char **argv) {
  int sock;

  if (argc < 2) {
    puts("MPU番号を入力してください");
    exit(1);
  }
  MPU_NUM = (unsigned char)atoi(argv[1]);
  // CANの初期化
  sock = can_init();

  sync(sock);

  close(sock);

  return 0;
}
