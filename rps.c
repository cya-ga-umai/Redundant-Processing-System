/**
 * @file  rps.c
 * @brief 冗長処理システム本体
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/if.h>

#include "can_lib.h"
#include "rps.h"

#define MPU_NUM 1

#define ALL_RECEIVED 0x07
#define BIT_FLAG_0 0x01
#define BIT_FLAG_1 0x02
#define BIT_FLAG_2 0x04
#define BIT_FLAG_3 0x08
#define BIT_FLAG_4 0x10
#define BIT_FLAG_5 0x20
#define BIT_FLAG_6 0x40
#define BIT_FLAG_7 0x80

/**
 * @brief 桁数ビット変換関数
 *
 * 桁数に対応したビットにフラグを立てます。
 *
 * @param[in] digit   求めるビットの桁
 * @return            対応したビットにフラグを立てる
 */
static unsigned char digit_to_bit(const unsigned int digit);

/**
 * @brief ターゲット指定関数
 *
 * 他のRaspberry PiのCAN IDをターゲットに設定します。
 *
 * @param[out] first_target   ひとつ目のターゲット
 * @param[out] second_target  ふたつ目のターゲット
 */
static void set_target(canid_t *first_target, canid_t *second_target);
static void sync_status(unsigned int flag);

// 桁数ビット変換関数
static unsigned char digit_to_bit(const unsigned int digit) {
  switch (digit) {
    case 1:
      return BIT_FLAG_0;
    case 2:
      return BIT_FLAG_1;
    case 3:
      return BIT_FLAG_2;
    case 4:
      return BIT_FLAG_3;
    case 5:
      return BIT_FLAG_4;
    case 6:
      return BIT_FLAG_5;
    case 7:
      return BIT_FLAG_6;
    case 8:
      return BIT_FLAG_7;
    default:
      return 0;
  }
}

// ターゲット指定関数
static void set_target(canid_t *first_target, canid_t *second_target) {
  switch (MPU_NUM) {
    case 1:
      *first_target  = 0x02;
      *second_target = 0x03;
      break;
    case 2:
      *first_target  = 0x01;
      *second_target = 0x03;
      break;
    case 3:
      *first_target  = 0x01;
      *second_target = 0x02;
      break;
  }
}

// 受信状況表示関数
static void sync_status(unsigned int flag) {
  for (unsigned int i = 1; i <= 3; i++) {
    printf("ID %d: ", i);
    if (flag & digit_to_bit(i)) {
      printf("\x1b[32m");  //　文字を緑色に
      printf("OK!!\n");
    } else {
      printf("\x1b[31m");  //　文字を赤色に
      printf("NG\n");
    }
    printf("\x1b[0m");  //　文字をデフォルト色に
  }
}

void sync(int socket) {
  rcv_frame_t       rcv;
  pthread_t         thread_id;
  struct can_filter rfilter[2];
  int               handle;
  int               rcv_status = 0;
  unsigned char     send_data;
  canid_t           first_target, second_target;

  puts("同期開始");

  // ターゲット設定
  set_target(&first_target, &second_target);

  // フィルタ
  set_can_filter(&rfilter[0], first_target, CAN_SFF_MASK);
  set_can_filter(&rfilter[1], second_target, CAN_SFF_MASK);
  setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

  // 受信確認フラグを兼ねる送信データの作成
  send_data = digit_to_bit(MPU_NUM);

  rcv.socket = socket;

  while(1){
    handle = pthread_create(&thread_id, NULL, (void *(*)(void *))can_read, &rcv);
    if (handle) {
      perror("pthread_create");
      exit(1);
    }
  
    can_send(socket, MPU_NUM, 1, &send_data);
  
    pthread_join(thread_id, (void *)&rcv_status);
    if (rcv_status) {
      puts("受信タイムアウト");
      exit(1);
    }

    send_data = send_data |
                digit_to_bit((canid_t)rcv.frame.can_id);  // 受信確認フラグを更新
  
    // 受信できたものはフィルタから削除する（予定）
  
    sync_status(send_data);

    // 自機の受信状況と他の受信状況を確認することで、送信漏れを防ぐ
    if ((send_data == ALL_RECEIVED) && (rcv.frame.data[0] == ALL_RECEIVED)){
      puts("同期完了！");
      break;
    }
  }
}
