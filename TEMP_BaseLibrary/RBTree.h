/* RBTree.h -- Implementation of Red-Black binary tree.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _RBTREE_
#define  _RBTREE_

//************************************************************************************
//*                                                                                  *
//*  Developed By:  Kurt A. Kramer                                                   *
//*                                                                                  *
//*  Date:          October-26-2003                                                  *
//************************************************************************************
//*                                                                                  *
//*  Utilized algorithms from Introduction to Algorithms  by  Cormen.                *
//*                                                                                  *
//************************************************************************************


#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <memory>

#include "BasicTypes.h"

namespace  KKU
{
  typedef  enum  {Red, Black}  RBcolor;

  template <class Entry>
  class  RBnode
  {
    typedef  Entry*   EntryPtr;

    typedef  RBnode*  RBnodePtr;

  public:
    RBnode (RBnodePtr   _parent,
            RBnodePtr   _left,
            RBnodePtr   _right,
            RBcolor     _color,
            EntryPtr    _entry
           );

    RBcolor    Color  ()  {return  color;}
    EntryPtr   Data   ()  {return  entry;}
    RBnodePtr  Left   ()  {return  left;}
    RBnodePtr  Parent ()  {return  parent;}
    RBnodePtr  Right  ()  {return  right;}
  
    void  Color  (RBcolor   _color)   {color  = _color;}
    void  Data   (EntryPtr  _data)    {entry  = _data;}
    void  Left   (RBnodePtr _left)    {left   = _left;}
    void  Parent (RBnodePtr _parent)  {parent = _parent;}
    void  Right  (RBnodePtr _right)   {right  = _right;}

  private:
    RBnodePtr  parent;
    RBnodePtr  left;
    RBnodePtr  right;
    EntryPtr   entry;
    RBcolor    color;
  };  /* RBnode */


  template <class Entry,class CompareNodes,typename KeyType>
  class  RBTree;

  template <class Entry,class CompareNodes,typename KeyType>
  class  Iterator
  {
  public:
    typedef  RBTree<Entry,CompareNodes,KeyType>*  TreePtr;
  
    typedef  Entry*          EntryPtr;
   
    typedef  RBnode<Entry>*  NodePtr;
  
    friend  class  RBTree<Entry,CompareNodes,KeyType>;
  
    Iterator (TreePtr  _tree);
  
    ~Iterator ();
  
    EntryPtr  GetEqual   (const KeyType&  key);
    
    EntryPtr  GetGreater (const KeyType&  key);
  
    EntryPtr  GetGreaterOrEqual (const KeyType&  key);
  
    EntryPtr  GetFirst ();
  
    EntryPtr  GetLast ();
  
    EntryPtr  GetNext ();
  
    EntryPtr  GetPrev ();
  
  private:
    void     IsTreeStillThere ();

    void     TreeHasBeenDeleted ();

    void     DeletionOfNode (NodePtr  n);

    void     InsertionOfNode (NodePtr  n);

    NodePtr  prevNode;
    NodePtr  curNode;
    NodePtr  nextNode;

    TreePtr  tree;
  };  /* Iterator */


  template <class Entry,class CompareNodes,typename KeyType>
  class  RBTree
  {
  public:
      typedef  Entry*          EntryPtr;
      typedef  RBnode<Entry>   Node;
      typedef  Node*           NodePtr;

      typedef  KKU::int32      int32;
      typedef  KKU::uint32     uint32;

      typedef  RBTree<Entry,CompareNodes,KeyType>  Tree;

      typedef  Tree*  TreePtr;

      friend  class  Iterator<Entry,CompareNodes,KeyType>;

      RBTree (CompareNodes&  _comparator,
              bool           _owner
             );

      RBTree (RBTree&  tree);

      ~RBTree ();

      uint32  Size ()  {return  size;}

      void      WalkTree ()  {WalkTree (root);}

      void      CalcTreeStats ();

      bool      CompareToTree (TreePtr  tree);
      
      void      DeleteCurrentNode ();

      void      DeleteEntry (EntryPtr  e);

      EntryPtr  GetEqual   (const KeyType&  key);

      EntryPtr  GetFirst ();

      EntryPtr  GetGreater (const KeyType&  key);

      EntryPtr  GetGreaterOrEqual (const KeyType&  key);

      EntryPtr  GetLess (const KeyType&  key);

      EntryPtr  GetLast ();

      EntryPtr  GetNext ()       {return  Successor ();}

      EntryPtr  GetPrev ()       {return  Predeccessor ();}

      EntryPtr  Predeccessor ();
 
      NodePtr   RBInsert (EntryPtr  e);

      EntryPtr  Successor ();

      bool      Validate ();

      typedef  Iterator<Entry,CompareNodes,KeyType>*  IteratorPtr;

  private:
      void     CheckNodeStats (NodePtr  n,
                               int32&     numOfNodes,
                               int32&     numOfLeafs,
                               int32*     leafPaths,
                               int32&     shortestPath,
                               int32&     longestPath,
                               int32      curDeapth
                              );

      NodePtr  CloneSubTree (NodePtr  s,
                             NodePtr  sNil,
                             NodePtr  dParent
                            );

      bool     AreSubTreesTheSame (NodePtr  n1,
                                   NodePtr  n1NIL,
                                   NodePtr  n2,
                                   NodePtr  n2NIL
                                  );

      int32      CountNodesInSubTree (NodePtr  subTree);

      NodePtr  Delete (NodePtr  node2Delete);

      void     DeleteSubTree (NodePtr  treeRoot);

      NodePtr  GetEqual (NodePtr         subTree,
                         const KeyType&  key
                        );

      NodePtr  GetGreater (NodePtr         subTree,
                           const KeyType&  key
                          );

      NodePtr  GetGreaterOrEqual (NodePtr         subTree,
                                  const KeyType&  key
                                 );

      NodePtr  GetLess    (NodePtr         subTree,
                           const KeyType&  key
                          );

      void     IteratorDelete (IteratorPtr  newIterator);

      void     IteratorAdd    (IteratorPtr  newIterator);

      NodePtr  Insert (EntryPtr  _entry);

      NodePtr  LeftRotate (NodePtr  x);

      NodePtr  Maximum (NodePtr  n);

      NodePtr  Minimum () {return  Minimum (root);}

      NodePtr  Minimum (NodePtr  n);

      NodePtr  Predeccessor (NodePtr  n);

      NodePtr  RBDelete (NodePtr z);

      void     RBDeleteFixUp (NodePtr x);

      NodePtr  RightRotate (NodePtr  x);

      NodePtr  SearchForEntry (NodePtr  n,   // Will walk the subtree pointed to by 'n' and 
                               Entry*   e    // return the node that is pointing to 'e'.
                              );

      NodePtr  Successor (NodePtr  n);

      int32    ValidateSubTree (NodePtr      subTree,
                                NodePtr      theParent,
                                const char*  linkDescription,
                                int32        blackNodeCount,
                                int32&       blackNodeHeight
                               );

      void     WalkTree (NodePtr  n);

      CompareNodes&  comparator;

      Node*          curNode;

      Node*          nil;

      bool           owner;   /**< if True this instance owns the objects it contains and is responsible for deleting them when it is deleted.  */

      Node*          root;

      uint32         size;


      int32          numOfIterators;
      IteratorPtr*   iterators;
  };  /* RBTree */
}  /* KKU */



