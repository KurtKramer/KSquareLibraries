/* KKQueue.h -- Double sided queue structure  sub-classed from vector template.
 * Copyright (C) 1994-2011 Kurt Kramer & Sergiy Fefilatyev
 * For conditions of distribution and use, see copyright notice in KKU.h
 *
 * 2011-06  Sergiy Fefilatyev found a bug in my 'KKU::KKQueue::RandomizeOrder' method.
 * 2011-10-21  Fixed bug in prev fix to 'KKU::KKQueue::RandomizeOrder'.     
 */
#ifndef  _KKU_KKQUEUE_
#define  _KKU_KKQUEUE_

// Have to use _KKU_KKQUEUE_ because _QUEUE_ is used by #include <queue>  from STL

/**
 *@file KKQueue.h
 *@details
 *@code
 *************************************************************************************
 **                                                                                  *
 **  Developed By:  Kurt A. Kramer                                                   *
 **                                                                                  *
 **  Date:          Way back in history                                              *
 **                                                                                  *
 *************************************************************************************
 **  A KKQueue template.  Makes it quick and easy to create Container objects.       *
 **                                                                                  *
 **  Based off the vector<> template, but consists of only pointers to objects of    *
 **  type 'Entry'.                                                                   *
 **                                                                                  *
 **  Can act as a 'Array', 'Queue', or 'Stack'.                                      *
 **                                                                                  *
 **  'owner' - When you create an instance of 'KKQueue' you specify weather it will  *
 **  own its contents or not.  If 'KKQueue' owns its contents then it will delete    *
 **  them when it is deleted.  That is 'KKQueue' will be responsible for calling     *
 **  the destructor for each 'entry' it contains.                                    *
 **                                                                                  *
 **                                                                                  *
 *************************************************************************************
 */


//#include <assert.h>
//#include <ctype.h>
//#include <stdlib.h>
//#include <memory>
//#include <vector>
//#include <algorithm>
//#include <functional>
//#include <iostream>

#include "BasicTypes.h"

namespace  KKU
{
  template <class Entry>  class QueueComparison;

  /**
   *@class  KKQueue  Template based off the std::vector template.
   *@brief  A typed container class/template that keeps track of entries via pointers only while.
   *@details  Will act as an Array, Queue or Stack structures.  Items are added by the 'PushOnFront' and 
   *          'PushOnBack' methods. They are removed by the 'PopFromFront' and 'PopFromBack' methods.
   *          What is important to keep in mind is that it holds pointers to its contents, not the actual
   *          instances. It is important to keep track who owns the objects you put in an instance of 
   *          KKQueue.  KKQueue has a 'Owner' flag that you can set. If it is set to 'true' then it will
   *          call the destructor on all its contents when you call KKQueue's destructor or the 
   *          'DestroyContents' method. When you add an element to a KKQueue container you can not delete
   *          it separately until you either remove it from KKQueue or delete the KKQueue container.
   *          If the KKQueue derived container owned its contents and you call its destructor then there 
   *          is no need to delete the contents separately.
   *
   *          KKQueue is sub-classed from vector<Entry*> so you can use any method that is available for
   *          the vector<> template.
   *
   *@tparam[in]  Entry  The type of objects that are to be held in this container. When you add new instances
   *                    of 'Entry', you need to add a pointer to it not the actual entry.
   */
  template <class Entry>
  class  KKQueue: public std::vector<Entry*>
  {
    typedef  Entry*        EntryPtr;
    typedef  Entry const * EntryConstPtr;
   //typedef  typename   Entry*  EntryPtr;

  public:
    typedef  KKU::int32   int32;
    typedef  KKU::uint32  uint32;


  private:
      uint32     growthRate;  /*!<  No longer does anything, left here for compatibility with older code. */

      bool       owner;       /*!< if True the KKQueue structure owns the objects and is responsible for
                                   deleting them when the KKQueue structure is deleted.
                               */

