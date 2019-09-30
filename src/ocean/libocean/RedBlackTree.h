/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Viorica Simion
 *	Simon de Graaf
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * A red-black tree is a binary search tree with one extra bit of storage
 * per node: its color, which can be either RED or BLACK. By constraining
 * the way nodes can be colored on any path from the root to a leaf, red-black
 * trees ensure that no such path is more than twice as long as any other,
 * so that the tree is approximately balanced.
 *
 * Each node of the tree now contains the fields color, key, left, right, p,
 * and treeMemberMode. If a child or the parent of a node does not exist,
 * the corresponding pointer of the node contains the value NULL.
 *
 * A binary search tree is a red-black tree if it satisfies the following
 * red-black properties:
 *  1. Every node is either red or black.
 *  2. Every leaf (NULL) is black.
 *  3. If a node is red, then both its children are black.
 *  4. Every simple path from a node to a descendant leaf contains the
 *     same number of black nodes.
 *  Lemma:
 *         A red-black tree with n internal nodes has height at most
 *         2lg(n+1).
 *
 *
 * See also: T.H. Cormen, C.E. Leiserson, nd R.L. Rivest "Introduction to
 * Algorithms", The MIT press 1989
 *
 *
 *
 *       HOW TO USE IT !!!
 *
 * Two classes, RedBlackNode and RedBlackTree, are derived from the basic
 * binary search tree classes BinaryNode and BinaryTree. Use it similar
 * with the binary search tree.
 *
 *         EXAMPLE:
 *
 * #include "src/ocean/libocean/RedBlackTree.h"
 *
 * class myNode: public RedBlackNode {
 *  private:
 *    int myKey;
 *  public:
 *    myNode ();
 *    myNode (int);
 *    ~myNode () {}
 *    void setKey (int);
 *    int getmyKey() { return (*(int*)getKey()); }
 *    const int compare(void*);
 *    void printKey(void*);
 *    friend class myTree;
 * };
 *
 *    myNode::myNode():RedBlackNode() { myKey = NULL; }
 *
 *    myNode::myNode(int k):RedBlackNode()
 *    {
 *       setKey(k);
 *    }
 *
 *    void myNode::setKey (int k)
 *    {
 *       myKey = k; RedBlackNode::setKey((void *)&myKey);
 *    }
 *
 *     Define, also, 'compare(void*)' and 'printKey(void*)' function.
 *
 * class myTree: public RedBlackTree {
 *  public:
 *    myTree ( myNode*);
 *    myNode* getRoot () { return (myNode*)RedBlackTree::getRoot(); }
 *    myNode* myInsert (myNode* node)
 *    {
 *       return (myNode*)rbInsert((myNode*)node);
 *    }
 *     myNode* mySearch (myNode* node)
 *    {
 *       return (myNode*)treeSearch ((myNode*)node);
 *    }
 *    myNode* myMin (myNode* node)
 *    {
 *       return (myNode*)treeMin ((myNode*)node);
 *    }
 *    myNode* myMax (myNode* node)
 *    {
 *       return (myNode*)treeMax ((myNode*)node);
 *    }
 *     myNode* mySucc (myNode* node)
 *    {
 *       return (myNode*)treeSucc ((myNode*)node);
 *    }
 *    myNode* myPred (myNode* node)
 *    {
 *        return (myNode*)treePred ((myNode*)node);
 *    }
 *    myNode* myDelete (myNode* node)
 *    {
 *        return (myNode*)rbDelete ((myNode*)node);
 *    }
 * };
 */

#ifndef RedBlackTree_h
#define RedBlackTree_h

#include "src/ocean/libocean/BinaryTree.h"

#ifndef NULL
#define NULL 0
#endif

class RedBlackTree;

typedef enum { Black, Red } Color;

class RedBlackNode: public BinaryNode {
private:
   Color color;
   RedBlackNode* left();
   void left (RedBlackNode*);
   RedBlackNode* right ();
   void right (RedBlackNode*);
   RedBlackNode* p ();
   void p (RedBlackNode*);
public:
   RedBlackNode ();
   ~RedBlackNode () {}
   friend class RedBlackTree;
};

class RedBlackTree: private BinaryTree {
private:
   void setRoot (RedBlackNode*);
   void leftRotate (RedBlackNode*);
   void rightRotate (RedBlackNode*);
   RedBlackNode* insert (RedBlackNode*, RedBlackNode*);
   void rbDeleteFixup (RedBlackNode*);
   RedBlackNode* getNil ();
   void setNil (RedBlackNode*);
   void deleteTreeNode (RedBlackNode*);
public:
   RedBlackTree (RedBlackNode*);
   ~RedBlackTree () {}
   RedBlackNode* rbInsert (RedBlackNode*);
   RedBlackNode* rbDelete  (RedBlackNode*);
   RedBlackNode* getRoot ();
   void inorderTreeWalk (RedBlackNode*);
   RedBlackNode* treeSearch (RedBlackNode*);
   RedBlackNode* treeMin (RedBlackNode*);
   RedBlackNode* treeMax (RedBlackNode*);
   RedBlackNode* treeSucc (RedBlackNode*);
   RedBlackNode* treePred (RedBlackNode*);
   int treeSize ();
   int treeSize (RedBlackNode*);
   int treeHeight ();
   int treeHeight (RedBlackNode*);
};

#endif
