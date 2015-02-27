/* KKQueue.h -- Souble sided queue structure  sub-classed from vector template.
 * Copyright (C) 1994-2011 Kurt Kramer & Sergiy Fefilatyev
 * For conditions of distribution and use, see copyright notice in KKU.h
 *
 * 2011-06  Sergiy Fefilatyev found a bug in my 'KKU::KKQueue::RandomizeOrder' method.
 * 2011-10-21  Fixed bug in prev fix to 'KKU::KKQueue::RandomizeOrder'.     
 */
#ifndef  _KKU_KKQUEUECONST_
#define  _KKU_KKQUEUECONST_

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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

#include  "BasicTypes.h"


namespace  KKU
{
  template <class Entry>  class QueueComparison;



  /**
   *@class  KKQueueConst  Simlar to "KKQueue" template except contents will have 'const' qualifier.
   *@brief  A typed container class/template that keeps track of entries via pointers only while.
   *@details  Will act as an Array, Queue or Stack structures.  Items are added by the 'PushOnFront' and 
   *          'PushOnBack' methods.  They are removed by the 'PopFromFront' and 'PopFromBack' methods.
   *          What is important to keep in mind is that it holds pointers to its contents, not the actual
   *          instances. It is important to keep track who owns the objects you put in an instance of 
   *          KKQueue.  KKQueue has a 'Owner' flag that you can set.  If it is set to 'true' then it will 
   *          call the destructor on all its contents when you call KKQueue's destructor or the 
   *          'DestroyContents' method.  When you add an element to a KKQueue container you can not delete
   *          it separately until you either remove it from KKQueue or delete the KKQueue container.
   *          If the KKQueue derived container owned its contents and you call its destructor then there 
   *          is no need to delete the contents separately.
   *
   *          KKQueue is sub-classed from vector<Entry*> so you can use any method that is available for
   *          the vector<> template.
   *
   *@tparam[in]  Entry  The type of objects that are to be held in this container.  When you add new instances 
   *                    of 'Entry', you need to add a pointer to it not the actual entry.
   */
  template <class Entry>
  class  KKQueueConst: public std::vector<Entry const *>
  {
    typedef  Entry *       EntryPtr;
    typedef  Entry const * EntryConstPtr;

  public:
    typedef  KKU::int32   int32;
    typedef  KKU::uint32  uint32;


  public:
      typedef  typename std::vector<Entry const *>::iterator        iterator;
      typedef  typename std::vector<Entry const *>::const_iterator  const_iterator;

      KKQueueConst  ();

  protected:
      /**
       *@brief  Copy Constructor, will create a new instance.
       */
      KKQueueConst (const KKQueueConst&  q);

  public:

      ~KKQueueConst ();


      /**
       *@brief  Add the contents of a separate KKQueue container to this container.
       *@details  Be careful how the Owner flags are set.  This method adds the pointers of 'q' to the end of its own container. It does not concern itself
       *          with the state of the 'Owner' flags.  If you are not careful you can have both containers thinking they own the same entries.  I suggest
       *          that after you add the contents of 'q' to this container that the caller set the 'owner' flag of 'q'  to false.
       */
      void      AddQueue       (const KKQueueConst& q);                    
                                                                   
      EntryConstPtr  BackOfQueue    ()                       const;  /**< @brief  Returns pointer of last element in the container without removing it.  If the container is empty will return NULL.  */
      void           DeleteContents ();                              /**< @brief  Empties the container.                                                                                              */
      void           DeleteEntry    (EntryConstPtr _entry);          /**< @brief  Removes from KKQueueConst the entry who's pointer = '_entry'                                                             */
      void           DeleteEntry    (uint32   _idx);                 /**< @brief  Removes from KKQueueConst the entry who's index = '_idx'.                                                                */
      EntryConstPtr  FrontOfQueue   ()                       const;  /**< @brief  Returns a pointer to the element that is at from of the queue with out removing it from the queue.                  */
      EntryConstPtr  IdxToPtr       (uint32   idx)           const;  /**< @brief  Returns back a pointer to the element who's index is 'idx'. If 'idx' is less than 0 or >= QueueSize()  will return NULL.               */

      int32          LocateEntry    (EntryConstPtr _entry)   const;  /**< @brief  Returns the index of the element who's address is '_entry'. If not found in container will return back -1.                             */
      void           PushOnFront    (EntryConstPtr _entry);          /**< @brief  Adds '_entry' to the Front of the container.                                                      */
      void           PushOnBack     (EntryConstPtr _entry);          /**< @brief  Adds '_entry' to the End of the container.                                                        */
      EntryConstPtr  PopFromFront   ();                              /**< @brief  Removes the first element in the container and returns its pointer.  If the container is empty will return NULL.  */
      EntryConstPtr  PopFromBack    ();                              /**< @brief  Removes the last element in the container and returns its pointer. If the container is empty will return NULL.    */
      int32          PtrToIdx       (EntryConstPtr _entry)   const;  /**< @brief  returns the index of the 'entry' that has the same pointer as '_entry', if none found returns -1 */
      void           RandomizeOrder ();
      void           RandomizeOrder (int64     seed);
      int32          QueueSize      ()                       const;  /**< @brief  Same as calling vector<>::size().  Returns the number of elements in KKQueue  */
      //void      Sort           (QueueComparison<Entry>*  comparison);