  public:
      typedef  typename std::vector<Entry*>::iterator        iterator;
      typedef  typename std::vector<Entry*>::const_iterator  const_iterator;


      KKQueue  (bool _owner = true, 
                int32  _growthRate = 50
               );

  protected:
      /**
       *@brief  Copy Constructor, will create a new instance.
       *@details If the parameter 'q' owns its contents then this constructor will also create new 
       *         instances of its contents. That is it will call the Copy  Constructor for each
       *         one of the elements it contains If 'q' does not own its contents it will just
       *         copy over the pointers from 'q', meaning that the new instance of KKQueue will 
       *         point to the same locations as 'q' does.
       */
      KKQueue (const KKQueue&  q);

  public:
      /**
       *@brief   Constructor, similar to the Copy Constructor except that you can control whether
       *         it duplicates the contents.
       *@details If the '_owner' parameter is set to true then it will create new instances of the 
       *         contents otherwise it will just point to the instances that already exist.
       */
      KKQueue (const KKQueue&  q,
               bool            _owner
              );

      ~KKQueue ();


      inline  void      Add         (EntryPtr _entry);          /**< @brief  same as PushOnBack   */
      inline  void      AddFirst    (EntryPtr _entry);          /**< @brief  same as PushOnFront  */
      inline  EntryPtr  GetFirst    ()                  const;
      inline  EntryPtr  GetLast     ()                  const;
      inline  EntryPtr  RemoveFirst ();                         /**< @brief  same as PopFromFront  */
      inline  EntryPtr  RemoveLast  ();                         /**< @brief  same as PopFromBack   */
         

      KKQueue*    DuplicateListAndContents ()           const;  /**< @brief  Creates a new container including duplicating the contents, which also makes the new instance the owner of those contents. */

      /**
       *@fn  void AddQueue (KKQueue& q);
       *@brief  Add the contents of a separate KKQueue container to this container.
       *@details  Be careful how the Owner flags are set. This method adds the pointers of 'q' to the end of its own container. It does not concern itself
       *          with the state of the 'Owner' flags. If you are not careful you can have both containers thinking they own the same entries. I suggest
       *          that after you add the contents of 'q' to this container that the caller set the 'owner' flag of 'q' to false.
       */
      void      AddQueue       (const KKQueue& q);                    
                                                                   
      EntryPtr  BackOfQueue    ()                       const;  /**< @brief  Returns pointer of last element in the container without removing it.  If the container is empty will return NULL.  */
      void      Compress       ();                              /**< @brief  No longer does anything. */
      void      DeleteContents ();                              /**< @brief  Empties the container, if 'owner' is set to true will call the destructor on each element.                     */
      void      DeleteEntry    (EntryPtr _entry);               /**< @brief  Removes from KKQueue the entry who's pointer = '_entry'                                                           */
      void      DeleteEntry    (uint32   _idx);                 /**< @brief  Removes from KKQueue the entry who's index = '_idx'.                                                              */
      EntryPtr  FrontOfQueue   ()                       const;  /**< @brief  Returns a pointer to the element that is at from of the queue with out removing it from the queue.                */
      uint32    GrowthRate     ()                       const;
      void      GrowthRate     (uint32   _growthRate);
      EntryPtr  IdxToPtr       (uint32   idx)           const;  /**< @brief  Returns back a pointer to the element who's index is 'idx'. If 'idx' is less than 0 or >= QueueSize()  will return NULL.               */