template <class Entry>
KKU::RBnode<Entry>::RBnode (RBnodePtr  _parent,
                            RBnodePtr  _left,
                            RBnodePtr  _right,
                            RBcolor    _color,
                            EntryPtr   _entry
                           ):
     parent (_parent),
     left   (_left),
     right  (_right),
     entry  (_entry),
     color  (_color)
{}




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBTree<Entry,CompareNodes,KeyType>::RBTree (CompareNodes&  _comparator,
                                                 bool           _owner
                                                ):
   comparator    (_comparator),
   curNode       (NULL),
   nil           (NULL),
   owner         (_owner),
   root          (NULL),
   size          (0),
   numOfIterators(0),
   iterators     (NULL)
{
  nil = new RBnode<Entry> (NULL, NULL, NULL, Black, NULL);
  root    = nil;
  curNode = nil;
}




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBTree<Entry,CompareNodes,KeyType>::RBTree (RBTree&  tree):

   comparator    (tree.comparator),
   size          (tree.size),
   owner         (true),
   root          (NULL),
   nil           (NULL),
   numOfIterators(0),
   iterators     (NULL)
{
  nil = new RBnode<Entry> (NULL, NULL, NULL, Black, NULL);
  root = CloneSubTree (tree.root, tree.nil, nil);
 
  int32  x = CountNodesInSubTree (root);

  curNode = nil;
}




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::CloneSubTree (NodePtr  s,
                                                                            NodePtr  sNil,
                                                                            NodePtr  dParent
                                                                           )
{
  if  (s == sNil)
    return  nil;

  EntryPtr entry = new Entry (*(s->Data ()));

  NodePtr  newNode = new Node (dParent,
                               nil,
                               nil,
                               s->Color (),
                               entry
                              );

  NodePtr  leftSubTree  = CloneSubTree (s->Left  (), sNil, newNode);
  NodePtr  rightSubTree = CloneSubTree (s->Right (), sNil, newNode);

  newNode->Left  (leftSubTree);
  newNode->Right (rightSubTree);

  return  newNode;
}  /* CloneSubTree */



template <class Entry,class CompareNodes,typename KeyType>
KKU::RBTree<Entry,CompareNodes,KeyType>::~RBTree ()
{
  if  (root != nil)
    DeleteSubTree (root);


  for  (int32 i = 0;  i < numOfIterators;  i++)
  {
     iterators[i]->TreeHasBeenDeleted ();
  }

  delete  nil;
}




template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::DeleteSubTree (NodePtr  treeRoot)
{
  if  (treeRoot == nil)
    return;

  if  (treeRoot->Left () != nil)
    DeleteSubTree (treeRoot->Left ());

  if  (treeRoot->Right () != nil)
    DeleteSubTree (treeRoot->Right ());

  if  (owner)
    delete  treeRoot->Data ();

  delete  treeRoot;
}  /* DeleteSubTree */




template <class Entry,class CompareNodes,typename KeyType>
bool  KKU::RBTree<Entry,CompareNodes,KeyType>::CompareToTree (TreePtr  tree)
{
 return  AreSubTreesTheSame (root, nil, tree->root, tree->nil);
}  /* CompareToTree */




