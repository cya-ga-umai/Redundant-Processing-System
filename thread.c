#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#define THREAD_NUM 3
unsigned int fibonacci(unsigned long *n);
unsigned int check_data(unsigned int *res);


int main(void)
{
    unsigned int res[THREAD_NUM];
    pthread_t th_id[THREAD_NUM];
    int st, i;
    unsigned int check;
    unsigned long num = 2;

    /* スレッドの生成 */
    for(i = 0; i < THREAD_NUM; i++){
        st = pthread_create(&th_id[i], NULL, (void *(*)(void *))fibonacci, &num);
        if (st != 0) return 1;
        printf("スレッド%d 作成。\n", i);
    }

    /* スレッドの終了を待つ */
    for(i = 0; i < THREAD_NUM; i++){
        pthread_join(th_id[i], (void **)&res[i]);
        printf("スレッド%d 終了。\n", i + 1);
    }

    /* 結果表示 */
    for(i = 0; i < THREAD_NUM; i++){
        printf("thread%d = %d\n", i + 1, res[i]);
    }
    check = check_data(res);
    printf("結果 = %d\n", check);
    

    return 0;
}

/* フィボナッチ数の下二桁を求める関数 */
unsigned int fibonacci(unsigned long *n)
{
    unsigned long i;
    unsigned int x, xn, xn1;
    x = 0;
    xn = 1;
    xn1 = 0;
    if(*n == 0) return 0;
    if(*n == 1) return 1;
    for(i = 0; i < *n; i++){
        xn1 = x + xn;
        x = xn;
        xn = xn1;
        
    }
    return xn1;
}

unsigned int check_data(unsigned int *res)
{
    /* int i, j; */
    /* for(i = 0; i < THREAD_NUM; i++){ */
    /*     for(j = THREAD_NUM - 1; j >= 0; i--); */
    /* } */
    if (res[0] == res[1]){
        if (res[0] == res[2]) return 0;
        else return 3;
    } else if (res[1] == res[2]){
        return 1;
    } else {
        return 2;
    }
}
