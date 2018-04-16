#ifndef __TLIB__H
#define __TLIB__H
#include <stdint.h>

//本文件包含了所有此os用到的数据结构

/*位图数据结构部分*/

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





/*双向链表数据结构部分*/
typedef struct _tnode
{
	struct _tnode * prenode;
	struct _tnode * nextnode;
}tnode;

typedef struct _tlist
{
	tnode head_node;
	uint32_t count;
	
}tlist;

#define offsetof(TYPE, MEMBER)  ((int)&(((TYPE *)0)->MEMBER))
	/** 
 * container_of - cast a member of a structure out to the containing structure 
 * @ptr:    the pointer to the member. 指向成员的指针,也就是一个地址值
 * @type:   the type of the container struct this is embedded in.  需要返回的结构实例类型。
 * @member: the name of the member within the struct. 成员在结构实例内部的名称，如果为数组，需要指定下标。
 * 
 */  
#define container_of(ptr, type, member) ({         \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);\
    (type *)( (char *)__mptr - offsetof(type,member) );}) 

void tnode_init(tnode * node);

void list_init(tlist * list);

uint32_t list_count(tlist * list);

/**********************************************************************************************************
** Function name        :   tlist_first
** Descriptions         :   返回链表的首个结点
** parameters           :   list 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
***********************************************************************************************************/
tnode * tlist_first (tlist * list);

/**********************************************************************************************************
** Function name        :   tlist_last
** Descriptions         :   返回链表的最后一个结点
** parameters           :   list 查询的链表
** Returned value       :   最后的结点，如果链表为空，则返回0
***********************************************************************************************************/
tnode * tlist_last (tlist * list);

/**********************************************************************************************************
** Function name        :   tlist_pre_node
** Descriptions         :   返回链表中指定结点的前一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   前一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
tnode * tlist_pre_node (tlist * list, tnode * node);

/**********************************************************************************************************
** Function name        :   tlist_next_node
** Descriptions         :   返回链表中指定结点的后一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   后一结点结点，如果结点没有hou结点（链表为空），则返回0
***********************************************************************************************************/
tnode * tlist_next_node (tlist * list, tnode * node);

/**********************************************************************************************************
** Function name        :   tListRemoveAll
** Descriptions         :   移除链表中的所有结点
** parameters           :   list 待清空的链表
** Returned value       :   无
** 思想就是从头结点开始，把头结点的下一个结点指针指向下下一个结点，然后把下一个结点的指针指向自己，最后把头结点的next和pre指针指向自己
***********************************************************************************************************/
void tlist_remove_all (tlist * list);
/**********************************************************************************************************
** Function name        :   tlist_add_first
** Descriptions         :   将指定结点添加到链表的头部
** parameters           :   list 待插入链表
** parameters						:   node 待插入的结点
** Returned value       :   无
** 画图更好理解
***********************************************************************************************************/
void tlist_add_first (tlist * list, tnode * node);

/**********************************************************************************************************
** Function name        :   tlist_add_last
** Descriptions         :   将指定结点添加到链表的末尾
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void tlist_add_last (tlist * list, tnode * node);

/**********************************************************************************************************
** Function name        :   tListRemoveFirst
** Descriptions         :   移除链表中的第1个结点
** parameters           :   list 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
***********************************************************************************************************/
tnode * tlist_remove_first (tlist * list);

/**********************************************************************************************************
** Function name        :   tListInsertAfter
** Descriptions         :   将指定的结点插入到某个结点后面
** parameters           :   list 			待插入的链表
** parameters           :   node_after 		参考结点
** parameters           :   node_to_insert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void tlist_insert_after (tlist * list, tnode * node_after, tnode * node_to_insert);

/**********************************************************************************************************
** Function name        :   tlist_remove
** Descriptions         :   将指定结点从链表中移除
** parameters           :   list 	待移除的链表
** parameters           :   node 	待移除的结点
** Returned value       :   无
***********************************************************************************************************/
void tlist_remove (tlist * list, tnode * node);







#endif

