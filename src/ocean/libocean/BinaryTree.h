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
 * Binary Search Trees are data structures that support many dynamic-set
 * operations including SEARCH, MINIMUM, MAXIMUM, PREDECESSOR, SUCCESSOR,
 * INSERT, and DELETE. Thus, a search tree can be used both as a dictionary
 * and as a priority queue.
 * Basic operations on a binary search tree take time proportional to the
 * hight of the tree. For a complete binary tree with n nodes, such
 * operations run in O(ln n) worst-case time. If the tree is a linear chain
 * of n nodes, however, the same operations take O(n) worst-case time.
 * A binary search tree can be represented by a linked data structure in
 * which each node is an object. In addition to a "key" field, each node
 * contains fields "left", "right", "p", and "treeMemberMode" that point
 * to the nodes corresponding to its left, its right, and its parent,
 * respectively and tells if the node belongs to the tree or not. If a
 * child or the parent is missing, the appropriate field contains the
 * node NULL. The "root" node is the only node in the tree whose parent
 * field is NULL.
 * The keys in a binary search tree are always stored in such a way as
 * to satisfy the binary-search-tree property:
 *
 *   Let x be a node in a binary search tree. If y is a node in the left
 *   subtree of x, then key[y] <= key[x]. If y is a node in the right
 *   subtree of x, then key[x] <= key[y].
 *
 * The binary-search-tree property allows us to print out all the keys
 * in a binary search tree in sorted order by a simple recursive
 * algorithm, called an inorderTreeWalk. This algorithm derives its name
 * from the fact that the key of the root of a subtree is printed between
 * the values in its left subtree and those in its right subtree.
 *
 *        HOW TO USE IT !!!
 *
 * Two classes are defined: a virtual class called BinaryNode and a BinartTree
 * class.
 * The BinaryNode class defines a node element in the tree which has the fields
 * left, right, p, treeMemberMode, and key. The first four fields are private
 * members. The key field is of void pointer type and must be defined by
 * user via 'setKey (void*)' function. The user must, also, define the virtual
 * functions 'compare (void*)' and 'printKey (void*)'.
 * The BinaryTree class constructs the binary search tree. To create a new
 * tree first a nilNode must be created and then 'new BinaryTree(nilNode)'
 * will do the job. Use 'insert' function to put a node in the tree or
 * 'delete' function to remove it. Besides the functions enumerated
 * earlier for manipulating the data, a 'treeSize ()' and a 'treeHeight ()'
 * functions are provided for getting the size of the tree (total
 * number of nodes) and the height of the tree respectively.
 *
 * See also: T.H. Cormen, C.E. Leiserson, nd R.L. Rivest "Introduction to
 * Algorithms", The MIT press 1989
 *
 *          EXAMPLE:
 *
 * #include "src/ocean/libocean/BinaryTree.h"
 *
 * class myNode: public BinaryNode {
 *  private:
 *    int myKey;
 *  public:
 *    myNode (int k) { myKey = k; setKey((void *)&myKey);}
 *    int getmyKey() { return (*(int*)getKey()); }
 *    const int compare(void* );
 *    friend class myTree;
 * };
 *
 * class myTree: public BinaryTree {
 *  public:
 *    myTree (myNode* node):BinaryTree ((myNode*)node) {}
 *    myNode* getRoot () { return (myNode*)BinaryTree::getRoot(); }
 *    myNode* myInsert (myNode* node)
 *    {
 *       return (myNode*)treeInsert((myNode*)node);
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
 *        return (myNode*)treeDelete ((myNode*)node);
 *    }
 * };
 */

#ifndef BinaryTree_h
#define BinaryTree_h

class BinaryTree;

typedef enum { nodeExist, nodeDoesNotExist } ErrorMode;

typedef enum { notTreeMember, treeMember } TreeMemberMode;

class BinaryNode
{
private:
   BinaryNode *left, *right, *p;
   void* key;
   TreeMemberMode treeMemberMode;   // if node belongs to a tree
   BinaryNode* getLeft () { return left; }
   void setLeft (BinaryNode*);
   BinaryNode* getRight () { return right; }
   void setRight (BinaryNode*);
   BinaryNode* getP () { return p; }
   void setP (BinaryNode*);
   void setNode ();
   TreeMemberMode getMode () { return treeMemberMode; }
   void setMode (TreeMemberMode);
// friend void treeError (const char*);
public:
   BinaryNode () { setNode(); }
   virtual ~BinaryNode() {}
   void setKey (void*);
   void* getKey () { return key; }
   virtual const int compare (void*)=0;
   virtual void printKey (void*)=0;
   friend class BinaryTree;
   friend class RedBlackNode;
   friend class RedBlackTree;
};

class BinaryTree
{
private:
   BinaryNode* root;
   BinaryNode* nilNode;
   ErrorMode error;            // node already exist
   int tree_size, subtree_size;
   int maxHeight;
   void setRoot ( BinaryNode* );
   BinaryNode* insert (BinaryNode*, BinaryNode*);
   BinaryNode* search (BinaryNode*, void*);
   void subtreeSize (BinaryNode*);
   void getHeight (BinaryNode*, int);
// friend void treeError (const char*);
   BinaryNode* getNil ();
   void setNil (BinaryNode*);
   void setError (ErrorMode=nodeDoesNotExist);
   ErrorMode getError ();
   void deleteTreeNode (BinaryNode*);
public:
   BinaryTree (BinaryNode* node) { setNil (node); }
   ~BinaryTree ();
   BinaryNode* getRoot ();
   void inorderTreeWalk (BinaryNode*);
   BinaryNode* treeSearch (BinaryNode*);
   BinaryNode* treeMin (BinaryNode*);
   BinaryNode* treeMax (BinaryNode*);
   BinaryNode* treeSucc (BinaryNode*);
   BinaryNode* treePred (BinaryNode*);
   BinaryNode* treeInsert (BinaryNode*);
   BinaryNode* treeDelete (BinaryNode*);
   int treeSize ();
   int treeSize (BinaryNode*);
   int treeHeight ();
   int treeHeight (BinaryNode*);
   friend class RedBlackTree;
};

#endif
