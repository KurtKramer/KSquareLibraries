#ifndef  _HASHTABLE_
#define  _HASHTABLE_

#include  "KKBaseTypes.h"
#include  "KKStr.h"



#ifndef  _KKSTR_
Error,   You must include  "KKStr.h"  before  "HashTable.h"
#endif

namespace  KKB
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
    kkuint32       tableSize;
    kkuint32       numOfEntries;
    kkuint32       keyLen;

    kkint32*           hashFactors;

  public:
    HashTable (bool    _owner,  
               kkuint32  _tableSize,  
               kkuint32  _keyLen
              );
    
    ~HashTable ();

    void      AddEntry (EntryPtr  entry);

    kkuint32  HashValue (const KKStr&  str);

    bool      DeleteEntry (EntryPtr  _entry);

    EntryPtr  LocateEntry (const KKStr&  str);

    void      Save (const KKStr&  fileName);

    kkuint32       NumOfEntries ()  {return  numOfEntries;}

    HashEntryPtr*  Table ()         {return  table;}

    kkuint32       TableSize ()     {return  tableSize;}
  };  /* HashTable */



  /**
   *@brief  Hash Table management template; developed to support BitReduction algorithm.
   *@author  Tong Luo
   *@details  This template was originally written by Tong Luo to support his experiments in Bit reduction.
   *          It was later heavily modified by Kurt Kramer to conform to the PicesLibrary structure.
   */
  template <class Entry>
  HashTable<Entry>::HashTable (bool    _owner,
                               kkuint32  _tableSize,  
                               kkuint32  _keyLen
                              ):
      owner        (_owner),
      numOfEntries (0),
      tableSize    (_tableSize),
      keyLen       (_keyLen)
  {
    table = new HashEntryPtr[tableSize];
    kkuint32  x;
    for  (x = 0; x < tableSize; x++)
      table[x] = NULL;

    hashFactors = new kkint32[keyLen];

    kkint32  fact = 53;
    kkint32  hashVal = 1;

    kkint32  maxHashVal = _tableSize * 10; 

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
    kkuint32      idx;
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
    kkuint32  hashValue = HashValue (key);

    // kkuint32  idx =  CalcIndex (hashValue);
    kkuint32  idx =  hashValue;

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
    kkuint32  hashValue = HashValue (entry->Key ());

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
    kkuint32  hashValue = HashValue (entry->Key ());

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
  kkuint32  HashTable<Entry>::HashValue (const KKStr&  str)
  {
    const char*  s = str.Str ();
    kkint32  len = strlen (s);
    kkint32 x = len;
    kkuint32  y = 0;
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
    kkuint32  idx = 0;

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
}  /* KKB */



#endif
