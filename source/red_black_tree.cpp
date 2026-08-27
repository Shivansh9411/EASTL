#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// The tree insert and erase functions below are based on the original 
// HP STL tree functions. Use of these functions was been approved by
// EA legal on November 4, 2005 and the approval documentation is available
// from the EASTL maintainer or from the EA legal deparatment on request.
// 
// Copyright (c) 1994
// Hewlett-Packard Company
// 
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation. Hewlett-Packard Company makes no
// representations about the suitability of this software for any
// purpose. It is provided "as is" without express or implied warranty.
///////////////////////////////////////////////////////////////////////////////




#include <EASTL/internal/config.h>
#include <EASTL/internal/red_black_tree.h>
#include <stddef.h>



namespace eastl
{
	// Forward declarations
	rbtree_node_base* RBTreeRotateLeft(rbtree_node_base* pNode, rbtree_node_base* pNodeRoot);
	rbtree_node_base* RBTreeRotateRight(rbtree_node_base* pNode, rbtree_node_base* pNodeRoot);



	/// RBTreeIncrement
	/// Returns the next item in a sorted red-black tree.
	///
	EASTL_API rbtree_node_base* RBTreeIncrement(const rbtree_node_base* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// RBTreeIncrement
	/// Returns the previous item in a sorted red-black tree.
	///
	EASTL_API rbtree_node_base* RBTreeDecrement(const rbtree_node_base* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// RBTreeGetBlackCount
	/// Counts the number of black nodes in an red-black tree, from pNode down to the given bottom node.  
	/// We don't count red nodes because red-black trees don't really care about
	/// red node counts; it is black node counts that are significant in the 
	/// maintenance of a balanced tree.
	///
	EASTL_API size_t RBTreeGetBlackCount(const rbtree_node_base* pNodeTop, const rbtree_node_base* pNodeBottom)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// RBTreeRotateLeft
	/// Does a left rotation about the given node. 
	/// If you want to understand tree rotation, any book on algorithms will
	/// discuss the topic in detail.
	///
	rbtree_node_base* RBTreeRotateLeft(rbtree_node_base* pNode, rbtree_node_base* pNodeRoot)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// RBTreeRotateRight
	/// Does a right rotation about the given node. 
	/// If you want to understand tree rotation, any book on algorithms will
	/// discuss the topic in detail.
	///
	rbtree_node_base* RBTreeRotateRight(rbtree_node_base* pNode, rbtree_node_base* pNodeRoot)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	/// RBTreeInsert
	/// Insert a node into the tree and rebalance the tree as a result of the 
	/// disturbance the node introduced.
	///
	EASTL_API void RBTreeInsert(rbtree_node_base* pNode,
								rbtree_node_base* pNodeParent, 
								rbtree_node_base* pNodeAnchor,
								RBTreeSide insertionSide)
	{
    __builtin_trap() /* STUB: not implemented */;
} // RBTreeInsert




	/// RBTreeErase
	/// Erase a node from the tree.
	///
	EASTL_API void RBTreeErase(rbtree_node_base* pNode, rbtree_node_base* pNodeAnchor)
	{
    __builtin_trap() /* STUB: not implemented */;
} // RBTreeErase



} // namespace eastl
























