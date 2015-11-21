/* Atom.h -- Experimental class that is not in use;  meant to be base class to all other classes.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _ATOM_
#define  _ATOM_

#include <ostream>
using namespace std;

namespace  KKB
{

#if  !defined(_XMLSTREAM_)
  class  XmlStream;
  typedef  XmlStream*  XmlStreamPtr;
#endif


  /**
   *@class  Atom  Atom.h
   *@brief Base class of all other classes that is meant to be managed by 'KKBase'
   *@details  'Atom' will have a few important virtual methods that all derived classes will be
   *          required to implement.  This will allow for the smooth functioning of XML
   *          file reading and writing.  Ex: 'WriteXML',  this method is to write a XML version
   *          of the derived class to a output stream.  A registered BuildFromXML function will
   *          be able to create a new instance of the derived class.
   *
   *          Create on 2010-02-22;  primary purpose is to help generate more ideas along these
   *          lines.
   */
  class Atom
  {
  public:
    typedef  Atom*  AtomPtr;

    typedef  AtomPtr  (*AtomCreator) (XmlStream& i);

    Atom ();
    virtual  ~Atom ();

    virtual  void  WriteXML (ostream&  o) = 0;
    virtual  const char*  ClassName () = 0;

    //static  AtomPtr  BuildFromXML (XmlStream&  i);
  

  private:  
    static  vector<AtomCreator>  registeredAtomCreators;
  };  /*  Atom */
}

#endif