template <class Entry,class CompareNodes,typename KeyType>
bool  KKU::RBTree<Entry,CompareNodes,KeyType>::AreSubTreesTheSame (NodePtr  n1,
                                                                   NodePtr  n1NIL,
                                                                   NodePtr  n2,
                                                                   NodePtr  n2NIL
                                                                  )
{
  if  ((n1 == n1NIL)  &&  (n2 == n2NIL))
    return  true;

  if  ((n1 == n1NIL)  ||  (n2 == n2NIL))
    return  false;


  if  (*(n1->Data ())  == *(n2->Data ()))
  {
    if  (!AreSubTreesTheSame (n1->Left (), n1NIL, n2->Left (), n2NIL))
      return false;

    if  (!AreSubTreesTheSame (n1->Right (), n1NIL, n2->Right (), n2NIL))
      return false;
  }
  else
  {
    return false;
  }

  return true;
}




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*   KKU::RBTree<Entry,CompareNodes,KeyType>::GetEqual (NodePtr         subTree,
                                                                         const KeyType&  key
                                                                        )
{
  if  (subTree == nil)
    return nil;

  NodePtr n = subTree;

  //KeyType  nodeKey;

  NodePtr lastEqual = nil;

  while  (n != nil)
  {
    const  KeyType&  nodeKey = comparator.ExtractKey (n->Data ());

    if  (key < nodeKey)
    {
      n = n->Left ();
    }
    
    else if  (key > nodeKey)
    {
      n = n->Right ();
    }

    else
    {
      lastEqual = n;
      n = n->Left ();
    }
  }

  return lastEqual;
}  /* GetEqual */





template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::GetEqual (const KeyType&  key)
{
  curNode = GetEqual (root, key);

  if  (curNode == nil)
    return NULL;

  return  curNode->Data ();
}





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*   KKU::RBTree<Entry,CompareNodes,KeyType>::GetGreater (NodePtr         subTree,
                                                                           const KeyType&  key
                                                                          )
{
  if  ((subTree == nil)  ||  (subTree == NULL))
    return nil;

  NodePtr n           = subTree;
  NodePtr lastGreater = nil;

  bool  found = false;

  while  (n != nil)
  {
    KeyType  nodeKey = comparator.ExtractKey (n->Data ());

    if  (key < nodeKey)
    {
      lastGreater = n;
      n = n->Left ();
    }
    
    else
    {
      n = n->Right ();
    } 
  }

  if  (lastGreater == nil)
  {
    curNode = nil;
    return nil;
  }

  curNode = lastGreater;

  return  curNode;
}  /* GetGreater */





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*   KKU::RBTree<Entry,CompareNodes,KeyType>::GetGreaterOrEqual (NodePtr         subTree,
                                                                                  const KeyType&  key
                                                                                 )
{
  if  ((subTree == nil)  ||  (subTree == NULL))
    return nil;

  NodePtr n           = subTree;
  NodePtr lastGreater = nil;

  bool  found = false;

  while  (n != nil)
  {
    KeyType  nodeKey = comparator.ExtractKey (n->Data ());

    if  (key <= nodeKey)
    {
      lastGreater = n;
      n = n->Left ();
    }
    
    else
    {
      n = n->Right ();
    } 
  }

  if  (lastGreater == nil)
  {
    curNode = nil;
    return nil;
  }
 
  curNode = lastGreater;

  return  curNode;
}  /* GetGreater */







template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::GetGreater (const KeyType&  key)
{
  curNode = GetGreater (root, key);
  return curNode->Data ();
}  /* GetGreater */




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::GetGreaterOrEqual (const KeyType&  key)
{
  curNode = GetGreaterOrEqual (root, key);
  return curNode->Data ();
}  /* GetGreater */




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*   KKU::RBTree<Entry,CompareNodes,KeyType>::GetLess (NodePtr         subTree,
                                                                        const KeyType&  key
                                                                       )
{
  if  ((subTree == nil)  ||  (subTree == NULL))
    return nil;

  NodePtr n        = subTree;
  NodePtr lastLess = nil;

  bool  found = false;

  KeyType  nodeKey;

  while  (n != nil)
  {
    KeyType  nodeKey = comparator.ExtractKey (n->Data ());

    if  (key > nodeKey)
    {
      lastLess = n;
      n = n->Left ();
    }
    
    else
    {
      n = n->Right ();
    } 
  }

  if  (lastLess == nil)
  {
    return nil;
  }

  n = lastLess;

  while  ((n != nil)  &&  (comparator.ExtractKey (n) < key))
  {
    lastLess = n;
    n = Successor (n);
  }
  
  return  lastLess;
}   /* GetLess */





template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::GetLess (const KeyType&  key)
{
  curNode = GetLess (root, key);
  return curNode->Data ();
}  /* GetGreater */







template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::Predeccessor ()
{
  if  ((curNode == nil)   ||  (curNode == NULL))
    return NULL;

  curNode = Predeccessor (curNode);

  if  ((curNode == nil)   ||  (curNode == NULL))
    return NULL;

  return  curNode->Data ();
}  /* Predeccessor */





