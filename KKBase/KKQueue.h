/* KKQueue.h -- Double sided queue structure  sub-classed from vector template.
 * Copyright (C) 1994-2014 Kurt Kramer & Sergiy Fefilatyev
 * For conditions of distribution and use, see copyright notice in KKB.h
 *
 * 2011-06  Sergiy Fefilatyev found a bug in my 'KKB::KKQueue::RandomizeOrder' method.
 * 2011-10-21  Fixed bug in prev fix to 'KKB::KKQueue::RandomizeOrder'.     
 */
#ifndef  _KKU_KKQUEUE_
#define  _KKU_KKQUEUE_

#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdlib.h>
#include <sstream>
#include <vector>

#include "KKBaseTypes.h"
#include "KKException.h"
#include "RandomNumGenerator.h"


namespace  KKB
{
  /**
   *@brief  A typed container class/template that keeps track of entries via pointers only.
   *@details  Will act as an Array, Queue or Stack structure. Items are added by the 'PushOnFront' and 
   *          'PushOnBack' methods.  They are removed by the 'PopFromFront' and 'PopFromBack' methods.
   *          What is important to keep in mind is that it holds pointers to its contents, not the actual
   *          instances. It is important to keep track who owns the objects you put in an instance of 
   *          KKQueue. KKQueue has a 'Owner' flag that you can set. If it is set to 'true' then it will 
   *          call the destructor on all its contents when you call KKQueue's destructor or the 
   *          'DestroyContents' method. When you add an element to a KKQueue container you can not delete
   *          it separately until you either remove it from KKQueue or delete the KKQueue container.
   *          If the KKQueue derived container owned its contents and you call its destructor then there 
   *          is no need to delete the contents separately.
   *
   *          KKQueue is sub-classed from vector<Entry*> so you can use any method that is available for
   *          the vector<> template.
   *
   *@tparam  Entry  The type of objects that are to be held in this container.  When you add new instances 
   *                of 'Entry', you need to add a pointer to it not the actual entry.
   */
  template <class Entry>
  class  KKQueue: public std::vector<Entry*>
  {
    typedef  Entry* EntryPtr;
    typedef  Entry const * EntryConstPtr;

  private:
      bool  owner;       /**< if True the KKQueue structure owns the objects and is responsible for
                          *   deleting them when the KKQueue structure is deleted.
                          */

  public:
      typedef  typename std::vector<Entry*>::iterator        iterator;
      typedef  typename std::vector<Entry*>::const_iterator  const_iterator;

      typedef  KKB::OptionUInt32  OptionUInt32;

      KKQueue  (bool _owner = true);

  protected:
      /**
       *@brief  Copy Constructor creating new instance; including duplicating contents if owner set to true. 
       *@details If the parameter 'q' owns its contents then will create new instances of its contents.
       * That is it will call the Copy Constructor for each one of the elements it contains. If 'q' does
       * not own its contents it will just copy over the pointers from 'q', meaning that the new instance 
       * of KKQueue will point to the same locations as 'q' does.
       */
      KKQueue (const KKQueue&  q);

  public:
      /**
       *@brief Constructor, similar to the Copy Constructor except that you can control whether it duplicates the contents.
       *@details If the '_owner' parameter is set to true then it will create new instances of the contents otherwise it 
        will just point to the instances that already exist.
       */
      KKQueue (const KKQueue&  q,
               bool            _owner
              );


      /** @brief Virtual destructor; if owns its contents will also call the destructor on each one entry that it contains. */
      virtual ~KKQueue ();

      KKQueue*  DuplicateListAndContents () const;  /**< @brief  Creates a new container including duplicating the contents, which
                                                     * also makes the new instance the owner of those contents. 
                                                     */

