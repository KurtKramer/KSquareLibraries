/* Matrix.h -- A simple two dimensional floating point matrix.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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

#include  "BasicTypes.h"

namespace  KKU
{
  class  Row; 
  typedef Row*  RowPtr;

  /**
   *@class  Matrix
   *@brief  Supports two dimensional matrices.
   *@details  Developed for Machine Learning Project for support of Import Vector Machine.
   *  Handles two dimensional matrixes.  Functions supported are matrix addition, subtraction
   *  multiplication, transpose, determinant, and inversion.  Where appropriate arithmetic
   *  arithmetic operators +, -, * were overloaded.  Addition, Subtraction and Multiplication
   *  can be done against either another matrix or scaler.  Also Transpose and Determinant
   *  operations are supported.
   */
  class  Matrix
  {
  public:
    typedef  Matrix*  MatrixPtr;

    Matrix ();

    Matrix (int32  _numOfRows,
            int32  _numOfCols
           );

    Matrix (const Matrix&  _matrix);

    Matrix (const VectorDouble&  _v);

    ~Matrix ();

    template<typename T>
    static  MatrixPtr  BuildFromArray (int32 numOfRows,
                                       int32 numOfCols,
                                       T**   data
                                      );

    Matrix&  operator=  (const Matrix&  right);

    Matrix&  operator=  (const VectorDouble&  right);

    Matrix&  operator*= (double  right);

    Matrix&  operator+= (double  right);
  
    Matrix&  operator+= (const Matrix&  right);

    Matrix   operator+  (const Matrix&  right);

    Matrix   operator+  (double  right);

    Matrix   operator-  (const Matrix&  right);  

    Matrix   operator-  (double right);  

    Matrix   operator*  (const Matrix&  right);

    Matrix   operator*  (double right);

    Row&     operator[] (int32  rowIDX) const;

    friend  Matrix  operator- (double left, const Matrix& right);


    MatrixPtr       CalcCoFactorMatrix ();

    /**
     *@brief  Returns a Covariance matrix.
     *@details  Each column represents a variable and each row represents an instance of each variable.
     *@return  Returns a symmetric matrix that will be (numOfRows x numOfRows) where each element will represent the covariance 
     *         between their respective variables.
     */
    MatrixPtr       Covariance ()  const;

    double** const  Data ()  const  {return  data;}

    double          Determinant ();

    double          DeterminantSlow ();  /**<  @brief Recursive Implementation. */

    void            EigenVectors (MatrixPtr&      eigenVectors,
                                  VectorDouble*&  eigenValues
                                 )  const;

    /** @brief  Locates the maximum value in a matrix along with the row and column that is is located. */
    void            FindMaxValue (double&  maxVal, 
                                  int32&     row, 
                                  int32&     col
                                 );

    VectorDouble    GetCol (int32 col)  const;

    Matrix          Inverse ();

    int32           NumOfCols () const  {return numOfCols;}

    int32           NumOfRows () const  {return numOfRows;}

    void            ReSize (int32 _numOfRows,
                            int32 _numOfCols
                           );

    bool            Symmetric ()  const;  /**< Returns true is the matrix is Symmetric */

    Matrix          Transpose ();

    friend  std::ostream&  operator<< (      std::ostream&  os, 
                                       const Matrix&        matrix
                                      );

  private:
    void  Destroy ();

    void  AllocateStorage ();

    double DeterminantSwap (double**        mat, 
                            unsigned short  offset
                           );

    double  CalcDeterminent (int32*  rowMap,
                             int32*  colMap,
                             int32 size
                            );

    double  Pythag (const double a,
                    const double b
                   )  const;

    int32  Tqli (double*  d, 
                 double*  e,
                 int32    n,
                 double** z
                )  const;

    void  Tred2 (double** a, 
                 int32    n, 
                 double*  d, 
                 double*  e
                )  const;

    double**  data;       /**< A two dimensional array that will index into 'dataArea'.       */
    double*   dataArea;   /**< one dimensional array that will contain all the matrixes data. */
    int32     numOfCols;
    int32     numOfRows;
    RowPtr    rows;
    int32     totNumCells; /**<  Total number of cells allocated  = (numOfRows x nmumOfCols). */
  };  /* matrix */


  typedef  Matrix::MatrixPtr  MatrixPtr;

  class  Row
  {
  public:
     Row  ();
 
     Row (int32    _numOfCols,
          double*  _cells
         );

     Row (const Row&  _row);

     ~Row ();


     double*  Cols ()  {return cells;}

     double&  operator[] (int32  idx);

     void  Define (int32    _numOfCols,
                   double*  _cells
                  );

  private:
    double*  cells;
    int32    numOfCols;
  };  /* Row */

  Matrix  operator- (double left, const Matrix& right);
}  /* KKU */




#endif