template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::Successor ()
{
  if  ((curNode == nil)   ||  (curNode == NULL))
    return NULL;

  curNode = Successor (curNode);

  if  ((curNode == nil)   ||  (curNode == NULL))
    return NULL;

  return  curNode->Data ();
}  /* Successor */





template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::DeleteCurrentNode ()
{
  if  ((curNode == nil)   ||  (curNode == NULL))
  {
    std::cerr << std::endl;
    std::cerr << "    *** Warning ***"  << std::endl;
    std::cerr << std::endl;
    std::cerr << "RBTree<Entry,CompareNodes,KeyType>::DeleteCurrentNode -  No Current Node Defined." << std::endl;
    std::cerr << std::endl;
    return;
  }


  EntryPtr  entry2Delete = curNode->Data ();

  NodePtr  node2Delete = curNode;

  curNode = Predeccessor (node2Delete);
  if  ((curNode == nil)  ||  (curNode == NULL))
    curNode = Successor (node2Delete);


  NodePtr  deletedNode = RBDelete (node2Delete);

  if  (deletedNode->Data () != entry2Delete)
  {
    std::cerr << std::endl;
    std::cerr << "    *** ERROR ***"  << std::endl;
    std::cerr << std::endl;
    std::cerr << "RBTree<Entry,CompareNodes,KeyType>::DeleteCurrentNode -  Wrong Node was Returned for Deletion." << std::endl;
    std::cerr << std::endl;
  }
  else
  {
    if  (owner)
      delete  deletedNode->Data ();

    delete  deletedNode;
  }
}  /* DeleteCurrentNode */




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::GetFirst ()
{
  if  (root == nil)
    return NULL;
  
  curNode = Minimum ();
  return  curNode->Data ();
}  /* GetFirst () */




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::RBTree<Entry,CompareNodes,KeyType>::GetLast ()
{
  if  (root == nil)
    return NULL;
  
  curNode = Maximum ();
  return  curNode->Data ();
}  /* GetLast () */





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::Insert (EntryPtr  _entry)
{
  NodePtr  newNode = NULL;

  const KeyType&  newKey = comparator.ExtractKey (_entry);

  if  (root == nil)
  {
    newNode = new RBnode<Entry> (nil, nil, nil, Red, _entry);
    root = newNode;
  }
  else
  {
    NodePtr  next = root;
    NodePtr  last = root;

    int32  lastBranch = 0;
    
    while (next != nil)
    {
      last = next;

      const KeyType&  nextKey = comparator.ExtractKey (next->Data ());

      if  (newKey < nextKey)
      {
        lastBranch = -1;
        next = next->Left ();
      }
      else
      {
        lastBranch = 1;
        next = next->Right ();
      }
    }

    newNode = new RBnode<Entry> (last, nil, nil, Red, _entry);

    if  (lastBranch < 0)
    {
      last->Left (newNode);
    }
    else
    {
      last->Right (newNode);
    }
  }  

  size++;

  curNode = newNode;

  return  newNode;
}  /* Insert */





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::Minimum (NodePtr  n)
{
  while  (n->Left () != nil)
    n = n->Left ();
    
  return  n;
}





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::Maximum (NodePtr  n)
{
  while  (n->Right () != nil)
    n = n->Right ();
    
  return  n;
}






template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::Successor (NodePtr  n)
{
  if  ((n == nil)  ||  (n == NULL))
    return nil;


  if  (n->Right () != nil)
    return  Minimum (n->Right ());

  NodePtr  next = n->Parent ();

  NodePtr  last = n;

  while  ((next != nil)  &&  (last == next->Right ()))
  {
    last = next;
    next = next->Parent ();
  }

  return  next;
}  /* Successor */





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::Predeccessor (NodePtr  n)
{
  if  ((n == nil)  ||  (n == NULL))
    return  nil;

  if  (n->Left () != nil)
    return  Maximum (n->Left ());

  NodePtr  pred = n->Parent ();

  NodePtr  last = n;
 
  while  ((pred != nil)  &&  (last == pred->Left ()))
  {
    last = pred;
    pred = pred->Parent ();
  }

  return  pred;
}  /* Predeccessor */



 


template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::Delete (NodePtr  z)
{
  NodePtr  y;
  NodePtr  x;

  if  (z == curNode)
  {
    curNode = Successor (z);
    if  (curNode == nil)
      curNode = Predeccessor (z);
  }

  if  ((z->Left () == nil)  ||  (z->Right () == nil))
  {
    y = z;
  }
  else
  {
    y = Successor (z);
    z->Data (y->Data ());

    if  (y == curNode)
      curNode = z;
  }


  if  (y->Left ()  !=  nil)
    x = y->Left ();
  else
    x = y->Right ();

  if  (x != nil)
  {
    x->Parent (y->Parent ());
  }

  
  if  (y->Parent () == nil)
  {
    root = x;
  }
  else
  {
    if  (y == y->Parent ()->Left ())
      y->Parent ()->Left  (x);
    else
      y->Parent ()->Right (x);
  }

  size--;

  return  y;
}  /* Delete */







