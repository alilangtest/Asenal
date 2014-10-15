/***************************************************************************
 * 
 * Copyright (c) 2008 Baidu.com, Inc. All Rights Reserved
 * $Id: dlist.cpp,v 1.2 2009/03/25 12:34:11 yingxiang Exp $ 
 * 
 **************************************************************************/
 
 
 
/**
 * @file dlist.cpp
 * @author xiaowei(xiaowei@baidu.com)
 * @date 2008/03/10 14:27:16
 * @version $Revision: 1.2 $ 
 * @brief 
 *  
 **/
#include <stdio.h>

#include "dlist.h"
#include <assert.h>

namespace comspace
{

dlist_t *dlist_insert(dlist_t *root, dlist_t *node)
{
	node->next = root;
	root = node;
	return root;
}

dlist_t *dlist_erase(dlist_t *root, dlist_t *node)
{
	dlist_t *front = root;
	dlist_t *now = root;
	if (root == node) {
		return root->next;
	}

	now = now->next;
	while (now) {
		if (node == now) {
			front->next = now->next;
			break;
		}
		front = front->next;
		now = now->next;
	}

	return root;
}

int dlist_walk(dlist_t *root, void (*fun)(void *))
{
	dlist_t *node = root;
	while (root != NULL) {
		root = root->next;
		fun(node);
	}
	return 0;
}

};
/* vim: set ts=4 sw=4 sts=4 tw=100 */
