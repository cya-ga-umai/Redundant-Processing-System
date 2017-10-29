#include <stdio.h>
#include <unistd.h>

#include "can_lib.h"
#include "rps.h"

int main(void) {
  int sock;
  // CANの初期化
  sock = can_init();

  sync(sock);

  close(sock);

  return 0;
}
