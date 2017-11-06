/**
 * @file  rps.h
 * @brief 冗長処理システム
 */

#ifndef RPS_H_INCLUDED
#define RPS_H_INCLUDED

unsigned char MPU_NUM;

void sync(void);
void sync_data(unsigned int *res_data);
unsigned char compare_result(unsigned int *result);
#endif
