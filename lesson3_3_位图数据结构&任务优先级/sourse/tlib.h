#ifndef __TLIB__H
#define __TLIB__H
#include <stdint.h>

//本文件包含了所有此os用到的数据结构

typedef struct _tbitmap
{
	uint32_t bitmap;
}tbitmap;

//位图数据结构初始化
void tbitmap_init(tbitmap * bitmap);

//获取位图最大位数，当前只有一个uint32_t的变量，所以直接返回32
uint32_t tbitmap_pos_count(void);

//将位图某1位置1
void tbitmap_setbit(tbitmap * bitmap, uint32_t pos);

//将位图某1位清零
void tbitmap_clearbit(tbitmap * bitmap, uint32_t pos);


//返回位图中第一个位是1的那个位置，从0开始
uint32_t tbitmap_get_first_set(tbitmap * bitmap);



#endif