      int32     LocateEntry    (EntryConstPtr _entry)   const;  /**< @brief  Returns the index of the element who's address is '_entry'. If not found in container will return back -1.                             */
      EntryPtr  LookAtBack     ()                       const;  /**< @brief  Returns pointer to element at Back of container with out removing it from the container.  If container is empty will return back NULL  */ 
      EntryPtr  LookAtFront    ()                       const;  /**< @brief  Returns pointer to element at Front of container with out removing it from the container.  If container is empty will return back NULL */ 
      bool      Owner          ()                       const;
      void      Owner          (bool     _owner);               /**< @brief  You can specify who owns the contents of the container.  '_owner' == true means when the container is deleted it will also call the destructor for each element it contains. */
      void      PushOnFront    (EntryPtr _entry);               /**< @brief  Adds '_entry' to the Front of the container.                                                      */
      void      PushOnBack     (EntryPtr _entry);               /**< @brief  Adds '_entry' to the End of the container.                                                        */
      EntryPtr  PopFromFront   ();                              /**< @brief  Removes the first element in the container and returns its pointer.  If the container is empty will return NULL.  */
      EntryPtr  PopFromBack    ();                              /**< @brief  Removes the last element in the container and returns its pointer. If the container is empty will return NULL.    */
      int32     PtrToIdx       (EntryConstPtr _entry)   const;  /**< @brief  returns the index of the 'entry' that has the same pointer as '_entry', if none found returns -1 */
      void      RandomizeOrder ();
      void      RandomizeOrder (int64     seed);
      int32     QueueSize      ()                       const;  /**< @brief  Same as calling vector<>::size().  Returns the number of elements in KKQueue  */
      //void      Sort           (QueueComparison<Entry>*  comparison);

      void      SetIdxToPtr    (uint32  _idx,
                                Entry*  _ptr
                               );

      void      SwapIndexes    (size_t idx1,  size_t idx2);

      Entry&    operator[] (uint32 i)      const;                 /**< @brief  Returns a reference to element indexed by 'i'  */


      /** 
       *@brief  Assignment Operator
       *@details
       *@code
       *  1) Destroy's its current contents by calling 'DeleteContents'.
       *  2) If 'q' owns its contents 
       *       create new instances of 'q's contents and add to its self  then set 'owner' flag to true.
       *     else 
       *       Copy over 'q's pointers to self and then set 'owner' flag to false.
       *@endcode
       */
      KKQueue&  operator=  (const KKQueue& q);                        
      
      bool      operator== (const KKQueue<Entry>& rightSide)   const;  /**< @brief  Returns True if every entry in both containers point to the same elements */
      bool      operator!= (const KKQueue<Entry>& rightSide)   const;  /**< @brief  returns False if NOT every entry in both containers point to the same elements */


  private:
  };  /* KKQueue */





  template <class Entry>
  class  QueueComparison
  {
     typedef  Entry* EntryPtr;

   public:
     QueueComparison () {}

     
     bool  operator () (const EntryPtr&  p1, const EntryPtr&  p2)
     {
       return  (p1 > p2);
     }

     //virtual  bool  LessOrEqual    (EntryPtr  p1,  EntryPtr  p2) = 0;

     //virtual  bool  GreaterOrEqual (EntryPtr  p1,  EntryPtr  p2) = 0;
  };



  template <class Entry>
  KKQueue<Entry>::KKQueue (bool   _owner, 
                           int32  _growthRate
                          ):
      growthRate (_growthRate),
      owner      (_owner)
  {
  }



  template <class Entry>
  KKQueue<Entry>::KKQueue (const KKQueue&  q):

        growthRate (q.GrowthRate ()), 
        owner      (q.Owner ())

  {
    //for  (vector<Entry*>::const_iterator x = q.begin ();  x != q.end ();  x++)
    for  (const_iterator x = q.begin ();  x != q.end ();  x++)
    {
      if  (owner)
        PushOnBack (new Entry (*(*x)));
      else
        PushOnBack (*x); 
    }
  }


  template <class Entry>
  KKQueue<Entry>::KKQueue (const KKQueue&  q,
                           bool            _owner
                          ):

        growthRate (q.GrowthRate ()), 
        owner      (_owner)

