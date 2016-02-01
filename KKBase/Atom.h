/* Atom.h -- Experimental class that is not in use;  meant to be base class to all other classes.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _ATOM_
#define  _ATOM_

#include <ostream>


#include "KKBaseTypes.h"


namespace  KKB
{
#if !defined(_KKSTR_)
  class KKStr;
  typedef  KKStr*            KKStrPtr;
  typedef  KKStr const       KKStrConst;
  typedef  KKStrConst*       KKStrConstPtr;
  typedef  std::pair<KKStr,KKStr>  KKStrPair;
#endif

#if  !defined(_RUNLOG_)
  class RunLog;
#endif

#if  !defined(_XMLSTREAM_)
  class   XmlStream;
  typedef XmlStream*  XmlStreamPtr;
  class   XmlTag;
  typedef XmlTag*       XmlTagPtr;
  typedef XmlTag const  XmlTagConst;
  typedef XmlTagConst*  XmlTagConstPtr;
#endif


  /**
   *@class  Atom  Atom.h
   *@brief Base class of all other classes that are meant to be managed by 'KKBase'
   *@details  'Atom' will have a few important virtual methods that all derived classes will be required to implement. 
   *          This will allow for the smooth functioning of XML file reading and writing. Ex: 'WriteXML', this method 
   *          is to write a XML version of the derived class to a output stream. A registered BuildFromXML function will
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

    virtual  const char*  ClassName () const = 0;

    virtual
    Atom*    Duplicate ()  const = 0;

    virtual
    void     ReadXML (XmlStream&      s,
                      XmlTagConstPtr  tag,
                      VolConstBool&   cancelFlag,
                      RunLog&         log
                     ) = 0;

    virtual
    void     WriteXML (const KKStr&   varName,
                       std::ostream&  o
                      )  const = 0;

  private:  
    static  std::vector<AtomCreator>  registeredAtomCreators;
  };  /*  Atom */
}

#endif