template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::LeftRotate (NodePtr  x)
{  
  if  ((x == nil)  ||  (x == NULL))
  {
    std::cerr << std::endl
              << "                    ******* ERROR *******"                      << std::endl
              << std::endl
              << "RBTree<Entry,CompareNodes,KeyType>::LeftRotate,  Invalid Node." << std::endl
              << std::endl;
    return x;
  }


  if  (x->Right () == nil)
  {
    // There is no Right Child, So can Not Posibly LeftRotate
    std::cerr << std::endl
              << "                   ******* ERROR *******"                            << std::endl
              << std::endl
              << "*****  RBTree<Entry,CompareNodes,KeyType>::LeftRotate,  No right Child  ***" << std::endl
              << std::endl;
    return x;
  }

  NodePtr  y = x->Right ();  // Set Y.

  x->Right (y->Left ());     // Turn y's left Sub-Tree into x's Right subtree

  if  (y->Left () != nil)
     y->Left ()->Parent (x);

  y->Parent (x->Parent ());  // Link X's parent to Y

  
  if  (x->Parent () == nil)
  {
    root = y;
  }

  else if  (x == x->Parent ()->Left ())
  {
    x->Parent ()->Left (y);
  }

  else
  {
    x->Parent ()->Right (y);
  }


  y->Left (x);
  x->Parent (y);

  return  y;
}  /* LeftRotate */




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::RightRotate (NodePtr  x)
{
  if  ((x == nil)  ||  (x == NULL))
  {
    cerr << std::endl
         << "                    ******* ERROR *******"                       << std::endl
         << std::endl
         << "RBTree<Entry,CompareNodes,KeyType>::RightRotate,  Invalid Node." << std::endl
         << std::endl;
    return x;
  }


  if  (x->Left () == nil)
  {
    // There is no Left Child, So can Not Posibly LeftRotate
    cerr << std::endl
         << "                   ******* ERROR *******"                            << std::endl
         << std::endl
         << "*****  RBTree<Entry,CompareNodes,KeyType>::RightRotate,  No Left Child  ***" << std::endl
         << std::endl;
    return x;
  }


  NodePtr  y = x->Left ();   // Set Y.

  x->Left (y->Right ());     // Turn y's left Sub-Tree into x's Right subtree

  if  (y->Right () != nil)
     y->Right ()->Parent (x);

  y->Parent (x->Parent ());  // Link X's parent to Y  //
  
  if  (x->Parent () == nil)
  {
    root = y;
  }

  else if  (x == x->Parent ()->Right ())
  {
    x->Parent ()->Right (y);
  }

  else
  {
    x->Parent ()->Left (y);
  }

  y->Right (x);
  x->Parent (y);

  return  y;
}  /* RightRotate */




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*    KKU::RBTree<Entry,CompareNodes,KeyType>::RBInsert (EntryPtr  e)
{
  NodePtr  newNode = Insert (e);

  NodePtr  x = newNode;

  NodePtr  y = NULL;

  x->Color (Red);

  while  ((x != root)  &&  (x->Parent ()->Color () == Red))
  {
    if  (x->Parent () == x->Parent ()->Parent ()->Left ())
    {
      y = x->Parent ()->Parent ()->Right ();

      if  (y->Color () == Red)
      {
        x->Parent ()->Color (Black);              // Case 1
        y->Color (Black);                         // Case 1
        x->Parent ()->Parent ()->Color (Red);     // Case 1
        x = x->Parent ()->Parent ();              // Case 1
      }

      else
      {
        if  (x == x->Parent ()->Right ())
        {        
          x = x->Parent ();                       // Case 2
          LeftRotate (x);                         // Case 2
        }

        x->Parent ()->Color (Black);              // Case 3
        x->Parent ()->Parent ()->Color (Red);     // Case 3
        RightRotate (x->Parent ()->Parent ());    // Case 3
      }
    }

    else
    {
      y = x->Parent ()->Parent ()->Left ();

      if  (y->Color () == Red)
      {
        x->Parent ()->Color (Black);
        y->Color (Black);
        x->Parent ()->Parent ()->Color (Red);
        x = x->Parent ()->Parent ();
      }

      else
      {
        if  (x == x->Parent ()->Left ())
        {
          x = x->Parent ();
          RightRotate (x);
        }

        x->Parent ()->Color (Black);
        x->Parent ()->Parent ()->Color (Red);
        LeftRotate (x->Parent ()->Parent ());
      }
    }
  }


  root->Color (Black);
  return  newNode;
}  /* RBInsert */





template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*  KKU::RBTree<Entry,CompareNodes,KeyType>::RBDelete (NodePtr z)
{
  if  ((z == nil) ||  (z == NULL))
  {
    cerr << std::endl
         << "                   ******* ERROR *******"                                    << std::endl
         << std::endl
         << "*****  RBTree<Entry,CompareNodes,KeyType>::RBDelete,     No Left Child  ***" << std::endl
         << std::endl;
    return NULL;
  }


  NodePtr  x = NULL;
  NodePtr  y = NULL;


  if  ((z->Left () == nil)  ||  (z->Right () == nil))
    y = z;
  else
    y = Successor (z);

  if  (y->Left () != nil)
    x = y->Left ();
  else
    x = y->Right ();

  x->Parent (y->Parent ());

  if  (y->Parent () == nil)
    root = x;

  else if  (y == y->Parent ()->Left ())
    y->Parent ()->Left (x);

  else
    y->Parent ()->Right (x);

  if  (y != z)
  {
    if  (y == curNode)
      curNode = z;
      
    EntryPtr  temp   = z->Data ();
    z->Entry  (y->Data ());
    y->Entry  (temp);
  }

  size--;

  if  (y->Color () == Black)
  {
    RBDeleteFixUp (x);
  }

  return y;
}  /* RBDelete */