  {
    //for  (vector<Entry*>::const_iterator x = q.begin ();  x != q.end ();  x++)
    for  (const_iterator x = q.begin ();  x != q.end ();  x++)
    {
      if  (owner)
        PushOnBack (new Entry (*(*x)));
      else
        PushOnBack (*x); 
    }
  }



  template <class Entry>
  KKQueue<Entry>*    KKQueue<Entry>::DuplicateListAndContents ()  const
  {
    KKQueue<Entry>*  duplicatedQueue = new KKQueue<Entry> (true, QueueSize ());

    for  (const_iterator x = KKQueue<Entry>::begin ();  x != KKQueue<Entry>::end ();  x++)
    {
      const EntryPtr e = *x;
      duplicatedQueue->PushOnBack (new Entry (*e));
    }
    
    return  duplicatedQueue;
  }  /* DuplicateListAndContents */




  template <class Entry>
  KKQueue<Entry>::~KKQueue () 
  {
    if  (owner)
    {
      iterator  i;

      for  (i = this->begin ();  i != this->end (); i++)
      {
        delete *i;
        *i = NULL;
      }
    }
  }  /* ~KKQueue */



  template <class Entry>
  void   KKQueue<Entry>::Owner  (bool _owner)
  {
    owner = _owner;
  }



  template <class Entry>
   bool  KKQueue<Entry>::Owner ()  const
  {
    return  (owner != 0);
  }



  template <class Entry>
  int32  KKQueue<Entry>::QueueSize ()  const    
  {
    return  (int32)this->size ();
  }



  template <class Entry>
  void  KKQueue<Entry>::DeleteContents ()  
  {
    if  (owner)
    {
      iterator  idx;

      for  (idx = KKQueue<Entry>::begin ();  idx != KKQueue<Entry>::end ();  idx++)
      {
        delete  (*idx);
      }
    }

    KKQueue<Entry>::clear ();
  }  /* DeleteContents */



  template <class Entry>
  inline  void  KKQueue<Entry>::Add (EntryPtr _entry)    
  {
    PushOnBack (_entry);
  }



  template <class Entry>
  inline  void   KKQueue<Entry>::AddFirst (EntryPtr _entry)    
  {
    PushOnFront (_entry);
  }



  template <class Entry>
  void    KKQueue<Entry>::AddQueue (const KKQueue& q)
  {
    for (const_iterator x = q.begin ();  x != q.end (); x++)
      PushOnBack (*x);
    return;
  }  /* AddQueue */




  template <class Entry>
  inline  typename  KKQueue<Entry>::EntryPtr   KKQueue<Entry>::GetFirst ()  const
  {
    return  FrontOfQueue ();
  }



  template <class Entry>
  inline  typename  KKQueue<Entry>::EntryPtr   KKQueue<Entry>::GetLast ()  const
  {
    return  BackOfQueue ();
  }




  template <class Entry>
  void  KKQueue<Entry>::PushOnFront (EntryPtr _entry)
  {
    this->insert (KKQueue<Entry>::begin (), _entry);

    Entry*  e = *KKQueue<Entry>::begin ();

    if  (e != _entry)
    {
      std::cout << "Error " << std::endl;
    }

  }


  template <class Entry>
  void  KKQueue<Entry>::PushOnBack  (EntryPtr _entry)
  {
    this->push_back (_entry);

    EntryPtr  e = KKQueue<Entry>::back ();

    if  (e != _entry)
    {
      std::cerr << "Error" << std::endl;
    }
    //vector<Entry*>  x;
  }




  template <class Entry>
  KKQueue<Entry>&   KKQueue<Entry>::operator= (const KKQueue& q)
  {
    DeleteContents ();

    if  (q.Owner ())
    {
      for  (const_iterator  x = q.begin ();  x < q.end (); x++)
        push_back (new Entry (**x));
      this->Owner (true);
    }
    else
    {
      for  (const_iterator  x = q.begin ();  x < q.end (); x++)
        push_back (*x);
      this->Owner (false);
    }
    return *this;
  }  /* operator= */




