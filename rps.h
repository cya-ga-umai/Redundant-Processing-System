/**
 * @file  rps.h
 * @brief 冗長処理システム
 */

#ifndef RPS_H_INCLUDED
#define RPS_H_INCLUDED

unsigned char MPU_NUM;

void sync(int socket);
void sync_data(int socket, unsigned int *res_data);
#endif