template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::RBDeleteFixUp (NodePtr x)
{
  NodePtr  w           = NULL;
  NodePtr  deletedNode = x;

  while  ((x != root)  &&  (x->Color () == Black))
  {
    if  (x == x->Parent ()->Left ())
    {
      w = x->Parent ()->Right ();

      if  (w->Color () == Red)
      {
        w->Color (Black);                       // Case 1
        x->Parent ()->Color (Red);              // Case 1
        LeftRotate (x->Parent ());              // Case 1
        w = x->Parent ()->Right ();             // Case 1
      }

      if  ((w->Left ()->Color () == Black)  
                          &&  
           (w->Right ()->Color () == Black))
      {
        w->Color (Red);                         // Case 2
        x = x->Parent ();                       // Case 2
      }

      else 
      {
        if  (w->Right ()->Color () == Black)
        {
          w->Left ()->Color (Black);            // Case 3
          w->Color (Red);                       // Case 3
          RightRotate (w);                      // Case 3
          w = x->Parent ()->Right ();           // Case 3
        }

        w->Color (x->Parent ()->Color ());      // Case 4
        x->Parent ()->Color (Black);            // Case 4
        w->Right ()->Color (Black);             // Case 4
        LeftRotate (x->Parent ());              // Case 4
        x = root; // Force Termination of Loop. // Case 4
      }
    }

    else
    {
      w = x->Parent ()->Left ();

      if  (w->Color () == Red)
      {
        w->Color (Black);                       // Case 1
        x->Parent ()->Color (Red);              // Case 1
        RightRotate (x->Parent ());             // Case 1
        w = x->Parent ()->Left ();              // Case 1
      }

      if  ((w->Right ()->Color () == Black)  
                          &&  
           (w->Left ()->Color () == Black))
      {
        w->Color (Red);                         // Case 2
        x = x->Parent ();                       // Case 2
      }

      else 
      {
        if  (w->Left ()->Color () == Black)
        {
          w->Right ()->Color (Black);           // Case 3
          w->Color (Red);                       // Case 3
          LeftRotate (w);                       // Case 3
          w = x->Parent ()->Left ();            // Case 3
        }

        w->Color (w->Parent ()->Color ());      // Case 4
        x->Parent ()->Color (Black);            // Case 4
        w->Left ()->Color (Black);              // Case 4
        RightRotate (x->Parent ());             // Case 4
        x = root; // Force Termination of Loop. // Case 4
      }
    }
  }

  x->Color (Black);

  return;;
}  /* RBDeleteFixUp */





template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::WalkTree (NodePtr  n)
{
  if  (n == nil)
    return;

  WalkTree (n->Left ());
  cout << n->Data ()->ToString () << endl;
  WalkTree (n->Right ());
}




template <class Entry,class CompareNodes,typename KeyType>
KKU::RBnode<Entry>*   KKU::RBTree<Entry,CompareNodes,KeyType>::SearchForEntry (NodePtr  n,
                                                                               Entry*   e
                                                                              )
{
  // Similar to WalkTree,  except will terminate when it finds the node
  // that is pointing to "e".
  if  (n == nil)
    return;
  
  if  (n->Data () == e)
    return n;

  RBnode<Entry>* nextN = NULL;

  nextN = SearchForEntry (n->Left (), e);
  if  (!nextN)
    nextN = SearchForEntry (n->Right (), e);

  return  nextN;
}  /* SearchForEntry */




template <class Entry,class CompareNodes,typename KeyType>
void   KKU::RBTree<Entry,CompareNodes,KeyType>::DeleteEntry (Entry*  e)
{
  RBnode<Entry>* node2Delete = SearchForEntry (root, e);
  if  (node2Delete)
    RBDelete (node2Delete);
}  /* DeleteEntry */




template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::CalcTreeStats ()
{
  int32  numOfLeafs = 0;
  int32  numOfNodes = 0;
  int32* leafPaths  = new int32[size];

  int32  shortestPath = INT_MAX;
  int32  longestPath  = INT_MIN;

  CheckNodeStats (root, numOfNodes, numOfLeafs, leafPaths, shortestPath, longestPath, 0);

  int32  totalLen = 0;

  for  (int32 x = 0; x < numOfLeafs; x++)
    totalLen += leafPaths[x];

  double  avgLen = (double)totalLen / (double)numOfLeafs;

  std::cout << std::endl
            << "Number of Nodes   [" << numOfNodes   << "]"  << std::endl
            << "Number of Leafs   [" << numOfLeafs   << "]"  << std::endl
            << "Shortest Path     [" << shortestPath << "]"  << std::endl
            << "Longest  Path     [" << longestPath  << "]"  << std::endl
            << "Average Lengath   [" << avgLen       << "]"  << std::endl;
}