  template <class Entry>
  bool   KKQueue<Entry>::operator== (const KKQueue<Entry>& rightSide)  const
  {
    if  (QueueSize () != rightSide.QueueSize ())
      return false;

    for (int32 x = 0;  x < QueueSize ();  x++)
    {
      Entry*  left  = IdxToPtr (x);
      Entry*  right = rightSide.IdxToPtr (x);
      if  (!((*left) == (*right)))
        return false;
    }

    return true;
  } /* operator== */



  template <class Entry>
  bool   KKQueue<Entry>::operator!= (const KKQueue<Entry>& rightSide)  const
  {
    return  !((*this) == rightSide);
  } /* operator!= */


        
  /// 2011-01-27  Sergiy
  /// Fisher-Yates Uniform Random Sort.
  template <class Entry>
  void  KKQueue<Entry>::RandomizeOrder ()
  {
    int32  i, j;
    uint32  numEntries = (uint32)KKQueue<Entry>::size ();
    if  (numEntries < 2)
      return;

    i = numEntries - 1;
    while (true)  
    {
      j = LRand48() % (i + 1);
      SwapIndexes  (i, j );
      if  (i < 1)
        break;
      i--;
    }
  }  /* RandomizeOrder */





  template <class Entry>
  void  KKQueue<Entry>::RandomizeOrder (int64  seed)
  {
    SRand48 ( seed );
    RandomizeOrder ();
  }  /* RandomizeOrder */




  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr KKQueue<Entry>::PopFromFront ()
  {
    if  (KKQueue<Entry>::size () <= 0)
    {
      return NULL;
    }

    iterator beg = KKQueue<Entry>::begin ();
    Entry*  e = *beg;
    this->erase (beg);
    return  e;
  }


  template <class Entry>
  inline  typename  KKQueue<Entry>::EntryPtr   KKQueue<Entry>::RemoveFirst ()  
  {
    return  PopFromFront ();
  }



  template <class Entry>
  inline  typename  KKQueue<Entry>::EntryPtr  KKQueue<Entry>::RemoveLast  ()  
  {
    return  PopFromBack ();
  }



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr KKQueue<Entry>::LookAtBack ()  const
  {
    if  (this->size () <= 0)
      return NULL;

    return  KKQueue<Entry>::back ();
  }



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr KKQueue<Entry>::LookAtFront ()  const
  {
    if  (this->size () <= 0)
      return NULL;
    return  *KKQueue<Entry>::begin ();
  }



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr  KKQueue<Entry>::PopFromBack ()
  {
    if  (KKQueue<Entry>::size () <= 0)
      return NULL;

    Entry*  e = KKQueue<Entry>::back ();
    KKQueue<Entry>::pop_back ();
    return e;
  }



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr  KKQueue<Entry>::BackOfQueue ()  const
  {
    if  (KKQueue<Entry>::size () <= 0)
      return NULL;
    return KKQueue<Entry>::back ();
  }





  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr  KKQueue<Entry>::FrontOfQueue () const
  {
    if  ( KKQueue<Entry>::size () <= 0)
       return NULL;

    return * KKQueue<Entry>::begin ();
  }


  template <class Entry>
  void   KKQueue<Entry>::Compress ()
  {
    // Don't need to do anything anymore, the vector template is doing it for me.
  }  /* Compress */




  template <class Entry>
  void   KKQueue<Entry>::DeleteEntry (EntryPtr _entry)
  {
    for  (iterator y =  KKQueue<Entry>::begin (); y !=  KKQueue<Entry>::end ();  y++)
    {
      if  (*y == _entry)
      {
        KKQueue<Entry>::erase (y);
        return;
      }
    }
  }  /* DeleteEntry */