      void           SetIdxToPtr    (uint32         _idx,
                                     Entry const *  _ptr
                                    );

      void           SwapIndexes    (size_t idx1,  size_t idx2);

      const Entry&   operator[] (uint32 i)      const;                 /**< @brief  Returns a reference to element indexed by 'i'  */


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
      KKQueueConst&  operator=  (const KKQueueConst& q);                        
      
      bool      operator== (const KKQueueConst<Entry>& rightSide)   const;  /**< @brief  Returns True if every entry in both containers point to the same elements */
      bool      operator!= (const KKQueueConst<Entry>& rightSide)   const;  /**< @brief  returns False if NOT every entry in both containers point to the same elements */


  private:
  };  /* KKQueueConst */




  template <class Entry>
  class  KKQueueConstComparison
  {
     typedef  Entry const * EntryConstPtr;

   public:
     KKQueueConstComparison () {}

     
     bool  operator () (const EntryConstPtr&  p1, const EntryConstPtr&  p2)
     {
       return  (p1 > p2);
     }

     //virtual  bool  LessOrEqual    (EntryPtr  p1,  EntryPtr  p2) = 0;

     //virtual  bool  GreaterOrEqual (EntryPtr  p1,  EntryPtr  p2) = 0;
  };



  template <class Entry>
  KKQueueConst<Entry>::KKQueueConst ()
  {
  }



  template <class Entry>
  KKQueueConst<Entry>::KKQueueConst (const KKQueueConst&  q)
  {
    for  (const_iterator x = q.begin ();  x != q.end ();  x++)
    {
      PushOnBack (*x); 
    }
  }



  template <class Entry>
  KKQueueConst<Entry>::~KKQueueConst () 
  {
  }  /* ~KKQueue */




  template <class Entry>
  int32  KKQueueConst<Entry>::QueueSize ()  const    
  {
    return  (int32)this->size ();
  }



  template <class Entry>
  void  KKQueueConst<Entry>::DeleteContents ()  
  {
    KKQueueConst<Entry>::clear ();
  }  /* DeleteContents */



  template <class Entry>
  void    KKQueueConst<Entry>::AddQueue (const KKQueueConst& q)
  {
    for (const_iterator x = q.begin ();  x != q.end (); x++)
      PushOnBack (*x);
    return;
  }  /* AddQueue */




  template <class Entry>
  void  KKQueueConst<Entry>::PushOnFront (EntryConstPtr _entry)
  {
    this->insert (KKQueueConst<Entry>::begin (), _entry);

    Entry const *  e = *KKQueueConst<Entry>::begin ();

    if  (e != _entry)
    {
      std::cout << "Error " << std::endl;
    }
  }


  template <class Entry>
  void  KKQueueConst<Entry>::PushOnBack  (EntryConstPtr _entry)
  {
    this->push_back (_entry);

    EntryConstPtr  e = KKQueueConst<Entry>::back ();

    if  (e != _entry)
    {
      std::cerr << "Error" << std::endl;
    }
    //vector<Entry*>  x;
  }




  template <class Entry>
  KKQueueConst<Entry>&   KKQueueConst<Entry>::operator= (const KKQueueConst& q)
  {
    DeleteContents ();

    for  (const_iterator  x = q.begin ();  x < q.end (); x++)
      push_back (*x);
    this->Owner (false);

    return *this;
  }  /* operator= */




  template <class Entry>
  bool   KKQueueConst<Entry>::operator== (const KKQueueConst<Entry>& rightSide)  const
  {
    if  (QueueSize () != rightSide.QueueSize ())
      return false;

    for (int32 x = 0;  x < QueueSize ();  x++)
    {
      Entry const *  left  = IdxToPtr (x);
      Entry const *  right = rightSide.IdxToPtr (x);
      if  (!((*left) == (*right)))
        return false;
    }

    return true;
  } /* operator== */



