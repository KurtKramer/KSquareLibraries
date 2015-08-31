/* KKStrMatrix.cpp -- 2D Matrix of Strings.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _KKSTRMATRIX_
#define  _KKSTRMATRIX_

#include <ostream>
#include <string>

#ifdef  WIN32
#else
#define  __cdecl  
#endif

#include "KKBaseTypes.h"
#include "KKStr.h"
#include "KKStrMatrix.h"

#define  EnterChar   13
#define  EscapeChar  27


namespace KKB
{
  /**
   *@class  KKStrMatrix
   *@brief  A two dimensional matrix of Strings.
   */
  class  KKStrMatrix
  {
  public:
    typedef  KKStrMatrix*  KKStrMatrixPtr;


    KKStrMatrix (kkuint32 _numCols):  
        data    (), 
        numCols (_numCols) 
    {
    }

        
    KKStrMatrix (kkuint32 _numCols,
                 kkuint32 _numRows
                ):  
        numCols (_numCols) 
    {
      while  ((kkuint32)data.QueueSize () < _numRows)
        AddRow ();
    }


    ~KKStrMatrix ()  
    {
    }


    kkuint32  NumRows ()  const  {return data.QueueSize ();}
    kkuint32  NumCols ()  const  {return numCols;}


    KKStrList&  operator[] (kkuint32 row);

    KKStrList&  operator() (kkuint32 row);

    KKStr&  operator() (kkuint32  row,
                        kkuint32  col
                       );


    /**
     *@brief  Adds a list of Strings to the end of the Matrix.
     *@details Will add another row to the matrix and populate its contents with the String List being passed in.
     *@param[in] newRowData  Will take ownership of this String list.
    */
    void  AddRow (KKStrListPtr  newRowData);


    /**
     *@brief  Adds a row of empty string to the end of the matrix.
    */
    void  AddRow ();


  private:
    KKQueue<KKStrList>   data;
    kkuint32             numCols;
  };  /* KKStrMatrix */



  typedef  KKStrMatrix::KKStrMatrixPtr  KKStrMatrixPtr;

}  /* namespace KKB; */

#endif