  template <class Entry>
  void   KKQueue<Entry>::SetIdxToPtr (uint32  _idx,
                                      Entry*  _ptr
                                     )
  {
    if  ((uint32)_idx >=  KKQueue<Entry>::size ())
      return;
    
    std::vector<Entry*>::operator[] (_idx) = _ptr;
  }  /* SetIdxToPtr */





  template <class Entry>
  void   KKQueue<Entry>::DeleteEntry (uint32 _idx)
  {
    if  ((uint32)_idx >=  KKQueue<Entry>::size ())
      return;

    iterator  i =  KKQueue<Entry>::begin ();

    for (uint32 j = 0;  ((j < _idx)  &&  (i !=  KKQueue<Entry>::end ()));  j++)
      i++;

    EntryPtr  ep = *i;
    KKQueue<Entry>::erase (i);
  }  /* DeleteEntry */




  template <class Entry>
  uint32 KKQueue<Entry>::GrowthRate () const
  {
    return growthRate;
  }



  template <class Entry>
  void  KKQueue<Entry>::GrowthRate  (uint32 _growthRate)
  {
    growthRate = _growthRate;
  }


  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr   KKQueue<Entry>::IdxToPtr (uint32 idx)  const
  {
    if  (idx >= KKQueue<Entry>::size ())
      return NULL;

    return  &((*this)[idx]);
    //return  (typename Entry*)(vector<Entry*>::operator[] (idx));
  }  /* IdxToPtr */




  template <class Entry>
  int32 KKQueue<Entry>::LocateEntry (EntryConstPtr _entry)  const
  {
    int32  i = 0; 

    for  (const_iterator j = KKQueue<Entry>::begin ();  j != KKQueue<Entry>::end (); j++)
    {
      if  (*j == _entry)
        return i;
      i++;
    }
    return -1;
  }  /* LocateEntry */





  template <class Entry>
  int32  KKQueue<Entry>::PtrToIdx (EntryConstPtr _entry)  const
  {
    return  LocateEntry (_entry);
  }




  template <class Entry>
  void  KKQueue<Entry>::SwapIndexes (size_t  idx1,  size_t idx2)
  {
    if  ((idx1 < 0)  ||  (idx1 >= KKQueue<Entry>::size ())  ||
         (idx2 < 0)  ||  (idx2 >= KKQueue<Entry>::size ())
        )
    {
      std::cerr << std::endl
        << "KKQueue::SwapIndexes  ***ERROR***  (KKQueue::  Indexes[" 
        << (int32)idx1 << ", " << (int32)idx2 << "] Out of Range;"
        << "  Size[" << (int32)KKQueue<Entry>::size () << "]."
        << std::endl << std::endl; 
      throw "KKQueue::SwapIndexes  ***ERROR***  Indexes out or range.";
    }

    //vector<typename Entry*>&  us (*this);
    EntryPtr  tempPtr = std::vector<Entry*>::operator[] (idx1);

    // (*this)[idx1];

    std::vector<EntryPtr>::operator[] (idx1) = std::vector<EntryPtr>::operator[] (idx2);
    std::vector<EntryPtr>::operator[] (idx2) = tempPtr;
    //(*this)[idx1] = (*this)[idx2];
    //(*this)[idx2] = tempPtr;
  }  /* SwapIndexes */





  template <class Entry>
  Entry&   KKQueue<Entry>::operator[] (uint32 idx)  const
  {
    //KKQueue<Entry>::Entry*  entry;

    if  ((idx < 0)  ||  (idx >= KKQueue<Entry>::size ()))
    {
      std::cerr << std::endl
        << "KKQueue<Entry>::operator[]   ***ERROR***   KKQueue<Entry>::operator[], Index [" << idx << "] out of bounds." << std::endl 
        << std::endl;
      throw "KKQueue<Entry>::operator[]   ***ERROR***   Index out of range.";
    }

    return  (Entry&)*(std::vector<Entry*>::operator[] (idx));
  }  /* operator[] */



}  /* namespace KKU; */




#endif
