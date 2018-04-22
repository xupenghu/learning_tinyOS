#include "tlib.h"


//位图数据结构源文件
//位图数据结构初始化
void tbitmap_init(tbitmap * bitmap)
{
	bitmap->bitmap = 0;
}

//获取位图最大位数，当前只有一个uint32_t的变量，所以直接返回32
uint32_t tbitmap_pos_count()
{
	return 32;
}

//将位图某1位置1
void tbitmap_setbit(tbitmap * bitmap, uint32_t pos)
{
	bitmap->bitmap |= (1<<pos);
}

//将位图某1位清零
void tbitmap_clearbit(tbitmap * bitmap, uint32_t pos)
{
	bitmap->bitmap &= ~(1<<pos);
}


//返回位图中第一个位是1的那个位置，从0开始
uint32_t tbitmap_get_first_set(tbitmap * bitmap)
{
	static const uint8_t quick_find_table[] =     
	{
	    /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};
	
	if(bitmap->bitmap&0xff)
	{
		return quick_find_table[(bitmap->bitmap)&0xff];
	}
	else if (bitmap->bitmap&0xff00)
	{
		return quick_find_table[(bitmap->bitmap >>8)& 0xff] + 8;
	}
	else if (bitmap->bitmap&0xff0000)
	{
		return quick_find_table[(bitmap->bitmap >>16)& 0xff] + 16;
	}
	else if (bitmap->bitmap&0xff000000)
	{
		return quick_find_table[(bitmap->bitmap >>24)& 0xff] + 24;
	}
	else 
	{
		return tbitmap_pos_count();
	}
	
}




