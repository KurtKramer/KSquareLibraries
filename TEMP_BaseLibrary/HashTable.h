#ifndef  _HASHTABLE_
#define  _HASHTABLE_

#include  "BasicTypes.h"
#include  "Str.h"



#ifndef  _KKSTR_
Error,   You must include  "Str.h"  before  "HashTable.h"
#endif

namespace  KKU
{
  class    HashEntry;
  typedef  HashEntry*  HashEntryPtr;

  template <class Entry>
  class  HashTable
  {
  public: 
    typedef  Entry* EntryPtr;

  private:
    class  HashEntry;
    typedef  HashEntry*  HashEntryPtr;

    class  HashEntry
    {
    public:
      HashEntry (EntryPtr      _entry,
                 HashEntryPtr  _next
                ):
          entry (_entry),
          next  (_next)
        {}

      EntryPtr      entry;
      HashEntryPtr  next;
    private:
    };

    HashEntryPtr*  table;

    bool           owner;
    uint32         tableSize;
    uint32         numOfEntries;
    uint32         keyLen;

    int32*           hashFactors;

  public:
    HashTable (bool    _owner,  
               uint32  _tableSize,  
               uint32  _keyLen
              );
    
    ~HashTable ();

    void      AddEntry (EntryPtr  entry);

    uint32    HashValue (const KKStr&  str);

    bool      DeleteEntry (EntryPtr  _entry);

    EntryPtr  LocateEntry (const KKStr&  str);

    void      Save (const KKStr&  fileName);

    uint32         NumOfEntries ()  {return  numOfEntries;}

    HashEntryPtr*  Table ()         {return  table;}

    uint32         TableSize ()     {return  tableSize;}
  };  /* HashTable */



  /**
   *@brief  Hash Table management template; developed to support BitReduction algorithm.
   *@author  Tong Luo
   *@details  This template was originally written by Tong Luo to support his experiments in Bit reduction.
   *          It was later heavily modified by Kurt Kramer to conform to the PicesLibrary structure.
   */
  template <class Entry>
  HashTable<Entry>::HashTable (bool    _owner,
                               uint32  _tableSize,  
                               uint32  _keyLen
                              ):
      owner        (_owner),
      numOfEntries (0),
      tableSize    (_tableSize),
      keyLen       (_keyLen)
  {
    table = new HashEntryPtr[tableSize];
    uint32  x;
    for  (x = 0; x < tableSize; x++)
      table[x] = NULL;

    hashFactors = new int32[keyLen];

    int32  fact = 53;
    int32  hashVal = 1;

    int32  maxHashVal = _tableSize * 10; 

    for  (x = 0; x < keyLen; x++)
    {
      if  (hashVal > maxHashVal)
        hashVal = 1;

      hashFactors[x] = hashVal;
      hashVal = hashVal * fact;
    }
  };


  template <class Entry>
  HashTable<Entry>::~HashTable ()
  {
    HashEntryPtr  cur;
    uint32        idx;
    HashEntryPtr  next;

    for  (idx = 0; idx < tableSize; idx++)
    {
      next = table[idx];
      while  (next)
      {
        cur = next;
        next = next->next;
        if  (owner)
        {
          delete  cur->entry;
        }
        delete cur;
      }
    }

    delete  table;
    delete  hashFactors;
  }


  template <class Entry>
  typename HashTable<Entry>::EntryPtr  HashTable<Entry>::LocateEntry (const KKStr&  key)
  {
    uint32  hashValue = HashValue (key);

    // uint32  idx = CalcIndex (hashValue);
    uint32  idx =  hashValue;

    EntryPtr entry     = NULL;

    HashEntryPtr next = table[idx];

    while  ((!entry)  &&  next)
    {
      if  (next->entry->Key () == key)
      {
        entry = next->entry;
      }
      else
      {
        next = next->next;
      }
    }
    
    return  entry;
  }  /* LocateEntry */


  template <class Entry>
  void  HashTable<Entry>::AddEntry (EntryPtr  entry)
  {
    uint32  hashValue = HashValue (entry->Key ());

    HashEntryPtr  next = table[hashValue];

    if  (next)
    {
      //cout << "Duplicate Hash " << hashValue << endl;
    }

    table[hashValue] = new HashEntry (entry, next);

    numOfEntries++;
  }  /* AddEntry */


  template <class Entry>
  bool  HashTable<Entry>::DeleteEntry (EntryPtr  entry)
  {
    uint32  hashValue = HashValue (entry->Key ());

    HashEntryPtr next = table[hashValue];
    HashEntryPtr last = NULL;

    bool  deleted = false;

    while  (next)
    {
      if  (next == entry)
      {
        if  (last)
        {
          last->next = next->next;
        }
        else
        {
          table[hashValue] = next->next;
        }

        delete  next;
        next = NULL;
        numOfEntries--;
        deleted = true;
      }
      else
      {
        last = next;
        next = next->next;
      }
    }

    return  deleted;
  }  /* DeleteEntry */


  template <class Entry>
  uint32  HashTable<Entry>::HashValue (const KKStr&  str)
  {
    const char*  s = str.Str ();
    int32  len = strlen (s);
    int32 x = len;
    uint32  y = 0;
    long  hashVal = 0;

    while  ((x > 0)  &&  (y < keyLen))
    {
      x--;
      hashVal = hashVal + s[x] * hashFactors[y];
      y++;
    }

    hashVal = hashVal % tableSize;
    return  hashVal;
  }  /* HashValue */


  template <class Entry>
  void  HashTable<Entry>::Save (const KKStr&  fileName)
  {
    ofstream f (fileName.Str ());

    if  (!f)
    {
      cerr << endl;
      cerr << "*** ERROR ***,  Error Opening File[" << fileName << "]." << endl;
      cerr << endl;
      return;
    }

    HashEntryPtr  next = NULL;
    uint32  idx = 0;

    while  (idx < TableSize ())
    {
      next = (Table ())[idx];

      while  (next)
      {
        f << next->entry->ToString () << endl;
        next = next->next;
      }

      idx++;    
    }

    f.close ();
  }  /* Save */
}  /* KKU */



#endif
