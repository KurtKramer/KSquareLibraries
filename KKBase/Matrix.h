/* Matrix.h -- A simple two dimensional floating point matrix.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _MATRIX_
#define  _MATRIX_
//****************************************************************************************
//*                                   Matrix Class                                       *
//*                                                                                      *
//*                                                                                      *
//*  Supports two dimensional matrices.                                                  *
//*                                                                                      *
//*  Developed for Machine Learning Project for support of Import Vector Machine.        *
//*  Handles two dimensional matrixes.  Functions supported are matrix addition, sub-    *
//*  traction, multiplication, transpose, determinant, and inversion.  Where appropriate *
//*  arithmetic operators +, -, * were overloaded.  Addition, Subtraction and Multipli-  *
//*  cation can be done against either another matrix or scaler.                         *
//*                                                                                      *
//*======================================================================================*
//*                                                                                      *
//*     Date      Descriptions                                                           *
//*  ===========  ====================================================================== *
//*  Nov-11-2002  Initial Development.                                                   *
//*                                                                                      *
//****************************************************************************************

#include <fstream>

#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStr.h"
#include "OSservices.h"

namespace  KKB
{
  template<typename T>
  class  Row;

  /**
   *@class  Matrix
   *@brief  Supports two dimensional matrices.
   *@details  Developed for Machine Learning Project for support of Import Vector Machine.
   *  Handles two dimensional matrices.  Functions supported are matrix addition, subtraction
   *  multiplication, transpose, determinant, and inversion.  Where appropriate arithmetic
   *  arithmetic operators +, -, * were overloaded.  Addition, Subtraction and Multiplication
   *  can be done against either another matrix or scaler.  Also Transpose and Determinant
   *  operations are supported.
   */
  template<typename T>
  class  Matrix
  {
  public:
    typedef  Matrix*  MatrixPtr;
    typedef  Row<T>*  RowPtr;

    Matrix ();

    Matrix (kkuint32  _numOfRows,
            kkuint32  _numOfCols
           );

    Matrix (const Matrix&  _matrix);

    Matrix (Matrix&&  _matrix);

    Matrix (const std::vector<T>&  _v);

    ~Matrix () {Destroy ();}

    template<typename U>
    static  MatrixPtr  BuildFromArray (kkuint32 numOfRows,
                                       kkuint32 numOfCols,
                                       U**   data
                                      );

    Matrix<T>&  operator=  (const Matrix<T>&  right);

    template<typename U>
    Matrix<T>&  operator=  (const std::vector<U>&  right);

    template<typename U>
    Matrix<T>&  operator*= (U  right);

    template<typename U>
    Matrix<T>&  operator+= (U  right);
  
    Matrix<T>&  operator+= (const Matrix<T>&  right);

    Matrix<T>   operator+  (const Matrix<T>&  right);

    Matrix<T>   operator+  (T  right);

    Matrix<T>   operator-  (const Matrix<T>&  right);  

    Matrix<T>   operator-  (T right);

    Matrix<T>   operator*  (const Matrix<T>&  right);

    Matrix<T>   operator*  (T right);

    Row<T>&     operator[] (kkuint32  rowIDX) const;

    friend  Matrix<T>  operator- (T left, const Matrix<T>& right);


    Matrix<T>*     CalcCoFactorMatrix ();

    /**
     *@brief  Returns a Covariance matrix.
     *@details  Each column represents a variable and each row represents an instance of each variable.
     *@return  Returns a symmetric matrix that will be (numOfRows x numOfRows) where each element will represent the covariance 
     *         between their respective variables.
     */
    Matrix<T>*      Covariance ()  const;

    T const * const *   Data ()  const  {return  data;}

    T*              DataArea () { return dataArea; }

    T const *       DataAreaConst () const { return dataArea; }

    T**             DataNotConst ()  {return data;}

    T               Determinant () const;

    T               DeterminantSlow () const;  /**<  @brief Recursive Implementation. */

    void            EigenVectors (Matrix<T>*&       eigenVectors,
                                  std::vector<T>*&  eigenValues
                                 )  const;

    /** @brief  Locates the maximum value in a matrixw1 along with the row and column that is located. */
    void            FindMaxValue (T&         maxVal, 
                                  kkuint32&  row, 
                                  kkuint32&  col
                                 );

    std::vector<T>  GetCol (kkuint32 col)  const;

    Matrix<T>       Inverse ();

    kkuint32        NumOfCols () const  {return numOfCols;}

    kkuint32        NumOfRows () const  {return numOfRows;}

    void            ReSize (kkuint32 _numOfRows,
                            kkuint32 _numOfCols
                           );

    bool            Symmetric ()  const;  /**< Returns true is the matrix is Symmetric */

    Matrix<T>       Transpose ();

    friend  std::ostream&  operator<< (      std::ostream&  os, 
                                       const Matrix<T>&     matrix
                                      );

  private:
    void  Destroy ();

    void  AllocateStorage ();

    T  DeterminantSwap (T**       mat, 
                        kkuint32  offset
                       ) const;

    T  CalcDeterminent (kkuint32*  rowMap,
                        kkuint32*  colMap,
                        kkuint32   size
                       );

    T  Pythag (const T a,
               const T b
              )  const;

    kkint32  Tqli (T*        d, 
                   T*        e,
                   kkuint32  n,
                   T**       z
                  )  const;

    void  Tred2 (T**       a, 
                 kkuint32  n, 
                 T*        d, 
                 T*        e
                )  const;

    T**       data;       /**< A two dimensional array that will index into 'dataArea'.       */
    T*        dataArea;   /**< one dimensional array that will contain all the matrices data. */
    kkuint32  alignment;
    kkuint32  numOfCols;
    kkuint32  numOfRows;
    RowPtr    rows;
    kkuint32  totNumCells; /**<  Total number of cells allocated  = (numOfRows x nmumOfCols). */
  };  /* matrix */


  typedef  Matrix<double> MatrixD;
  typedef  MatrixD*  MatrixDPtr;

  typedef  Matrix<float> MatrixF;

  typedef  Matrix<float>*   MatrixFPtr;
  typedef  Matrix<double>*  MatrixDPtr;

  void  MultiplyMatrix (const Matrix<float>&  a, const Matrix<float>&  b, Matrix<float>&  c);

  void  MultiplyMatrix (const Matrix<double>& a, const Matrix<double>& b, Matrix<double>& c);


  template<typename T>
  Matrix<T>  operator- (T left, const Matrix<T>& right);


  template<typename T>
  std::ostream&  operator<< (std::ostream&     os,
                             const Matrix<T>&  matrix
                            );



  template<typename T>
  class  Row
  {
  public:
     Row  ();
 
     Row (kkuint32  _numOfCols,
          T*        _cells
         );

     Row (const Row&  _row);

     ~Row ();


     T*  Cols ()  {return cells;}

     T&  operator[] (kkuint32  idx);

     void  Define (kkuint32  _numOfCols,
                   T*        _cells
                  );

  private:
    T*  cells;
    kkuint32  numOfCols;
  };  /* Row */

  typedef Row<double> RowD;
  typedef Row<float> RowF;


  template<typename T>
  Row<T>::Row (): cells (NULL), numOfCols (0) { }


  template<typename T>
  Row<T>::Row (kkuint32 _numOfCols,  T* _cells): cells (_cells), numOfCols (_numOfCols) {}

  

  template<typename T>
  Row<T>::Row (const Row&  _row): cells (_row.cells), numOfCols (_row.numOfCols) {}



  template<typename T>
  Row<T>::~Row () {cells = NULL;}



  template<typename T>
  void  Row<T>::Define (kkuint32  _numOfCols, T* _cells)
  {
    numOfCols = _numOfCols;
    cells = _cells;
  }



  template<typename T>
  T&  Row<T>::operator[] (kkuint32  idx)
  {
    if (idx >= numOfCols)
    {
      KKStr errMsg(80);
      errMsg << "Row::operator[]  **** ERROR ****,  Index["
        << idx << "]  out of range of [0-" << numOfCols << "]."
        << std::endl;
      std::cerr << errMsg << std::endl;
      throw new KKException(errMsg);
    }

    return  cells[idx];
  }  /*  Row::operator[] */



  template<typename T>
  Matrix<T>::Matrix ():

    alignment   (64),
    data        (NULL),
    dataArea    (NULL),
    numOfCols   (0),
    numOfRows   (0),
    rows        (NULL),
    totNumCells (0)
  {
  }



  template<typename T>
  Matrix<T>::Matrix (kkuint32  _numOfRows,
                     kkuint32  _numOfCols
                    ):
    alignment   (64),
    data        (NULL),
    dataArea    (NULL),
    numOfCols   (_numOfCols),
    numOfRows   (_numOfRows),
    rows        (NULL),
    totNumCells (0)

  {
    AllocateStorage ();
  }  /*  Matrix::Matrix  */



  template<typename T>
  Matrix<T>::Matrix (const Matrix&  _matrix) :
    alignment   (_matrix.alignment),
    data        (NULL),
    dataArea    (NULL),
    numOfCols   (_matrix.numOfCols),
    numOfRows   (_matrix.numOfRows),
    rows        (NULL),
    totNumCells (0)
  {
    AllocateStorage ();
    memcpy (dataArea, _matrix.dataArea, totNumCells * sizeof (T));
  } /* Matrix::Matrix  */



  template<typename T>
  Matrix<T>::Matrix (Matrix&&  _matrix) :
    alignment   (_matrix.alignment),
    data        (_matrix.data),
    dataArea    (_matrix.dataArea),
    numOfCols   (_matrix.numOfCols),
    numOfRows   (_matrix.numOfRows),
    rows        (_matrix.rows),
    totNumCells (_matrix.totNumCells)
  {
    _matrix.data     = NULL;
    _matrix.dataArea = NULL;
    _matrix.rows     = NULL;
  }



  template<typename T>
  Matrix<T>::Matrix (const std::vector<T>&  _v) :
    alignment (64),
    data      (NULL),
    dataArea  (NULL),
    numOfCols (1),
    numOfRows (kkuint32 (_v.size ())),
    rows      (NULL)
  {
    AllocateStorage ();
    for (kkuint32 row = 0; row < numOfRows; ++row)
      dataArea[row] = _v[row];
  } /* Matrix::Matrix  */



  /**
   *@brief  Creates new matrix using the 2dArray "data" for source.
   *@details  If any row in "data" is equal to NULL then the corresponding row
   * in the returned matrix will be initialized to zeros.
   *@param[in]  numOfRows  Number of rows in resultant matrix.
   *@param[in]  numOfCols  Number of cols if resultant matrix.
   *@param[in]  data       A two dimensional array.
   *@return  A matrix that is initialized with the contents of "data".
  */
  template<typename T>
  template<typename U>
  Matrix<T>*  Matrix<T>::BuildFromArray (kkuint32 numOfRows,
                                         kkuint32 numOfCols,
                                         U**      data
                                        )
  {
    if ((numOfRows < 1) || (numOfCols < 1))
    {
      cerr << std::endl << std::endl << "Matrix::BuildFromArray   ***ERROR***    NumOfRows[" << numOfRows << "]  or  NumOfCols[" << numOfCols << "] is invalid." << std::endl << std::endl;
      return  NULL;
    }

    if (!data)
    {
      cerr << std::endl << std::endl << "Matrix::BuildFromArray   ***ERROR***    No source data (data == NULL)." << std::endl << std::endl;
      return  NULL;
    }

    Matrix<T>*  m = new Matrix<T> (numOfRows, numOfCols);

    for (kkuint32 row = 0; row < numOfRows; ++row)
    {
      U*  srcRow = data[row];
      T*  destRow = m->data[row];
      if (srcRow)
      {
        for (kkuint32 col = 0; col < numOfCols; ++col)
          destRow[col] = (T)(srcRow[col]);
      }
    }
    return  m;
  }  /* BuildFromArray */



  template<typename T>
  void   Matrix<T>::ReSize (kkuint32 _numOfRows,
                            kkuint32 _numOfCols
                           )
  {
    Destroy ();
    numOfRows = _numOfRows;
    numOfCols = _numOfCols;
    AllocateStorage ();
  }  /* ReSize */



  template<typename T>
  void  Matrix<T>::AllocateStorage ()
  {
    totNumCells = numOfRows * numOfCols;
    dataArea = new T[totNumCells];
    data = new T*[numOfRows];
    rows = new Row<T>[numOfRows];

    for (kkuint32 x = 0; x < totNumCells; ++x)
      dataArea[x] = 0.0;

    T*  dataAreaPtr = dataArea;
    for (kkuint32 x = 0; x < numOfRows; x++)
    {
      rows[x].Define (numOfCols, dataAreaPtr);
      data[x] = dataAreaPtr;
      dataAreaPtr += numOfCols;
    }
  }  /* AllocateStorage */



  template<typename T>
  void  Matrix<T>::Destroy ()
  {
    delete[]  rows;     rows = NULL;
    delete[]  data;     data = NULL;
    delete[]  dataArea; dataArea = NULL;
  }



  template<typename T>
  Row<T>&  Matrix<T>::operator[] (kkuint32  rowIDX) const
  {
    if (rowIDX >= numOfRows)
    {
      KKStr  msg (80);
      msg << "Matrix::operator[]   **** ERROR ****,  Row Index[" << rowIDX << "]  Invalid.";
      std::cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    return  (rows[rowIDX]);
  } /* Matrix::operator[] */



  template<typename T>
  T  Matrix<T>::DeterminantSlow () const
  {
    if (numOfCols != numOfRows)
    {
      KKStr  msg (80);
      msg << "Matrix::DeterminantSlow   *** ERROR ***   Dimensions are not Square[" << numOfRows << "," << numOfCols << "]  Invalid.";
      cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    if (numOfCols == 1)
    {
      return data[0][0];
    }

    kkuint32*  rowMap = new kkuint32[numOfRows];
    for (kkuint32 x = 0; x < numOfRows; x++)
      rowMap[x] = x;

    kkuint32*  colMap = new kkuint32[numOfCols];
    for (kkuint32 x = 0; x < numOfCols; x++)
      colMap[x] = x;

    T det = CalcDeterminent (rowMap, colMap, numOfCols);

    delete[]  colMap;
    return  det;
  }  /* DeterminantSlow */



  template<typename T>
  Matrix<T>&  Matrix<T>::operator= (const Matrix<T>&  right)
  {
    ReSize (right.numOfRows, right.numOfCols);
    memcpy (dataArea, right.dataArea, totNumCells * sizeof (T));
    return  *this;
  }



  template<typename T>
  template<typename U>
  Matrix<T>&  Matrix<T>::operator=  (const std::vector<U>&  right)
  {
    ReSize ((kkuint32)right.size (), 1);
    for (kkuint32 row = 0; row < numOfRows; row++)
      dataArea[row] = static_cast<T>(right[row]);

    return  *this;
  }  /* operator= */



  template<typename T>
  template<typename U>
  Matrix<T>&  Matrix<T>::operator*= (U  right)
  {
    for (kkuint32 x = 0; x < totNumCells; ++x)
      dataArea[x] *= static_cast<T>(right);
    return  *this;
  }



  template<typename T>
  template<typename U>
  Matrix<T>&  Matrix<T>::operator+= (U  right)
  {
    for (kkuint32 x = 0; x < totNumCells; ++x)
      dataArea[x] += static_cast<T>(right);
    return  *this;
  }



  template<typename T>
  Matrix<T>  Matrix<T>::operator+ (const Matrix<T>&  right)
  {
    if ((numOfRows != right.numOfRows) ||
      (numOfCols != right.numOfCols))
    {
      KKStr  msg (100);
      msg << "Matrix::operator+   **** ERROR ****,  Dimensions Don't Match [" << numOfRows << "," << numOfCols << "] + [" << right.numOfRows << "," << right.numOfCols << "].";
      cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    Matrix<T>  result (*this);

    T*  resultDataArea = result.dataArea;
    T*  rightDataArea = right.dataArea;

    for (kkuint32 x = 0; x < totNumCells; ++x)
      resultDataArea[x] = dataArea[x] + rightDataArea[x];

    return  result;
  }  /* Matrix::operator+ */



  template<typename T>
  Matrix<T>&  Matrix<T>::operator+= (const Matrix<T>&  right)
  {
    if ((numOfRows != right.numOfRows) ||
      (numOfCols != right.numOfCols))
    {
      KKStr  msg (100);
      msg << "Matrix::operator+=   **** ERROR ****,  Dimensions Don't Match [" << numOfRows << "," << numOfCols << "] + [" << right.numOfRows << "," << right.numOfCols << "].";
      cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    T*  rightDataArea = right.dataArea;

    for (kkuint32 x = 0; x < totNumCells; ++x)
      dataArea[x] += rightDataArea[x];

    return  *this;
  }  /* Matrix::operator+ */



  template<typename T>
  Matrix<T>  Matrix<T>::operator- (const Matrix<T>&  right)
  {
    if ((numOfRows != right.numOfRows) ||
      (numOfCols != right.numOfCols))
    {
      KKStr  msg (100);
      msg << "Matrix::operator-   **** ERROR ****,  Dimensions Don't Match [" << numOfRows << "," << numOfCols << "] + [" << right.numOfRows << "," << right.numOfCols << "].";
      cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    Matrix<T>  result (*this);

    T*  resultDataArea = result.dataArea;
    T*  rightDataArea = right.dataArea;

    for (kkuint32 x = 0; x < totNumCells; ++x)
      resultDataArea[x] = dataArea[x] - rightDataArea[x];

    return  result;
  } /* operator- */



  template<typename T>
  Matrix<T>  Matrix<T>::operator- (T right)
  {
    Matrix  result (*this);
    T*  resultDataArea = result.dataArea;
    for (kkuint32 x = 0; x < totNumCells; ++x)
      resultDataArea[x] = dataArea[x] - right;

    return  result;
  }



  template<typename T>
  Matrix<T>  operator- (T                left,
                        const Matrix<T>& right
                       )
  {
    kkuint32  numOfRows   = right.NumOfRows ();
    kkuint32  numOfCols   = right.NumOfCols ();
    kkuint32  totNumCells = right.totNumCells;

    Matrix<T>  result (numOfRows, numOfCols);
    T*  resultDataArea = result.dataArea;
    T*  rightDataArea = right.dataArea;

    for (kkuint32 x = 0; x < totNumCells; ++x)
      resultDataArea[x] = left - rightDataArea[x];

    return  result;
  }  /* operator- */



  template<typename T>
  Matrix<T>  Matrix<T>::operator* (const Matrix<T>&  right)
  {
    if (numOfCols != right.numOfRows)
    {
      KKStr  msg (100);
      msg << "Matrix::operator*   **** ERROR ****,  Dimension Mismatch  Left[" << numOfRows << "," << numOfCols << "]  Right[" << right.numOfRows << "," << right.numOfCols << "].";
      std::cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    Matrix<T> result (this->NumOfRows (), right.NumOfCols ());
    MultiplyMatrix (*this, right, result);
    return  result;
}  /* Matrix::operator */



  template<typename T>
  Matrix<T>  Matrix<T>::operator+ (T  right)
  {
    Matrix<T>  result (*this);
    T*  resultDataArea = result.dataArea;
    for (kkuint32 x = 0; x < totNumCells; ++x)
      resultDataArea[x] = dataArea[x] + right;
    return  result;
  }  /* operator+ */


  
  template<typename T>
  Matrix<T>  Matrix<T>::operator* (T  right)
  {
    Matrix  result (*this);
    T*  resultDataArea = result.dataArea;
    for (kkuint32 x = 0; x < totNumCells; ++x)
      resultDataArea[x] = dataArea[x] * right;
    return  result;
  }  /* operator* */



  /**
   *@brief Computes the Determinant using a recursive algorithm and co-factors matrixes.
   *@details  Very inefficient implementation,  would only use on very small matrices.   *
   */
  template<typename T>
  T  Matrix<T>::CalcDeterminent (kkuint32*  rowMap,
                                 kkuint32*  colMap,
                                 kkuint32   size
                                )
  {
    if (size == 2)
    {
      T* topCols = data[rowMap[0]];
      T* botCols = data[rowMap[1]];
      return  topCols[colMap[0]] * botCols[colMap[1]] - topCols[colMap[1]] * botCols[colMap[0]];
    }

    if (size == 1)
      return (T)0.0;

    T* coFactors = data[rowMap[0]];
    T  det = (T)0.0;
    kkuint32 newSize = size - 1;
    kkint32 sign = 1;

    kkuint32*  newRowMap = new kkuint32[newSize];

    for (kkuint32 row = 1; row < size; row++)
    {
      newRowMap[row - 1] = rowMap[row];
    }

    for (kkuint32 cfCol = 0; cfCol < size; ++cfCol)
    {
      kkuint32*  newColMap = new kkuint32[newSize];

      kkuint32  newCol = 0;
      for (kkuint32 oldCol = 0; oldCol < size; ++oldCol)
      {
        if (oldCol != cfCol)
        {
          newColMap[newCol] = colMap[oldCol];
          newCol++;
        }
      }

      det = det + sign * coFactors[colMap[cfCol]] * CalcDeterminent (newRowMap, newColMap, newSize);
      if (sign > 0)
        sign = -1;
      else
        sign = 1;

      delete[]  newColMap;
      newColMap = NULL;
    }

    delete[]  newRowMap;
    newRowMap = NULL;

    return  det;
  }  /* CalcDeterminent */



  template<typename T>
  Matrix<T>*  Matrix<T>::CalcCoFactorMatrix ()
  {
    if (numOfCols != numOfRows)
    {
      KKStr errMsg (128);
      errMsg << "Matrix::CalcCoFactors   **** ERROR ****,  Matrix not a Square["
        << numOfRows << "," << numOfCols << "].";

      std::cerr << std::endl << errMsg << std::endl << std::endl;
      throw KKException (errMsg);
    }

    auto  result = new Matrix<T> (numOfRows, numOfCols);

    kkuint32  newSize = numOfCols - 1;

    kkuint32*  colMap = new kkuint32[newSize];
    kkuint32*  rowMap = new kkuint32[newSize];

    for (kkuint32 row = 0;  row < numOfRows;  ++row)
    {
      // Create a map of all rows except row we are calculating 
      // CoFactors for.

      kkuint32  newRow = 0;
      for (kkuint32 x = 0; x < numOfRows; ++x)
      {
        if (x != row)
        {
          rowMap[newRow] = x;
          ++newRow;
        }
      }

      for (kkuint32 col = 0; col < numOfCols; ++col)
      {
        // Create a map of all cells except row we are calculating 
        // CoFactor for.

        kkuint32  newCol = 0;
        for (kkuint32 x = 0; x < numOfCols; x++)
        {
          if (x != col)
          {
            colMap[newCol] = x;
            newCol++;
          }
        }

        kkint32 sign = (((row + col) % 2) == 0) ? 1 : -1;

        Matrix<T>  temp (newSize, newSize);
        for (kkuint32 r = 0; r < newSize; ++r)
        {
          kkuint32  tempR = rowMap[r];
          for (kkuint32 c = 0; c < newSize; ++c)
            temp[r][c] = data[tempR][colMap[c]];
        }

        result->data[row][col] = temp.Determinant () * sign;
      }
    }

    return  result;
  } /* CalcCoFactor  */


  template<typename T>
  bool  Matrix<T>::Symmetric ()  const
  {
    if ((data == NULL) || (numOfRows != numOfCols))
      return false;

    for (kkuint32 row = 0; row < numOfRows; ++row)
    {
      for (kkuint32 col = row + 1; col < numOfCols; ++col)
      {
        if (data[row][col] != data[col][row])
          return false;
      }
    }
    return  true;
  }  /* Symmetric */



  template<typename T>
  Matrix<T>  Matrix<T>::Transpose ()
  {

    Matrix<T>  result (numOfCols, numOfRows);

    T**  resultData = result.data;

    for (kkuint32 row = 0; row < numOfRows; ++row)
    {
      for (kkuint32 col = 0; col < numOfCols; ++col)
      {
        resultData[col][row] = data[row][col];
      }
    }

    return  result;
  }  /* Transpose */



  template<typename T>
  Matrix<T>  Matrix<T>::Inverse ()
  {
    if (numOfCols != numOfRows)
    {
      KKStr  msg (80);
      msg << "Matrix::Inverse   *** ERROR ***   Dimensions are not Square[" << numOfRows << "," << numOfCols << "]  Invalid.";
      std::cerr << std::endl << msg << std::endl << std::endl;
      throw  KKException (msg);
    }

    T  det = Determinant ();
    if (det == 0)
    {
      std::cerr << std::endl << "Matrix::Inverse   *** ERROR ***   Determinant of Matrix is Zero." << std::endl << std::endl;
    }

    auto  coFactors = CalcCoFactorMatrix ();

    auto  result = coFactors->Transpose ();

    delete  coFactors;

    if (det == 0.0)
      result *= 0.0;
    else
      result *= (1.0 / det);

    return  result;
  } /* Inverse */



 /**
  *@brief  Will derive the Eigen vectors and values of the matrix.
  *@details Will make use of routines from Numerical Recipes for c++, ex:  Tred2 and Tqli.
  */
  template<typename T>
  void  Matrix<T>::EigenVectors (Matrix<T>*&       eigenVectors,
                                 std::vector<T>*&  eigenValues
                                )  const
  {
    eigenVectors = NULL;
    eigenValues = NULL;
    if ((data == NULL) || (numOfRows < 1))
    {
      cerr << std::endl << "Matrix::EigenVectors   ***ERROR***   'data' not defined in Matrix." << std::endl << std::endl;
      return;
    }

    if (numOfRows != numOfCols)
    {
      cerr << std::endl << "Matrix::EigenVectors   ***ERROR***   Not a square matrix  NumOfRows[" << numOfRows << "]  NumOfCols[" << numOfCols << "]." << std::endl << std::endl;
      return;
    }

    eigenVectors = new Matrix (*this);

    if (Symmetric ())
    {
      T*  d = new T[numOfRows];
      T*  e = new T[numOfRows];
      for (kkuint32 x = 0; x < numOfRows; ++x)
      {
        d[x] = 0.0;
        e[x] = 0.0;
      }
      Tred2 (eigenVectors->data, numOfRows, d, e);
      kkint32  successful = Tqli (d, e, numOfRows, eigenVectors->data);
      if (successful != 1)
      {
        delete[] d;  d = NULL;
        delete[] e;  e = NULL;
        return;
      }

      eigenValues = new std::vector<T> ();
      for (kkuint32 x = 0; x < numOfRows; ++x)
        eigenValues->push_back (d[x]);

      delete[]  d;  d = NULL;
      delete[]  e;  e = NULL;
    }
  }  /* GetEigenVectors */



  template<typename T>
  void   Matrix<T>::FindMaxValue (T&         maxVal,
                                  kkuint32&  rowIdx,
                                  kkuint32&  colIdx
                                 )
  {
    rowIdx = 0;
    colIdx = 0;
    maxVal = DBL_MIN;

    if (!data)
      return;

    maxVal = data[0][0];
    rowIdx = colIdx = 0;

    for (kkuint32 row = 0; row < numOfRows; ++row)
    {
      T*  dataRow = data[row];
      for (kkuint32 col = 0; col < numOfCols; ++col)
      {
        if (dataRow[col] > maxVal)
        {
          maxVal = dataRow[col];
          colIdx = col;
          rowIdx = row;
        }
      }
    }

    return;
  }  /* FindMaxValue */



  template<typename T>
  std::ostream&  operator<< (std::ostream&     os,
                             const Matrix<T>&  matrix
                            )
  {
    os << "[" << matrix.NumOfRows () << "," << matrix.NumOfCols () << "]" << std::endl;

    os << "[";

    for (kkuint32 row = 0; row < matrix.NumOfRows (); row++)
    {
      if (row  > 0)
        os << " ";

      os << "[";

      for (kkuint32 col = 0; col < matrix.NumOfCols (); col++)
      {
        if (col > 0)
          os << ", ";

        os.width (8);
        os.precision (6);
        os << matrix[row][col];
      }

      os << "]" << std::endl;
    }

    os << "]" << std::endl;

    return os;
  }  /* operator<< */



  template<typename T>
  std::vector<T>  Matrix<T>::GetCol (kkuint32  col)  const
  {
    std::vector<T>  colResult (numOfRows, 0.0);
    for (kkuint32 r = 0; r < numOfRows; ++r)
      colResult[r] = data[r][col];

    return  colResult;
  }  /* GetCol */


#if  !defined(DBL_EPSILON)
#define DBL_EPSILON    2.2204460492503131e-016
#endif



  /**
   * @param mat
   * @param offset
   * @return A[offset][offset]
   */
  template<typename T>
  T  Matrix<T>::DeterminantSwap (T** mat,  kkuint32 offset) const
  {
    if (fabs (mat[offset][offset]) < DBL_EPSILON)
    {
      for (kkuint32 i = offset + 1; i < numOfRows; i++)
      {
        if (fabs (mat[i][offset]) >= DBL_EPSILON)
        {
          /* Swap line `i' and line `offset'*/
          T	*tmp = mat[offset];
          mat[offset] = mat[i];
          mat[i] = tmp;
          break;
        }
      }
    }
    return (mat[offset][offset]);
  }  /* DeterminantSwap */



  /** @return the determinant of mat. */
  template<typename T>
  T  Matrix<T>::Determinant ()  const
  {
    if (numOfCols != numOfRows)
      return (T)-999999.99;

    T** mat = new T*[numOfRows];
    for (kkuint32 r = 0; r < numOfRows; ++r)
    {
      mat[r] = new T[numOfCols];
      for (kkuint32 c = 0; c < numOfCols; ++c)
      {
        mat[r][c] = data[r][c];
      }
    }

    T det = static_cast<T>(1.0);

    for (kkuint32 i = 0; i < numOfRows; ++i)
    {
      const T  Aii = DeterminantSwap (mat, i);

      if (fabs (Aii) < DBL_EPSILON)
      {
        det = 0.0;
        break;
      }

      det *= Aii;

      //Do elimination
      for (kkuint32 j = i + 1; j < numOfRows; j++)
      {
        const T pivot = mat[j][i] / Aii;
        for (kkuint32 k = i; k < numOfRows; k++)
          mat[j][k] -= pivot * mat[i][k];
      }
    }

    for (kkuint32 r = 0; r < numOfRows; r++)
    {
      delete  mat[r];
      mat[r] = NULL;
    }
    delete[]  mat;
    mat = NULL;
    return det;
  }  /* Determinant */



  /**
   *@brief  Returns a Covariance matrix.
   *@details  Each column represents a variable and each row represents an instance of each variable.
   *@return  Returns a symmetric matrix that will be (numOfRows x numOfRows) where each element will represent the covariance
   *         between their respective variables.
   */
  template<typename T>
  Matrix<T>*  Matrix<T>::Covariance ()  const
  {
    if ((data == NULL) || (numOfRows < 1))
    {
      cerr << std::endl << "Matrix::Covariance   ***ERROR***   'data' not defined in Matrix." << std::endl << std::endl;
      return NULL;
    }

    // Used web site below to help with Covariance calculations
    //  http://www.itl.nist.gov/div898/handbook/pmc/section5/pmc541.htm

    T*   totals = new T[numOfCols];
    T*   means = new T[numOfCols];
    T**  centeredVals = new T*[numOfCols];
    for (kkuint32 col = 0; col < numOfCols; ++col)
    {
      totals[col] = static_cast<T>(0.0);
      centeredVals[col] = new T[numOfRows];
    }

    for (kkuint32 row = 0; row < numOfRows; ++row)
    {
      T*  rowData = data[row];
      for (kkuint32 col = 0; col < numOfCols; ++col)
        totals[col] += rowData[col];
    }

    for (kkuint32 col = 0; col < numOfCols; ++col)
      means[col] = totals[col] / numOfRows;

    for (kkuint32 row = 0; row < numOfRows; ++row)
    {
      T*  rowData = data[row];
      for (kkuint32 col = 0; col < numOfCols; ++col)
        centeredVals[col][row] = rowData[col] - means[col];
    }

    auto  covariances = new Matrix<T> (numOfCols, numOfCols);

    for (kkuint32 varIdxX = 0; varIdxX < numOfCols; ++varIdxX)
    {
      T*  varXs = centeredVals[varIdxX];
      for (kkuint32 varIdxY = varIdxX; varIdxY < numOfCols; ++varIdxY)
      {
        // Calculate the covariance between chanIdx0 and chanIdx1

        T*  varYs = centeredVals[varIdxY];
        T total = 0.0f;
        for (kkuint32 row = 0; row < numOfRows; ++row)
          total += varXs[row] * varYs[row];
        (*covariances)[varIdxX][varIdxY] = total / (numOfRows - 1);
        (*covariances)[varIdxY][varIdxX] = (*covariances)[varIdxX][varIdxY];
      }
    }

    for (kkuint32 col = 0; col < numOfCols; col++)
    {
      delete[]  centeredVals[col];
      centeredVals[col] = NULL;
    }
    delete[]  centeredVals;   centeredVals = NULL;
    delete[]  means;          means = NULL;
    delete[]  totals;         totals = NULL;

    return  covariances;
  }  /* Covariance */



  /**
   *@brief  Householder reduction of a real symmetric matrix a[0..n-1][0..n-1].
   *@details  From Numerical Recipes in c++, page 479.  Tred2 is designed to work with Tqli.  Its purpose
   *          is to reduce the matrix 'a' from a symmetric matrix to a orthogonal matrix.
   *@param[in,out]  a  On output a is replaced by the orthogonal matrix Q effecting the transform.
   *@param[in]      n  Size of matrix is (n x n).
   *@param[out]     d  Returns the diagonal elements of the tridiagonal matrix,
   *@param[out]     e  The off-diagonal elements with e[0] = 0.
   */
  template<typename T>
  void  Matrix<T>::Tred2 (T**       a,
                          kkuint32  n,
                          T*        d,
                          T*        e
                         )  const
  {
    kkuint32  i, j, k, l;

    T  scale, hh, h, g, f;

    for (i = n - 1; i > 0; --i)
    {
      l = i - 1;
      h = scale = 0.0;

      if (l > 0)
      {
        for (k = 0; k < l + 1; k++)
          scale += fabs (a[i][k]);

        if (scale == (T)0.0)
        {
          e[i] = a[i][l];
        }

        else
        {
          for (k = 0; k < l + 1; k++)
          {
            a[i][k] /= scale;       // Use scaled a's for transformation
            h += a[i][k] * a[i][k];  // h = sigma
          }

          f = a[i][l];
          g = (f > 0 ? -sqrt (h) : sqrt (h));
          e[i] = scale * g;
          h -= f * g;
          a[i][l] = f - g;
          f = 0.0;

          for (j = 0; j < l + 1; j++)
          {
            a[j][i] = a[i][j] / h;
            g = 0.0;
            for (k = 0; k < j + 1; k++)
              g += a[j][k] * a[i][k];

            for (k = j + 1; k < l + 1; k++)
              g += a[k][j] * a[i][k];

            e[j] = g / h;

            f += e[j] * a[i][j];
          } /* for  (j = 0; */

          hh = f / (h + h);

          for (j = 0; j < l + 1; j++)
          {
            f = a[i][j];
            e[j] = g = e[j] - hh * f;
            for (k = 0; k < j + 1; k++)
              a[j][k] -= (f * e[k] + g * a[i][k]);
          }
        }
      }  /* if  (l > 0) */
      else
      {
        e[i] = a[i][l];
      }

      d[i] = h;
    }

    // We can now calculate Eigen Vectors

    d[0] = (T)0.0;
    e[0] = (T)0.0;

    for (i = 0; i < n; i++)
    {
      l = i;

      if (d[i] != 0.0)
      {
        for (j = 0; j < l; j++)
        {
          g = 0.0;
          for (k = 0; k < l; k++)
            g += a[i][k] * a[k][j];

          for (k = 0; k < l; k++)
            a[k][j] -= g * a[k][i];
        }
      }

      d[i] = a[i][i];

      // Reset row and column of 'a' to identity matrix;
      a[i][i] = (T)1.0;
      for (j = 0; j < l; j++)
        a[j][i] = a[i][j] = 0.0;
    }  /* for (i) */

  }  /* Tred2 */



  template<class T>
  inline const T SIGN (const T& a,
                       const T& b
                      )
  {
    if (b >= 0)
    {
      if (a >= 0)
        return  a;
      else
        return -a;
    }

    else
    {
      if (a >-0)
        return -a;
      else
        return  a;
    }
  }  /* SIGN */


#define  SQR(X)  ((X) * (X))

  // Computes sqrt (a^2 + b^2)  without destructive underflow or overflow
  template<typename T>
  T  Matrix<T>::Pythag (const T a,  const T b)  const
  {
    T  absa, absb;
    absa = fabs (a);
    absb = fabs (b);

    if (absa > absb)
    {
      return  absa * sqrt ((T)1.0 + SQR (absb / absa));
    }
    else
    {
      if (absb == 0.0)
        return 0.0;
      else
        return absb * sqrt (1.0 + SQR (absa / absb));
    }
  }  /* pythag */



  /**
   *@brief  Determines the eigenvalues and eigen-vectors of a real symmetric, tridiagonal matrix previously reduced by Tred2.
   *@details TQLI = "Tridiagonal QL Implicit".
   *@param[in,out]   d contains the diagonal elements of the tridiagonal matrix.  On output will contain the eigenvalues.
   *@param[in]       e On input contains the sub-diagonal of the tridiagonal matrix, with e[0] arbitrary.  On output e is destroyed.
   *@param[in]       n Size of matrix side.
   *@param[in,out]   z If eigen-vectors of Tridiagonal matrix are required input as a identity matrix[0..n-1][o..n-1].  If
   *                   Eigen-vectors of input matrix to "Tred2" are required then z should equal the output matrix from
   *                   Tred2 "a".
   */
  template<typename T>
  kkint32  Matrix<T>::Tqli (T*        d,
                            T*        e,
                            kkuint32  n,
                            T**       z
                           ) const
  {
    kkuint32  m, l, iter, i, k;
    T  s, r, p, g, f, dd, c, b;

    for (i = 1; i < n; ++i)
      e[i - 1] = e[i];

    e[n - 1] = 0.0;

    for (l = 0; l < n; ++l)
    {
      iter = 0;

      do
      {
        for (m = l; m < n - 1; ++m)
        {
          // Looking for a single small sub-diagonal element
          // to split the matrix
          dd = fabs (d[m]) + fabs (d[m + 1]);
          if (fabs (e[m]) + dd == dd)
            break;
        }

        if (m != l)
        {
          if (iter == 100)
          {
            cerr << std::endl << std::endl
              << "Matrix::tqli    **** ERROR ****            To many iterations in tqli" << std::endl
              << std::endl;
            return  0;
          }
          iter++;

          g = (d[l + 1] - d[l]) / (2.0 * e[l]);
          r = Pythag (g, 1.0);
          g = d[m] - d[l] + e[l] / (g + SIGN (r, g));
          s = c = 1.0;
          p = 0.0;

          for (i = m - 1; i >= l; i--)
          {
            f = s * e[i];
            b = c * e[i];
            e[i + 1] = (r = Pythag (f, g));
            if (r == 0.0)
            {
              d[i + 1] -= p;
              e[m] = 0.0;
              break;
            }

            s = f / r;
            c = g / r;
            g = d[i + 1] - p;
            r = (d[i] - g) * s + 2.0 * c * b;
            d[i + 1] = g + (p = s * r);
            g = c * r - b;

            // Next loop can be omitted if eigen-vectors not wanted

            for (k = 0; k < n; k++)
            {
              f = z[k][i + 1];
              z[k][i + 1] = s * z[k][i] + c * f;
              z[k][i] = c * z[k][i] - s * f;
            }  /* for (k) */

          }  /* for (i) */

          if ((r == 0.0) && (i >= l))
            continue;

          d[l] -= p;
          e[l] = g;
          e[m] = 0.0;
        }
      } while (m != l);

    }  /* for (l) */

    return 1;
  }  /* Tqli*/

}  /* KKB */
#endif
