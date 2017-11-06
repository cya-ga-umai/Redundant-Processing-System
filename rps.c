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

static void set_target_filter(int socket);

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

static void set_target_filter(int socket) {
  canid_t first_target  = 0;
  canid_t second_target = 0;

  struct can_filter rfilter[2];
  set_target(&first_target, &second_target);

  set_can_filter(&rfilter[0], first_target, CAN_SFF_MASK);
  set_can_filter(&rfilter[1], second_target, CAN_SFF_MASK);
  setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
}

void sync(void) {
  rcv_frame_t   rcv;
  pthread_t     thread_id;
  int           handle;
  int           rcv_status = 0;
  unsigned char send_data;
  unsigned char mpu_status[3] = {0};
  int           socket;

  puts("同期開始");

  socket = can_init();
  // ターゲット設定
  set_target_filter(socket);

  // 受信確認フラグを兼ねる送信データの作成
  send_data = digit_to_bit(MPU_NUM);

  rcv.socket = socket;

  while (1) {
    handle =
        pthread_create(&thread_id, NULL, (void *(*)(void *))can_read, &rcv);
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

    send_data =
        send_data |
        digit_to_bit((canid_t)rcv.frame.can_id);  // 受信確認フラグを更新

    // 受信できたものはフィルタから削除する（予定）
    mpu_status[MPU_NUM - 1]          = send_data;
    mpu_status[rcv.frame.can_id - 1] = rcv.frame.data[0];

    // 自機の受信状況と他の受信状況を確認することで、送信漏れを防ぐ
    if ((mpu_status[0] == ALL_RECEIVED) && (mpu_status[1] == ALL_RECEIVED) &&
        (mpu_status[2] == ALL_RECEIVED)) {
      puts("同期完了！");
      close(socket);
      break;
    }
  }
}

void sync_data(unsigned int *res_data) {
  rcv_frame_t   rcv;
  pthread_t     thread_id;
  int           handle;
  int           rcv_status = 0;
  unsigned char send_data[5];
  unsigned char mpu_status[3] = {0};
  int           socket;

  puts("演算結果送受信開始");
  socket = can_init();

  // ターゲット設定
  set_target_filter(socket);

  // 受信確認フラグを兼ねる送信データの作成
  send_data[0] = digit_to_bit(MPU_NUM);
  send_data[1] = (unsigned char)(res_data[MPU_NUM - 1] & 0xFF);
  send_data[2] = (unsigned char)((res_data[MPU_NUM - 1] >> 8) & 0xFF);
  send_data[3] = (unsigned char)((res_data[MPU_NUM - 1] >> 16) & 0xFF);
  send_data[4] = (unsigned char)((res_data[MPU_NUM - 1] >> 24) & 0xFF);

  rcv.socket = socket;

  while (1) {
    handle =
        pthread_create(&thread_id, NULL, (void *(*)(void *))can_read, &rcv);
    if (handle) {
      perror("pthread_create");
      exit(1);
    }

    can_send(socket, MPU_NUM, sizeof(send_data), send_data);

    pthread_join(thread_id, (void *)&rcv_status);
    if (rcv_status) {
      puts("受信タイムアウト");
      exit(1);
    }

    send_data[0] =
        send_data[0] |
        digit_to_bit((canid_t)rcv.frame.can_id);  // 受信確認フラグを更新

    res_data[rcv.frame.can_id - 1] =
        (unsigned int)((rcv.frame.data[4] << 24) | (rcv.frame.data[3] << 16) |
                       (rcv.frame.data[2] << 8) | rcv.frame.data[1]);

    mpu_status[MPU_NUM - 1]          = send_data[0];
    mpu_status[rcv.frame.can_id - 1] = rcv.frame.data[0];

    // 自機の受信状況と他の受信状況を確認することで、送信漏れを防ぐ
    if ((mpu_status[0] == ALL_RECEIVED) && (mpu_status[1] == ALL_RECEIVED) &&
        (mpu_status[2] == ALL_RECEIVED)) {
      puts("送受信完了！");
      close(socket);
      break;
    }
  }
}