  template <class Entry>
  bool   KKQueueConst<Entry>::operator!= (const KKQueueConst<Entry>& rightSide)  const
  {
    return  !((*this) == rightSide);
  } /* operator!= */


        
  /// 2011-01-27  Sergiy
  /// Fisher-Yates Uniform Random Sort.
  template <class Entry>
  void  KKQueueConst<Entry>::RandomizeOrder ()
  {
    int32  i, j;
    uint32  numEntries = (uint32)KKQueueConst<Entry>::size ();
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
  void  KKQueueConst<Entry>::RandomizeOrder (int64  seed)
  {
    SRand48 (seed);
    RandomizeOrder ();
  }  /* RandomizeOrder */




  template <class Entry>
  typename  KKQueueConst<Entry>::EntryConstPtr KKQueueConst<Entry>::PopFromFront ()
  {
    if  (KKQueueConst<Entry>::size () <= 0)
    {
      return NULL;
    }

    iterator beg = KKQueueConst<Entry>::begin ();
    Entry  const *  e = *beg;
    this->erase (beg);
    return  e;
  }



  template <class Entry>
  typename  KKQueueConst<Entry>::EntryConstPtr  KKQueueConst<Entry>::PopFromBack ()
  {
    if  (KKQueueConst<Entry>::size () <= 0)
      return NULL;

    Entry const *  e = KKQueueConst<Entry>::back ();
    KKQueueConst<Entry>::pop_back ();
    return e;
  }



  template <class Entry>
  typename  KKQueueConst<Entry>::EntryConstPtr  KKQueueConst<Entry>::BackOfQueue ()  const
  {
    if  (KKQueueConst<Entry>::size () <= 0)
      return NULL;
    return KKQueueConst<Entry>::back ();
  }




  template <class Entry>
  typename  KKQueueConst<Entry>::EntryConstPtr  KKQueueConst<Entry>::FrontOfQueue () const
  {
    if  (KKQueueConst<Entry>::size () <= 0)
       return NULL;

    return * KKQueueConst<Entry>::begin ();
  }





  template <class Entry>
  void   KKQueueConst<Entry>::DeleteEntry (EntryConstPtr _entry)
  {
    for  (iterator y =  KKQueueConst<Entry>::begin (); y !=  KKQueueConst<Entry>::end ();  y++)
    {
      if  (*y == _entry)
      {
        KKQueueConst<Entry>::erase (y);
        return;
      }
    }
  }  /* DeleteEntry */




  template <class Entry>
  void   KKQueueConst<Entry>::SetIdxToPtr (uint32         _idx,
                                           Entry const *  _ptr
                                          )
  {
    if  ((uint32)_idx >=  KKQueueConst<Entry>::size ())
      return;
    
    std::vector<Entry const *>::operator[] (_idx) = _ptr;
  }  /* SetIdxToPtr */





  template <class Entry>
  void   KKQueueConst<Entry>::DeleteEntry (uint32 _idx)
  {
    if  ((uint32)_idx >=  KKQueueConst<Entry>::size ())
      return;

    iterator  i =  KKQueueConst<Entry>::begin ();

    for (uint32 j = 0;  ((j < _idx)  &&  (i != KKQueueConst<Entry>::end ()));  j++)
      i++;

    EntryConstPtr  ep = *i;
    KKQueueConst<Entry>::erase (i);
  }  /* DeleteEntry */



  template <class Entry>
  typename  KKQueueConst<Entry>::EntryConstPtr   KKQueueConst<Entry>::IdxToPtr (uint32 idx)  const
  {
    if  (idx >= KKQueueConst<Entry>::size ())
      return NULL;

    return  &((*this)[idx]);
  }  /* IdxToPtr */




  template <class Entry>
  int32  KKQueueConst<Entry>::LocateEntry (EntryConstPtr _entry)  const
  {
    int32  i = 0; 

    for  (const_iterator j = KKQueueConst<Entry>::begin ();  j != KKQueueConst<Entry>::end (); j++)
    {
      if  (*j == _entry)
        return i;
      i++;
    }
    return -1;
  }  /* LocateEntry */





  template <class Entry>
  int32  KKQueueConst<Entry>::PtrToIdx (EntryConstPtr _entry)  const
  {
    return  LocateEntry (_entry);
  }




  template <class Entry>
  void  KKQueueConst<Entry>::SwapIndexes (size_t  idx1,  size_t idx2)
  {
    if  ((idx1 < 0)  ||  (idx1 >= KKQueueConst<Entry>::size ())  ||
         (idx2 < 0)  ||  (idx2 >= KKQueueConst<Entry>::size ())
        )
    {
      std::cerr << std::endl;

      std::cerr << "  ***  ERROR  ***  (KKQueue::SwapIndexes)  Indexes[" 
                << (int32)idx1 << ", " << (int32)idx2 << "] Out of Range."
                << std::endl;

      std::cerr << "                   KKQueue Size[" << (int32)KKQueue<Entry>::size () << "]." 
                << std::endl
                << std::endl;

      exit (1);
    }

    EntryConstPtr  tempPtr = std::vector<Entry const *>::operator[] (idx1);

    std::vector<EntryConstPtr>::operator[] (idx1) = std::vector<EntryConstPtr>::operator[] (idx2);
    std::vector<EntryConstPtr>::operator[] (idx2) = tempPtr;
  }  /* SwapIndexes */





  template <class Entry>
  const Entry&   KKQueueConst<Entry>::operator[] (uint32 idx)  const
  {
    //KKQueue<Entry>::Entry*  entry;

    if  ((idx < 0)  ||  (idx >= KKQueueConst<Entry>::size ()))
    {
      std::cerr << std::endl 
                << "*** ERROR ***   KKQueue<Entry>::operator[], Index [" << idx << "] out of bounds." << std::endl
                << std::endl;
      exit (-1);
    }

    return  (const Entry&)*(std::vector<Entry const *>::operator[] (idx));
  }  /* operator[] */



}  /* namespace KKU; */




#endif