      //  Access Methods that do not update the instance.
      EntryPtr  BackOfQueue  () const;   /**< Returns pointer of last element without removing it; if empty returns NULL.  */
      EntryPtr  FrontOfQueue () const;   /**< Returns pointer to first  element that is at from of the queue with out removing it from the queue.                  */
      EntryPtr  GetFirst     () const;   /**< Same as FrontOfQueue. */
      EntryPtr  GetLast      () const;   /**< Same as BackOfQueue.  */
      EntryPtr  LookAtBack   () const;   /**< Same as BackOfQueue.  */
      EntryPtr  LookAtFront  () const;   /**< Same as FrontOfQueue. */
      kkuint32  QueueSize    () const;   /**< Same as calling vector<>::size(); returns the number of elements in KKQueue  */
      bool      Owner        () const;

      OptionUInt32  LocateEntry  (EntryConstPtr _entry)  const;  /**< Returns index of the element who's address is '_entry'. If not found in container will return back -1.                */
      EntryPtr      IdxToPtr     (size_t        idx)     const;  /**< Returns pointer to the element with index 'idx'; if 'idx' out or range returns NULL.  */
      OptionUInt32  PtrToIdx     (EntryConstPtr _entry)  const;  /**< returns the index of the 'entry' that has the same pointer as '_entry', if none found returns -1 */

      // Basic Queue operators.
      virtual   void      Add          (EntryPtr _entry);    /**< same as PushOnBack   */
      virtual   void      AddFirst     (EntryPtr _entry);    /**< same as PushOnFront  */
      virtual   EntryPtr  PopFromFront ();                   /**< Removes first element and returns its pointer; if empty returns NULL.  */
      virtual   EntryPtr  PopFromBack  ();                   /**< Removes last element and returns its pointer; if empty will return NULL. */
      virtual   void      PushOnFront  (EntryPtr _entry);    /**< Adds '_entry' to the Front of the container. */
      virtual   void      PushOnBack   (EntryPtr _entry);    /**< Adds '_entry' to the End of the container. */
      virtual   EntryPtr  RemoveFirst  ();                   /**< same as PopFromFront  */
      virtual   EntryPtr  RemoveLast   ();                   /**< same as PopFromBack   */

      /**
       *@fn  void AddQueue (KKQueue& q);
       *@brief  Add the contents of a separate KKQueue container to this container.
       *@details  Be careful how the Owner flags are set; this method adds the pointers of 'q' to the end of its own container; it does not concern itself
       *          with the state of the 'Owner' flags. If you are not careful you can have both containers thinking they own the same entries. I suggest
       *          that after you add the contents of 'q' to this container that the caller set the 'owner' flag of 'q'  to false.
       */
      virtual  void  AddQueue (const KKQueue& q);                    

      virtual void   DeleteContents ();                       /**< Empties the container,  if 'owner' is set to true will call the destructor on each element.  */

      void      DeleteEntry    (EntryPtr _entry);             /**< Removes from KKQueue the entry who's pointer = '_entry'                                      */
      void      DeleteEntry    (size_t   _idx);               /**< Removes from KKQueue the entry who's index = '_idx'.                                         */

      void      Owner          (bool     _owner);             /**< specifies who owns the contents of the container; when true the contents will
                                                                *  be deleted when the container is deleted.  
                                                                */

      void      RandomizeOrder ();
      void      RandomizeOrder (kkint64   seed);
      void      RandomizeOrder (RandomNumGenerator&  randomVariable);

      void      SetIdxToPtr    (size_t _idx,
                                Entry* _ptr
                               );

      void      SwapIndexes    (size_t idx1,  size_t idx2);

      template<typename Functor>
      KKB::OptionUInt32  FindTheKthElement (kkuint32  k,
                                            Functor   pred
                                           );

      /**
       *@param[in] f Function taking pointer to [[Entry]] and returns 'true' if to be included in returned list.
       *@Returns sublist containing only elements where [[f]] indicates true. 
       */
      virtual  KKQueue<Entry>*  Filter(bool (*f)(const Entry* e)) const;


      template<typename T>
      KKQueue<T>*  Map(T* (*f)(const Entry* e)) const;


