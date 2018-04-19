#include "tlib.h"

void tnode_init(tnode * node)
{
	node->prenode = node;
	node->nextnode = node;
}

#define first_node 	head_node.nextnode
#define last_node 	head_node.prenode

void list_init(tlist * list)
{
	list->first_node = &(list->head_node);
	list->last_node = &(list->head_node);
}

uint32_t list_count(tlist * list)
{
	return list->count;
}

/**********************************************************************************************************
** Function name        :   tlist_first
** Descriptions         :   返回链表的首个结点
** parameters           :   list 查询的链表
** Returned value       :   首个结点，如果链表为空，则返回0
***********************************************************************************************************/
tnode * tlist_first (tlist * list)
{
	tnode * node = (tnode *)0;
	if(list->count != 0)
	{
		node = list->first_node;
	}
	return node;
}

/**********************************************************************************************************
** Function name        :   tlist_last
** Descriptions         :   返回链表的最后一个结点
** parameters           :   list 查询的链表
** Returned value       :   最后的结点，如果链表为空，则返回0
***********************************************************************************************************/
tnode * tlist_last (tlist * list)
{
	tnode * node = (tnode *)0;
	if(list->count != 0)
	{
		node = list->head_node.prenode;
	}
	return node;
}

/**********************************************************************************************************
** Function name        :   tlist_pre_node
** Descriptions         :   返回链表中指定结点的前一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   前一结点结点，如果结点没有前结点（链表为空），则返回0
***********************************************************************************************************/
tnode * tlist_pre_node (tlist * list, tnode * node)
{
	if(node->prenode == node)
	{
		return (tnode *)0;
	}
	else
	{
		return node->prenode ;
	}
}

/**********************************************************************************************************
** Function name        :   tlist_next_node
** Descriptions         :   返回链表中指定结点的后一结点
** parameters           :   list 查询的链表
** parameters           :   node 参考结点
** Returned value       :   后一结点结点，如果结点没有hou结点（链表为空），则返回0
***********************************************************************************************************/
tnode * tlist_next_node (tlist * list, tnode * node)
{
	if (node->nextnode == node)
	{
		return (tnode *)0;
	}
	else
	{
		return node->nextnode;
	}
}

/**********************************************************************************************************
** Function name        :   tListRemoveAll
** Descriptions         :   移除链表中的所有结点
** parameters           :   list 待清空的链表
** Returned value       :   无
** 思想就是从头结点开始，把头结点的下一个结点指针指向下下一个结点，然后把下一个结点的指针指向自己，最后把头结点的next和pre指针指向自己
***********************************************************************************************************/
void tlist_remove_all (tlist * list)
{
	uint32_t count ;
	tnode * next_node;
	next_node = list->first_node;
	
	for(count = list->count; count != 0; count --)
	{
		tnode * current_node;
		current_node = next_node;
		next_node = next_node->nextnode;
		
		current_node->nextnode = current_node;
		current_node->prenode = current_node;
	}
	
	list->first_node = &(list->head_node);
	list->last_node = &(list->head_node);
	list->count = 0;
	
}

/**********************************************************************************************************
** Function name        :   tlist_add_first
** Descriptions         :   将指定结点添加到链表的头部
** parameters           :   list 待插入链表
** parameters						:   node 待插入的结点
** Returned value       :   无
** 画图更好理解
***********************************************************************************************************/
void tlist_add_first (tlist * list, tnode * node)
{
	node->prenode = list->first_node->prenode;
	node->nextnode = list->first_node;
	
	list->first_node->prenode = node;
	list->first_node = node;
	
	list->count ++;
	
}

/**********************************************************************************************************
** Function name        :   tlist_add_last
** Descriptions         :   将指定结点添加到链表的末尾
** parameters           :   list 待插入链表
** parameters			:   node 待插入的结点
** Returned value       :   无
***********************************************************************************************************/
void tlist_add_last (tlist * list, tnode * node)
{
	node->nextnode = &(list->head_node);
	node->prenode = list->last_node;
	
	list->last_node->nextnode = node;
	list->last_node = node;
	list->count ++;
	
}

/**********************************************************************************************************
** Function name        :   tListRemoveFirst
** Descriptions         :   移除链表中的第1个结点
** parameters           :   list 待移除链表
** Returned value       :   如果链表为空，返回0，否则的话，返回第1个结点
***********************************************************************************************************/
tnode * tlist_remove_first (tlist * list)
{
	tnode * node;
	node = (tnode *)0;
	
	if(list->count != 0)
	{
		node = list->first_node;
		node->nextnode->prenode = &(list->head_node);
		list->first_node = node->nextnode;
		list->count --;
		
	}
	return node;
}

/**********************************************************************************************************
** Function name        :   tListInsertAfter
** Descriptions         :   将指定的结点插入到某个结点后面
** parameters           :   list 			待插入的链表
** parameters           :   node_after 		参考结点
** parameters           :   node_to_insert 	待插入的结构
** Returned value       :   无
***********************************************************************************************************/
void tlist_insert_after (tlist * list, tnode * node_after, tnode * node_to_insert)
{
	node_to_insert->prenode =  node_after;
	node_to_insert->nextnode = node_after->nextnode;
	
	node_after->nextnode->prenode = node_to_insert;
	node_after->nextnode = node_to_insert;

	list->count ++;
}

/**********************************************************************************************************
** Function name        :   tlist_remove
** Descriptions         :   将指定结点从链表中移除
** parameters           :   list 	待移除的链表
** parameters           :   node 	待移除的结点
** Returned value       :   无
***********************************************************************************************************/
void tlist_remove (tlist * list, tnode * node)
{
	node->prenode->nextnode = node->nextnode;
	node->nextnode->prenode = node->prenode;
	
	
	list->count --;
}