template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::CheckNodeStats (NodePtr  n,
                                                               int32&     numOfNodes,
                                                               int32&     numOfLeafs,
                                                               int32*     leafPaths,
                                                               int32&     shortestPath,
                                                               int32&     longestPath,
                                                               int32      curDeapth
                                                              )

{
  if  (n == nil)
    return;

  numOfNodes++;

  curDeapth++;
  
  if  ((n->Left () == nil)  &&  (n->Right () == nil))
  {
    // We have a Leaf
    leafPaths[numOfLeafs] = curDeapth;

    if  (curDeapth < shortestPath)
      shortestPath = curDeapth;

    if  (curDeapth > longestPath)
      longestPath = curDeapth;

    numOfLeafs++;

    return;
  }


  if  (n->Left () != nil)
     CheckNodeStats (n->Left (),
                     numOfNodes, 
                     numOfLeafs,
                     leafPaths,
                     shortestPath,
                     longestPath,
                     curDeapth
                    );


  if  (n->Right () != nil)
     CheckNodeStats (n->Right (), 
                     numOfNodes, 
                     numOfLeafs,
                     leafPaths,
                     shortestPath,
                     longestPath,
                     curDeapth
                    );
}  /* CheckNodeStats */




template <class Entry,class CompareNodes,typename KeyType>
bool  KKU::RBTree<Entry,CompareNodes,KeyType>::Validate ()
{
  int32  blackNodeHeight = -1;

  int32  numOfNodes = ValidateSubTree (root, nil, "Root", 0, blackNodeHeight);

  if  (numOfNodes != size)
  {
    std::cerr << std::endl
              << "*** ERROR *** NumOfNodes[" << numOfNodes << "] Does not add up to size[" << size << "]." << std::endl;
    return  false;
  }

  return true;
}


template <class Entry,class CompareNodes,typename KeyType>
KKU::int32  KKU::RBTree<Entry,CompareNodes,KeyType>::ValidateSubTree (NodePtr      subTree,
                                                                      NodePtr      theParent,
                                                                      const char*  linkDescription,
                                                                      KKU::int32   blackNodeCount,
                                                                      KKU::int32&  blackNodeHeight
                                                                     )
{
  int32  numOfNodes = 0;


  if  (subTree->Color () == Black)
    blackNodeCount++;


  if  (subTree == nil)
  {
    if  (blackNodeHeight < 0)
    {
      blackNodeHeight = blackNodeCount;
    }
    else
    {
      if  (blackNodeCount != blackNodeHeight)
      {
        std::cerr << std::endl
             << "*** ERROR ***"  << std::endl
             << std::endl
             << "ValidateSubTree,   Not all Leafs have Same Height" << std::endl
             << "                   firstHeight[" << blackNodeHeight << "]  " 
             <<                    "This Leaf["   << blackNodeCount  << "]"
             << std::endl
             << std::endl;
      }
    }
    
    return  0;
  }

  numOfNodes++;

  if  (subTree->Parent () != theParent)
  {
    std::cerr << std::endl;
    std::cerr << "*** ERROR *** Node[" << subTree->Data ()->ToString () 
                           << "]  Child[" << linkDescription << "]"
                           << " not Pointing yo Its Parent[" 
                           << theParent->Data ()->ToString () 
                           << "]." 
                           << std::endl;
    cerr << std::endl;

    return  numOfNodes;
  }

  numOfNodes += ValidateSubTree (subTree->Left  (), subTree, "Left",  blackNodeCount, blackNodeHeight);
  numOfNodes += ValidateSubTree (subTree->Right (), subTree, "Right", blackNodeCount, blackNodeHeight);

  return  numOfNodes;
}  /* ValidateSubTree */






template <class Entry,class CompareNodes,typename KeyType>
KKU::int32  KKU::RBTree<Entry,CompareNodes,KeyType>::CountNodesInSubTree (NodePtr  subTree)
{
  if  (subTree == nil)
    return 0;
  
  return 1 + CountNodesInSubTree (subTree->Left ())
           + CountNodesInSubTree (subTree->Right ());
}





template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::IteratorDelete (IteratorPtr  deletedIterator)
{
  if  (numOfIterators < 1)
    return;


  if  (numOfIterators == 1)
  {
    delete  iterators;
    iterators = NULL;
    numOfIterators = 0;
    return;
  }


  numOfIterators--;
  IteratorPtr*  newIterators = new IteratorPtr[numOfIterators];

  int32  j = 0;
  for  (int32 i = 0;  i <= numOfIterators;  i++)
  {
    if  (iterators[i] != deletedIterator)
    {
      newIterators[j] = iterators[i];
      j++;
    }
  }

  delete  iterators;
  iterators = newIterators;
}  /* IteratorDelete */





template <class Entry,class CompareNodes,typename KeyType>
void  KKU::RBTree<Entry,CompareNodes,KeyType>::IteratorAdd  (IteratorPtr  newIterator)
{
  IteratorPtr*  newIterators = new IteratorPtr[numOfIterators + 1];

  for (int32  i = 0;  i < numOfIterators; i++)
  {
    newIterators[i] = iterators[i];
  }

  newIterators[numOfIterators] = newIterator;
  numOfIterators++;

  if  (iterators != NULL)
    delete  iterators;

  iterators = newIterators;
}  /* IteratorAdd */