      Entry&    operator[] (size_t i)  const;       /**< Returns reference to element indexed by 'i'; similar to IdxToPtr 
                                                       * except returns reference rather than a pointer. 
                                                       */

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
      template<typename Functor>
      kkuint32  FindTheKthElement (kkuint32  k,
                                   kkuint32  left,
                                   kkuint32  right,
                                   Functor   pred,
                                   kkuint32* redirectionArray
                                  );

      template<typename Functor>
      kkint32   Partition (kkuint32  left,
                           kkuint32  right,
                           Functor   pred,
                           kkuint32* redirectionArray
                          );

  };  /* KKQueue */



  template <class Entry>
  KKQueue<Entry>::KKQueue (bool  _owner):
      owner (_owner)
  {
  }



  template <class Entry>
  KKQueue<Entry>::KKQueue (const KKQueue&  q):
        std::vector<Entry*>(q),
        owner (q.Owner ())

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
        owner (_owner)

  {
    //for  (vector<Entry*>::const_iterator x = q.begin
    //  owner (_owner) ();  x != q.end ();  x++)
    for  (const_iterator x = q.begin ();  x != q.end ();  ++x)
    {
      if  (owner)
        PushOnBack (new Entry (*(*x)));
      else
        PushOnBack (*x); 
    }
  }

  