template <class Entry,class CompareNodes,typename KeyType>
KKU::Iterator<Entry,CompareNodes,KeyType>::Iterator (TreePtr  _tree)
{
  tree = _tree;

  prevNode = tree->nil;
  curNode  = tree->nil;
  nextNode = tree->Minimum ();

  tree->IteratorAdd (this);
}



template <class Entry,class CompareNodes,typename KeyType>
KKU::Iterator<Entry,CompareNodes,KeyType>::~Iterator ()
{
  if  (tree)
    tree->IteratorDelete (this);
}




template <class Entry,class CompareNodes,typename KeyType>
void  KKU::Iterator<Entry,CompareNodes,KeyType>::IsTreeStillThere ()
{
  if  (tree == NULL)
  {
    std::cerr << std::endl
         << "*** ERROR ***"  << std::endl
         << std::endl
         << "***  RBTree<Entry,CompareNodes,KeyType>::Iterator::IsTreeStillThere ***"  << std::endl
         << std::endl
         << "Tree is no longer available"  << std::endl
         << std::endl;
     exit (-1);
  }
}  /* IsTreeStillThere */




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetEqual (const KeyType&  key)
{
  IsTreeStillThere ();

  curNode = tree->GetEqual (tree->root, key);

  if  (curNode == tree.nil)
  {
    nextNode = tree->GetGreater (tree->root, key);
    if  (nextNode == tree.nil)
      prevNode = tree->GetLess (tree->root, key);

    else
      prevNode = tree->Predeccessor (nextNode);
  
    return NULL;
  }


  prevNode = tree->Predeccessor (curNode);
  nextNode = tree->Successor (curNode);

  return  curNode->Data ();
}


 
  
template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetGreater (const KeyType&  key)
{
  IsTreeStillThere ();

  curNode = tree->GetGreater (tree->root, key);

  if  (curNode == tree->nil)
  {
    nextNode = tree->nil;
    prevNode = tree->Maximum (tree->root);
  }

  else
  {
    nextNode = tree->Successor (curNode);
    prevNode = tree->Predeccessor (curNode);
  }

  return  curNode->Data ();
}  /* Iterator::GetGreater */




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetGreaterOrEqual (const KeyType&  key)
{
  IsTreeStillThere ();

  curNode = tree->GetGreaterOrEqual (tree->root, key);

  if  (curNode == tree->nil)
  {
    nextNode = tree->nil;
    prevNode = tree->Maximum (tree->root);
  }

  else
  {
    nextNode = tree->Successor (tree.curMode);
    prevNode = tree->Predeccessor (tree.curNode);
  }

  return  curNode->Data ();
}  /* Iterator::GetGreaterOrEqual */






template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetFirst ()
{
  IsTreeStillThere ();

  prevNode = tree->nil;
  curNode  = tree->Minimum ();
  nextNode = tree->Successor (curNode);

  return  curNode->Data ();
}




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetLast ()
{
  IsTreeStillThere ();
  
  curNode  = tree->Maximum ();
  prevNode = tree->Predeccessor (curNode);
  nextNode = tree->nil;

  return  curNode->Data ();
}




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetNext ()
{
  IsTreeStillThere ();


  if  (nextNode == tree->nil)
  {
    return NULL;
  }

  prevNode = curNode;
  curNode = nextNode;

  if  (prevNode == tree->nil)
  {
    prevNode = tree->Predeccessor (curNode);
  }

  nextNode = tree->Successor (curNode);
  
  return  curNode->Data ();  
}




template <class Entry,class CompareNodes,typename KeyType>
Entry*  KKU::Iterator<Entry,CompareNodes,KeyType>::GetPrev ()
{
  IsTreeStillThere ();

  if  (prevNode == tree->nil)
  {
    return NULL;
  }

  nextNode = curNode;
  curNode = prevNode;

  if  (nextNode == tree->nil)
  {
    prevNode = tree->Successor (curNode);
  }

  prevNode = tree->Predeccessor (curNode);
  
  return  curNode->Data ();  
}




template <class Entry,class CompareNodes,typename KeyType>
void  KKU::Iterator<Entry,CompareNodes,KeyType>::TreeHasBeenDeleted ()
{
  // Called by RBTree object to let iterators know that the tree they are
  // Iterating is been deleted.
  tree = NULL;

  curNode = prevNode = nextNode = NULL;
}




template <class Entry,class CompareNodes,typename KeyType>
void  KKU::Iterator<Entry,CompareNodes,KeyType>::DeletionOfNode (NodePtr  n)
{
  // Called by RBTree object to let iterators know that a k=node has been deleted 
  // and this way they can make the appropriate adjustments to the next, prev, and
  // curt node pointers.

  if  (n == prevNode)
  {
    if  (prevNode != tree->nil)
      prevNode = tree->Predeccessor (prevNode);

    return;
  }

  if (n == curNode)
  {
    curNode = tree->nil;
    return;
  }

  if (n == nextNode)
  {
    nextNode = tree->Successor (nextNode);
  }
}  /* DeletionOfNode */



template <class Entry,class CompareNodes,typename KeyType>
void  KKU::Iterator<Entry,CompareNodes,KeyType>::InsertionOfNode (NodePtr  n)
{
}


#endif