  template <class Entry>
  KKQueue<Entry>::~KKQueue () 
  {
    if  (owner)
    {
      iterator  i;

      for  (i = KKQueue<Entry>::begin ();  i != KKQueue<Entry>::end (); i++)
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
  kkuint32  KKQueue<Entry>::QueueSize ()  const    
  {
    return  (kkint32)this->size ();
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
	KKQueue<Entry>::insert (KKQueue<Entry>::begin (), _entry);

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
      std::cerr << "KKQueue<Entry>::PushOnBack   ***ERROR***    Operation Failed!!!" << std::endl;
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

    for (kkint32 x = 0;  x < QueueSize ();  x++)
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



  /**
   *@brief  Randomizes the order of the vector.
   *@details  Implements the "Fisher-Yates Uniform Random Sort"; this implementation was done by Sergiy Fefilatyev (2011-01-27)
   */
  template <class Entry>
  void  KKQueue<Entry>::RandomizeOrder ()
  {
    size_t  i, j;
    size_t numEntries = KKQueue<Entry>::size ();
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
  void  KKQueue<Entry>::RandomizeOrder (kkint64  seed)
  {
    SRand48 (seed);
    RandomizeOrder ();
  }  /* RandomizeOrder */



  /**
   *@brief  Randomizes the order of the vector.
   *@details  Implements the "Fisher-Yates Uniform Random Sort"; this implementation was done by Sergiy Fefilatyev (2011-01-27)
   */
  template <class Entry>
  void  KKQueue<Entry>::RandomizeOrder (RandomNumGenerator&  randomVariable)
  {
    kkint32  i, j;
    kkuint32  numEntries = KKQueue<Entry>::size ();
    if  (numEntries < 2)
      return;

    i = numEntries - 1;
    while (true)  
    {
      j = randomVariable.Next () % (i + 1);
      SwapIndexes  (i, j );
      if  (i < 1)
        break;
      i--;
    }
  }  /* RandomizeOrder */


  
  template <class Entry>
  KKQueue<Entry>*    KKQueue<Entry>::DuplicateListAndContents ()  const
  {
    KKQueue<Entry>*  duplicatedQueue = new KKQueue<Entry> (true);

    for  (const_iterator x = KKQueue<Entry>::begin ();  x != KKQueue<Entry>::end ();  x++)
    {
      const EntryPtr e = *x;
      duplicatedQueue->PushOnBack (new Entry (*e));
    }
    
    return  duplicatedQueue;
  }  /* DuplicateListAndContents */



  template <class Entry>
  KKQueue<Entry>*   KKQueue<Entry>::Filter(bool (*f)(const Entry* e)) const
  {
    KKQueue<Entry>*  results = new KKQueue<Entry> (false);

    for (auto idx: *this)
    {
      bool zed = f(idx);
      if  (zed)
        results->PushOnBack(idx);
    }

    return results;
  }



  template <class Entry>
  template<typename T>
  KKQueue<T>*  KKQueue<Entry>::Map(T* (*f)(const Entry* e)) const
  {
    KKQueue<Entry>*  results = new KKQueue<T>(true);
    for (auto idx: *this)
    {
      auto newRow = f(idx);
      if  (newRow)
        results->PushOnBack(newRow);
    }

    return results;
  }



  template <class Entry>
  template <typename  Functor>
  KKB::OptionUInt32   KKQueue<Entry>::FindTheKthElement (kkuint32  k,
                                                         Functor   pred
                                                        )
  {
    kkuint32 qSize = QueueSize ();
    if  (k >= qSize)
      return {};

    kkuint32* redirectionArray = new kkuint32[qSize];
    for (kkuint32 x = 0;  x < qSize;  ++x)
      redirectionArray[x] = x;

    KKQueue<Entry>*  kkqueueObj = this;

    auto Comp = [kkqueueObj, redirectionArray, pred](kkuint32 x, kkuint32 y) -> bool 
    {
      auto pX = kkqueueObj->IdxToPtr(redirectionArray[x]);
      auto pY = kkqueueObj->IdxToPtr(redirectionArray[y]);
      return pred(pX, pY);
    };

    std::nth_element (redirectionArray, redirectionArray + k, redirectionArray + QueueSize (), Comp);
   
    auto  kthElementIdx = redirectionArray[k];

    //kkuint32 kthElementIdx = FindTheKthElement (k, 0, KKQueue<Entry>::size () - 1, pred, redirectionArray);

    delete redirectionArray;
    return  kthElementIdx;
  }  /* FindTheKthElement */

  

  template <class Entry>
  template <typename  Functor>
  kkuint32   KKQueue<Entry>::FindTheKthElement (kkuint32  k,
                                                kkuint32  left,
                                                kkuint32  right,
                                                Functor   pred,
                                                kkuint32* redirectionArray
                                               )
  {
    if  (left == right)
      return  left;

    kkint32 m = Partition (left, right, pred, redirectionArray);
    if  (k <= m)
      return  Partition (left, m, pred, redirectionArray);

    else if  (m < right)
      return  Partition (m + 1, right, pred, redirectionArray);

    else
    {
      // This should not be able to happen; but if it does then we have a flaw in the code or the logic somewhere,
      cerr << "KKQueue<Entry>::FindTheKthElement  ***ERROR***  An invalid situation just occurred." << endl
           <<"        k=" << k << ",  left=" << left << ", right=" << right << ", m=" << m          << endl;
      return  m - 1;
    }
  }  /* FindTheKthElement */



  template <class Entry>
  template <typename  Functor>
  kkint32   KKQueue<Entry>::Partition (kkuint32  left,
                                       kkuint32  right,
                                       Functor   pred,
                                       kkuint32* redirectionArray
                                      )
  {
    kkuint32  width = 1 + right - left;
    kkuint32  pivitIdx = left + (LRand48() % width);
    EntryPtr  pivitPtr = IdxToPtr (redirectionArray[pivitIdx]);

    while  (left < right)
    {
      while  ((left < right)  &&  (pred (*IdxToPtr(redirectionArray[left]), *pivitPtr)))
        ++left;

      while  ((left < right)  &&  (pred (*pivitPtr, *IdxToPtr(redirectionArray[right]))))
        --right;

      if  (left < right)
      {
        kkuint32 t = redirectionArray[left];
        redirectionArray[left] = redirectionArray[right];
        redirectionArray[right] = t;
      }
    }
    return left;
  }

  

  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr KKQueue<Entry>::PopFromFront ()
  {
    if  (KKQueue<Entry>::size () <= 0)
    {
      return NULL;
    }

    iterator beg = KKQueue<Entry>::begin ();
    Entry*  e = *beg;
    KKQueue<Entry>::erase (beg);
    return  e;
  }



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr   KKQueue<Entry>::RemoveFirst ()  
  {
    return  PopFromFront ();
  }



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr  KKQueue<Entry>::RemoveLast  ()  
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
  void  KKQueue<Entry>::SetIdxToPtr (size_t  _idx,
                                     Entry*  _ptr
                                    )
  {
    if  (_idx >=  KKQueue<Entry>::size ())
    {
      std::stringstream errMsg;

      errMsg << "Test";

      errMsg << _idx;

      auto zed = KKQueue<Entry>::size ();

      errMsg << zed;

      errMsg << "KKQueue<Entry>::SetIdxToPtr  _idx: " << _idx << " out of range: " << KKQueue<Entry>::size ();
      std::cerr << errMsg.str () << std::endl;
      throw std::exception (errMsg.str ().c_str ());
    }
    
    std::vector<Entry*>::operator[] (_idx) = _ptr;
  }  /* SetIdxToPtr */



  template <class Entry>
  void  KKQueue<Entry>::DeleteEntry (size_t _idx)
  {
    if  (_idx >=  KKQueue<Entry>::size ())
    {
      stringstream errMsg;
      errMsg << "KKQueue<Entry>::DeleteEntry  _idx: " << _idx << " out of range: " << KKQueue<Entry>::size ();
      std::cerr << errMsg.str () << std::endl;
      throw std::exception (errMsg.str ().c_str ());
    }

    iterator  i =  KKQueue<Entry>::begin ();

    for (kkuint32 j = 0;  ((j < _idx)  &&  (i !=  KKQueue<Entry>::end ()));  j++)
      i++;

    EntryPtr  ep = *i;
    KKQueue<Entry>::erase (i);
  }  /* DeleteEntry */



  template <class Entry>
  typename  KKQueue<Entry>::EntryPtr   KKQueue<Entry>::IdxToPtr (size_t idx)  const
  {
    if  (idx >= KKQueue<Entry>::size ())
      return NULL;

    return  &((*this)[idx]);
  }  /* IdxToPtr */



  template <class Entry>
  KKB::OptionUInt32 KKQueue<Entry>::LocateEntry (EntryConstPtr _entry)  const
  {
    kkint32  i = 0; 

    for  (const_iterator j = KKQueue<Entry>::begin ();  j != KKQueue<Entry>::end (); j++)
    {
      if  (*j == _entry)
        return i;
      i++;
    }
    return {};
  }  /* LocateEntry */



  template <class Entry>
  KKB::OptionUInt32  KKQueue<Entry>::PtrToIdx (EntryConstPtr _entry)  const
  {
    return  LocateEntry (_entry);
  }



  template <class Entry>
  void  KKQueue<Entry>::SwapIndexes (size_t idx1,  size_t idx2)
  {
    if  ((idx1 >= KKQueue<Entry>::size ()) || (idx2 >= KKQueue<Entry>::size ()))
    {
      std::stringstream errMsg;
      errMsg << "KKQueue<Entry>::SwapIndexes  idx1: " << idx1 << " and/or idx2: " << idx2
             << " exceeds QueueSize: " << KKQueue<Entry>::size ();
      std::cerr << errMsg.str () << std::endl;
      throw std::exception (errMsg.str ().c_str ());
    }

    EntryPtr  tempPtr = std::vector<Entry*>::operator[] (idx1);
    std::vector<EntryPtr>::operator[] (idx1) = std::vector<EntryPtr>::operator[] (idx2);
    std::vector<EntryPtr>::operator[] (idx2) = tempPtr;
  }  /* SwapIndexes */



  template <class Entry>
  Entry&   KKQueue<Entry>::operator[] (size_t idx)  const
  {
    //KKQueue<Entry>::Entry*  entry;

    if  (idx >= KKQueue<Entry>::size ())
    {
      std::stringstream errMsg;
      errMsg << "KKQueue<Entry>::operator[]  idx: " << idx << " exceeds QueueSize: " << KKQueue<Entry>::size ();
      std::cerr << errMsg.str () << std::endl;
      throw std::exception (errMsg.str ().c_str ());
    }

    return  (Entry&)*(std::vector<Entry*>::operator[] (idx));
  }  /* operator[] */

}  /* namespace KKB; */




#endif
