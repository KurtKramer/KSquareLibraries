/* Matrix.cpp -- A simple two dimensional floating point matrix.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include "FirstIncludes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "Matrix.h"
#include "KKException.h"
#include "OSservices.h"
#include "Str.h"
using namespace KKU;


Row::Row  ():
  cells     (NULL),
  numOfCols (-1)
{
}



Row::Row  (int32    _numOfCols,
           double*  _cells
          ):
  cells     (_cells),
  numOfCols (_numOfCols)
{
  if  (_numOfCols < 0)
  {
	KKStr  msg;
	msg << "Row::Row  **** ERROR **** Invalid Dimension[" << _numOfCols << "].";
    cerr << std::endl  << msg << std::endl << std::endl;
    throw KKException (msg);
  }
}



Row::Row (const Row&  _row):
  cells     (_row.cells),
  numOfCols (_row.numOfCols)
{
}



Row::~Row ()
{
  cells = NULL;
}



void  Row::Define (int32    _numOfCols,
                   double*  _cells
                  )
{
  numOfCols = _numOfCols;
  cells     = _cells;
}



double&  Row::operator[] (int32  idx)
{
  if  ((idx < 0)  ||  (idx >= numOfCols))
  {
    cerr << std::endl
         << "Row::operator[]  **** ERROR ****,  Index[" 
         << idx << "]  out of range of [0-" << numOfCols << "]."
         << std::endl;
    osWaitForEnter ();
    exit (1);
  }

  return  cells[idx];
}  /*  Row::operator[] */




Matrix::Matrix ():

  data        (NULL),
  dataArea    (NULL),
  numOfCols   (0),
  numOfRows   (0),
  rows        (NULL),
  totNumCells (0)
{
}




Matrix::Matrix (int32  _numOfRows,
                int32  _numOfCols
               ):

  data        (NULL),
  dataArea    (NULL),
  numOfCols   (_numOfCols),
  numOfRows   (_numOfRows),
  rows        (NULL),
  totNumCells (0)

{
  if  (_numOfRows < 0)
  {
    KKStr  msg(80);
    msg << "Matrix::Matrix   **** ERROR ****,  Row Dimension[" << _numOfRows << "]  Invalid.";
    cerr << std::endl << msg <<std::endl << std::endl;
    throw KKException (msg);
  }

  if  (_numOfCols < 0)
  {
	KKStr  msg(80);
	msg << "Matrix::Matrix   **** ERROR ****,  Col Dimension[" << _numOfCols << "]  Invalid.";
	cerr << std::endl << msg <<std::endl << std::endl;
	throw KKException (msg);
  }

  AllocateStorage ();
}  /*  Matrix::Matrix  */




Matrix::Matrix (const Matrix&  _matrix):
  data        (NULL),
  dataArea    (NULL),
  numOfCols   (_matrix.numOfCols),
  numOfRows   (_matrix.numOfRows),
  rows        (NULL),
  totNumCells (0)

{
  AllocateStorage ();
  memcpy (dataArea, _matrix.dataArea, totNumCells * sizeof (double));
} /* Matrix::Matrix  */



Matrix::Matrix (const VectorDouble&  _v):
  data      (NULL),
  dataArea  (NULL),
  numOfCols (1),
  numOfRows (int32 (_v.size ())),
  rows      (NULL)
{
  AllocateStorage ();
  int32  row;
  for  (row = 0;  row < numOfRows;  ++row)
    dataArea[row] = _v[row];
} /* Matrix::Matrix  */




Matrix::~Matrix ()
{
  Destroy ();
}



/**
 *@brief  Will create a new matrix using the 2dArray "data" for source.
 *@details  If any row in "data" is equal to NULL then the corresponding row 
 * in the returned matrix will be initialized to zeros.
 *@param[in]  numOfRows  Number of rows in resultant matrix.
 *@param[in]  numOfCols  Number of cols if resultant matrix.
 *@param[in]  data       A two dimensional array.
 *@return  A matrix that is initialized with the contents of "data".
 */
template<typename T>
MatrixPtr  Matrix::BuildFromArray (int32 numOfRows,
                                   int32 numOfCols,
                                   T**   data
                                  )
{
  if  ((numOfRows < 1)  ||  (numOfCols < 1))
  {
    cerr << std::endl << std::endl << "Matrix::BuildFromArray   ***ERROR***    NumOfRows[" << numOfRows << "]  or  NumOfCols[" << numOfCols << "] is invalid." << std::endl << std::endl;
    return  NULL;
  }

  if  (!data)
  {
    cerr << std::endl << std::endl << "Matrix::BuildFromArray   ***ERROR***    No source data (data == NULL)." << std::endl << std::endl;
    return  NULL;
  }

  MatrixPtr  m = new Matrix (numOfRows, numOfCols);

  for  (int32 row = 0;  row < numOfRows;  ++row)
  {
    T*       srcRow  = data[row];
    double*  destRow = m->data[row];
    if  (srcRow)
    {
      for  (int32 col = 0;  col < numOfCols;  ++col)
        destRow[col] = (double)(srcRow[col]);
    }
  }
  return  m; 
}  /* BuildFromArray */



void   Matrix::ReSize (int32 _numOfRows,
                       int32 _numOfCols
                      )
{
  Destroy ();
  numOfRows = _numOfRows;
  numOfCols = _numOfCols;
  AllocateStorage ();
}  /* ReSize */




void  Matrix::AllocateStorage ()
{
  totNumCells = numOfRows * numOfCols;
  dataArea = new double[totNumCells];
  data = new double*[numOfRows];
  rows = new Row [numOfRows];
  
  int32  x;

  for  (x = 0;  x < totNumCells;  ++x)
    dataArea[x] = 0.0;


  double*  dataAreaPtr = dataArea;
  for  (x = 0;  x < numOfRows;  x++)
  {
    rows[x].Define (numOfCols, dataAreaPtr);
    data[x] = dataAreaPtr;
    dataAreaPtr += numOfCols;
  }
}  /* AllocateStorage */



void  Matrix::Destroy ()
{
  delete[]  rows;     rows     = NULL;
  delete[]  data;     data     = NULL;
  delete[]  dataArea; dataArea = NULL;
}



Row&  Matrix::operator[] (int32  rowIDX) const
{
  if  ((rowIDX < 0)  ||  (rowIDX >= numOfRows))
  {
    KKStr  msg (80);
    msg << "Matrix::operator[]   **** ERROR ****,  Row Index[" << rowIDX << "]  Invalid.";
    cerr << std::endl << msg  << std::endl  << std::endl;
    throw  KKException (msg);
  }

  return  (rows[rowIDX]);
} /* Matrix::operator[] */




double  Matrix::DeterminantSlow ()
{
  if  (numOfCols != numOfRows)
  {
    KKStr  msg (80);
    msg << "Matrix::Determinant   *** ERROR ***   Dimensions are not Square[" << numOfRows << "," << numOfCols << "]  Invalid.";
    cerr << std::endl << msg  << std::endl  << std::endl;
    throw  KKException (msg);
  }

  if  (numOfCols == 1)
  {
    // return  rows[0]->cells[0];
    return data[0][0];
  }

  int32  x;

  int32*  rowMap = new int32[numOfRows];
  for  (x = 0; x < numOfRows; x++)
    rowMap[x] = x;

  int32*  colMap = new int32[numOfCols];
  for  (x = 0; x < numOfCols; x++)
    colMap[x] = x;

  double det = CalcDeterminent (rowMap, colMap, numOfCols);

  delete  colMap;
  return  det;
}  /* Determinent */


/*
Matrix&  Matrix::Invert ()
{


}

*/


Matrix&  Matrix::operator= (const Matrix&  right)
{
  ReSize (right.numOfRows, right.numOfCols);
  memcpy (dataArea, right.dataArea, totNumCells * sizeof (double));
  return  *this;
}


Matrix&  Matrix::operator=  (const VectorDouble&  right)
{
  ReSize (right.size (), 1);
  for  (int32 row = 0; row < numOfRows; row++)
    dataArea[row] = right[row];

  return  *this;
}  /* operator= */




Matrix&  Matrix::operator*= (double  right)
{
  int32  x = 0;
  for  (x = 0;  x < totNumCells;  ++x)
    dataArea[x] *= right;
  return  *this;
}



Matrix&  Matrix::operator+= (double  right)
{
  int32  x = 0;
  for  (x = 0;  x < totNumCells;  ++x)
    dataArea[x] += right;
  return  *this;
}



Matrix  Matrix::operator+ (const Matrix&  right)
{
  if  ((numOfRows != right.numOfRows)  ||
       (numOfCols != right.numOfCols))
  {
    KKStr  msg (100);
    msg << "Matrix::operator+   **** ERROR ****,  Dimensions Don't Match [" << numOfRows << "," << numOfCols << "] + [" << right.numOfRows << "," << right.numOfCols << "].";
    cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

  Matrix  result (*this);

  double*  resultDataArea = result.dataArea;
  double*  rightDataArea  = right.dataArea;

  for  (int32 x = 0;  x < totNumCells;  ++x)
    resultDataArea[x] = dataArea[x] + rightDataArea[x];

  return  result;
}  /* Matrix::operator+ */






Matrix&  Matrix::operator+= (const Matrix&  right)
{
  if  ((numOfRows != right.numOfRows)  ||
       (numOfCols != right.numOfCols))
  {
    KKStr  msg (100);
    msg << "Matrix::operator+=   **** ERROR ****,  Dimensions Don't Match [" << numOfRows << "," << numOfCols << "] + [" << right.numOfRows << "," << right.numOfCols << "].";
    cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

  double*  rightDataArea  = right.dataArea;

  for  (int32 x = 0;  x < totNumCells;  ++x)
    dataArea[x] += rightDataArea[x];

  return  *this;
}  /* Matrix::operator+ */





Matrix  Matrix::operator- (const Matrix&  right)
{
  if  ((numOfRows != right.numOfRows)  ||
       (numOfCols != right.numOfCols))
  {
    KKStr  msg (100);
    msg << "Matrix::operator-   **** ERROR ****,  Dimensions Don't Match [" << numOfRows << "," << numOfCols << "] + [" << right.numOfRows << "," << right.numOfCols << "].";
    cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

  Matrix  result (*this);

  double*  resultDataArea = result.dataArea;
  double*  rightDataArea  = right.dataArea;

  for  (int32 x = 0;  x < totNumCells;  ++x)
    resultDataArea[x] = dataArea[x] - rightDataArea[x];

  return  result;
} /* operator- */



Matrix   Matrix::operator- (double right)
{
  Matrix  result (*this);
  double*  resultDataArea = result.dataArea;
  for  (int32 x = 0;  x < totNumCells;  ++x)
    resultDataArea[x] = dataArea[x] - right;

  return  result;
}




Matrix  KKU::operator- (double        left, 
                        const Matrix& right
                       )
{
  int32  numOfRows = right.NumOfRows ();
  int32  numOfCols = right.NumOfCols ();
  int32  totNumCells = right.totNumCells;

  Matrix  result (numOfRows, numOfCols);
  double*  resultDataArea = result.dataArea;
  double*  rightDataArea  = right.dataArea;

  for  (int32 x = 0;   x < totNumCells;  ++x)
    resultDataArea[x] = left - rightDataArea[x];

  return  result;
}  /* operator- */





Matrix  Matrix::operator* (const Matrix&  right)
{
  if  (numOfCols != right.numOfRows)
  {
    KKStr  msg (100);
    msg << "Matrix::operator*   **** ERROR ****,  Dimension Mismatch  Left["  << numOfRows << "," << numOfCols << "]  Right[" << right.numOfRows << "," << right.numOfCols << "].";
    cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

  int32  col;
  int32  row;

  int32  rRows = numOfRows;
  int32  rCols = right.numOfCols;

  Matrix  result (rRows, rCols);

  double**  resultData = result.data;
  double**  rightData  = right.data;

  int32  innerDim = numOfCols;

  for  (row = 0;   row < rRows;  row++)
  {
    for  (col = 0;  col < rCols;  col++)
    {
       double  val = 0;
       int32   x   = 0;
       for  (x = 0; x < innerDim; x++)
         val = val + data[row][x] * rightData[x][col];
       resultData[row][col] = val;
    }
  }

  return  result;
}  /* Matrix::operator */




Matrix  Matrix::operator+ (double  right)
{
  Matrix  result (*this);
  double*  resultDataArea = result.dataArea;
  for  (int32 x = 0;  x < totNumCells;  ++x)
    resultDataArea[x] = dataArea[x] + right;
  return  result;
}  /* operator+ */



Matrix  Matrix::operator* (double  right)
{
  Matrix  result (*this);
  double*  resultDataArea = result.dataArea;
  for  (int32 x = 0;  x < totNumCells;  ++x)
    resultDataArea[x] = dataArea[x] * right;
  return  result;
}  /* operator* */



/**
 *@brief Computes the Determinant using a recursive algorithm and co-factors matrixes.
 *@details  Very inefficient implementation,  would only use on very small matrices.   *
 */
double  Matrix::CalcDeterminent (int32*  rowMap,
                                 int32*  colMap,
                                 int32  size
                                )

{
  if  (size == 2)
  {
    double* topCols = data[rowMap[0]];
    double* botCols = data[rowMap[1]];
    return  topCols[colMap[0]] * botCols[colMap[1]] - topCols[colMap[1]] * botCols[colMap[0]];
  }

  double* coFactors = data[rowMap[0]];
  double  det  = 0.0;
  int32   newSize = size - 1;
  int32   row;
  int32   sign = 1;


  int32*  newRowMap = new int32[newSize];

  for  (row = 1; row < size; row++)
  {
    newRowMap[row - 1] = rowMap[row];
  }

  int32  cfCol;
  for  (cfCol = 0; cfCol < size; cfCol++)
  {
    int32*  newColMap = new int32[newSize];

    int32  oldCol;
    int32  newCol = 0;
    for  (oldCol = 0; oldCol < size; oldCol++)
    {
      if  (oldCol != cfCol)
      {
        newColMap[newCol] = colMap[oldCol]; 
        newCol++;
      }
    }
   
    det = det + sign * coFactors[colMap [cfCol]] * CalcDeterminent (newRowMap, newColMap, newSize);
    if  (sign > 0)
      sign = -1;
    else
      sign = 1;

    delete  newColMap;
  }

  delete  newRowMap;

  return  det;
}  /* CalcDeterminent */





MatrixPtr  Matrix::CalcCoFactorMatrix ()
{
  if  (numOfCols != numOfRows)
  {
    cerr << std::endl
         << "Matrix::CalcCoFactors   **** ERROR ****,  Matrix not a Square["
         << numOfRows << "," << numOfCols << "]."
         << std::endl
         << std::endl;
    osWaitForEnter ();
    exit (1);
  }

  MatrixPtr  result = new Matrix (numOfRows, numOfCols);

  int32  newSize = numOfCols - 1;

  int32*  colMap = new int32 [newSize];
  int32*  rowMap = new int32 [newSize];

  int32  row;
  int32  col;
  int32  x;


  int32  sign;  

  for  (row = 0; row < numOfRows; row++)
  {
    // Create a map of all rows except row we are calculating 
    // CoFactors for.

    int32  newRow = 0;
    for  (x = 0; x < numOfRows; x++)
    {
      if  (x != row)
      {
        rowMap[newRow] = x;
        newRow++;
      }
    }

    for  (col = 0; col < numOfCols; col++)
    {
      // Create a map of all cells except row we are calculating 
      // CoFactor for.

      int32  newCol = 0;
      for  (x = 0; x < numOfCols; x++)
      {
        if  (x != col)
        {
          colMap[newCol] = x;
          newCol++;
        }
      }

      if  (((row + col) % 2) == 0)
        sign = 1;
      else 
        sign = -1;


      Matrix  temp (newSize, newSize);
      for  (int32 r = 0;  r < newSize;  r++)
      {
        int32  tempR = rowMap[r];
        for  (int32 c = 0;  c < newSize;  c++)
          temp[r][c] = data[tempR][colMap[c]]; 
      }

      result->data[row][col] = temp.Determinant () * sign;
    }
  }

  return  result;
} /* CalcCoFactor  */





bool  Matrix::Symmetric ()  const
{
  if  ((data == NULL)  ||  (numOfRows != numOfCols))
    return false;

  for  (int32 row = 0;  row < numOfRows;  ++row)
  {
    for  (int32 col = row + 1;   col < numOfCols;  ++col)
    {
      if  (data[row][col] != data[col][row])
        return false;
    }
  }
  return  true;
}  /* Symetric */




Matrix  Matrix::Transpose ()
{
  int32   col;
  int32   row;

  Matrix  result (numOfCols, numOfRows);

  double**  resultData = result.data;

  for  (row = 0; row < numOfRows; row++)
  {
    for  (col = 0; col < numOfCols; col++)
    {
      resultData[col][row] = data[row][col];
    }
  }

  return  result;
}  /* Transpose */





Matrix  Matrix::Inverse ()
{
  if  (numOfCols != numOfRows)
  {
    KKStr  msg (80);
    msg <<  "Matrix::Inverse   *** ERROR ***   Dimensions are not Square[" << numOfRows << "," << numOfCols << "]  Invalid.";

    cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

  double  det = Determinant ();
  if  (det == 0)
  {
    cerr << std::endl << "Matrix::Inverse   *** ERROR ***   Determinant of Matrix is Zero." << std::endl << std::endl;
  }

  MatrixPtr  coFactors = CalcCoFactorMatrix ();

  Matrix  result = coFactors->Transpose ();

  delete  coFactors;

  if  (det == 0.0)
    result *= 0.0;
  else
    result *= (1.0 / det);

  return  result;
} /* Inverse */



/**
 *@brief  Will derive the eigen vectors and values of the matrix.
 *@details Will make use of routines from Numerical Recipes for c++, ex:  Tred2 and Tqli.
 */
void  Matrix::EigenVectors (MatrixPtr&      eigenVectors,
                            VectorDouble*&  eigenValues
                           )  const
{
  eigenVectors = NULL;
  eigenValues  = NULL;
  if  ((data == NULL)  ||  (numOfRows < 1))
  {
    cerr << std::endl << "Matrix::EigenVectors   ***ERROR***   'data' not defined in Matrix." << std::endl << std::endl;
    return;
  }
  
  if  (numOfRows != numOfCols)
  {
    cerr << std::endl << "Matrix::EigenVectors   ***ERROR***   Not a square matrix  NumOfRows[" << numOfRows << "]  NumOfCols[" << numOfCols << "]." << std::endl << std::endl;
    return;
  }

  eigenVectors = new Matrix (*this);

  if  (Symmetric ())
  {
    double*  d = new double[numOfRows];
    double*  e = new double[numOfRows];
    for  (int32 x = 0;  x < numOfRows;  ++x)
    {
      d[x] = 0.0;
      e[x] = 0.0;
    }
    Tred2 (eigenVectors->data, numOfRows, d, e);
    int32  successful = Tqli (d, e, numOfRows, eigenVectors->data);
    if  (successful != 1)
    {
      delete  eigenVectors;  eigenVectors = NULL;
      delete  d;             d = NULL;
      delete  e;             e = NULL;
      return;
    }

    eigenValues = new VectorDouble ();
    for  (int32 x = 0;  x < numOfRows;  ++x)
      eigenValues->push_back (d[x]);

    delete  d;  d = NULL;
    delete  e;  e = NULL;
  }
}  /* GetEigenVectors */



void   Matrix::FindMaxValue (double&  maxVal, 
                             int32&     rowIdx, 
                             int32&     colIdx
                            )
{
  rowIdx = -1;
  colIdx = -1;
  maxVal = DBL_MIN;

  if  (!data)
    return;

  maxVal = data[0][0];
  rowIdx = colIdx = 0;

  for  (int32 row = 0;  row < numOfRows;  ++row)
  {
    double*  dataRow = data[row];
    for  (int32 col = 0;  col < numOfCols;  ++col)
    {
      if  (dataRow[col] > maxVal)
      {
        maxVal = dataRow[col];
        colIdx = col;
        rowIdx = row;
      }
    }
  }

  return;
}  /* FindMaxValue */




ostream&  operator<< (      ostream&  os, 
                      const Matrix&   matrix
                     )
{
  int32  col;
  int32  row;

  os << "[" << matrix.NumOfRows () << "," << matrix.NumOfCols () << "]" << std::endl;

  os << "[";

  for  (row = 0; row < matrix.NumOfRows (); row++)
  {
    if  (row  > 0)
      os << " ";

    os << "[";

    for  (col = 0; col < matrix.NumOfCols (); col++)
    {
      if  (col > 0)
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



VectorDouble  Matrix::GetCol (int32 col)  const
{
  VectorDouble  colResult (numOfRows, 0.0);
  for  (int32 r = 0;  r < numOfRows;  r++)
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
double Matrix::DeterminantSwap (double**        mat, 
                                unsigned short  offset
                               )

{
	if  (fabs(mat[offset][offset]) < DBL_EPSILON)
	{
		for  (ushort i = offset + 1;  i < numOfRows;  i++)
		{
			if  (fabs (mat[i][offset]) >= DBL_EPSILON)
			{
				/*
				 * Swap line `i' and line `offset'
				 */
				double	*tmp;

				tmp=mat[offset];
				mat[offset]=mat[i];
				mat[i]=tmp;
				break;
			}
		}
	}

	return (mat[offset][offset]);
}  /* DeterminantSwap */






/**
 * @return the determinant of mat
 */
double   Matrix::Determinant ()
{
  if  (numOfCols != numOfRows)
    return -999999.99;

  int32  r, c;

  double** mat = new double*[numOfRows];
  for  (r = 0;  r < numOfRows;  r++)
  {
    mat[r] = new double[numOfCols];
    for  (c = 0;  c < numOfCols;  c++)
    {
      mat[r][c] = data[r][c];
    }
  }

	double det = 1.0;
	unsigned short i = 0;

	for  (i = 0;  i < numOfRows;  i++)
	{
		const double  Aii = DeterminantSwap (mat, i);
		unsigned short j = 0;

		if  (fabs (Aii) < DBL_EPSILON)
		{
			det = 0.0;
			break;
		}

		det *= Aii;
		
		//Do elimination
		for  (j = i + 1;  j < numOfRows;  j++)
		{
			const double pivot = mat[j][i] / Aii;
			for (ushort k = i;  k < numOfRows;  k++)
				mat[j][k] -= pivot * mat[i][k];
		}
	}

  for  (r = 0;  r < numOfRows;  r++)
  {
    delete  mat[r];
    mat[r] = NULL;
  }
  delete  mat;
  mat = NULL;
	return (det);
}  /* Determinant */


/**
 *@brief  Returns a Covariance matrix.
 *@details  Each column represents a variable and each row represents an instance of each variable.
 *@return  Returns a symmetric matrix that will be (numOfRows x numOfRows) where each element will represent the covariance 
 *         between their respective variables.
 */
MatrixPtr  Matrix::Covariance ()  const
{
  if  ((data == NULL)  ||  (numOfRows < 1))
  {
    cerr << std::endl << "Matrix::Covariance   ***ERROR***   'data' not defined in Matrix." << std::endl << std::endl;
    return NULL;
  }

  // Used web site below to help with Covariance calcs.
  //  http://www.itl.nist.gov/div898/handbook/pmc/section5/pmc541.htm

  int32  col = 0;
  int32  row = 0;

  double*   totals       = new double[numOfCols];
  double*   means        = new double[numOfCols];
  double**  centeredVals = new double*[numOfCols];
  for  (col = 0;  col < numOfCols;  ++col)
  {
    totals[col] = 0.0;
    centeredVals[col] = new double[numOfRows];
  }

  for  (row = 0;  row < numOfRows;  ++row)
  {
    double*  rowData = data[row];
    for  (col = 0;  col < numOfCols;  ++col)
      totals[col] += rowData[col];
  }

  for  (col = 0;  col < numOfCols;  ++col)
    means[col] = totals[col] / numOfRows;

  for  (row = 0;  row < numOfRows;  ++row)
  {
    double*  rowData = data[row];
    for  (col = 0;  col < numOfCols;  ++col)
      centeredVals[col][row] = rowData[col] - means[col];
  }
 
  MatrixPtr  covariances = new Matrix (numOfCols, numOfCols);

  for  (int32 varIdxX = 0;  varIdxX < numOfCols;  ++varIdxX)
  {
    double*  varXs = centeredVals[varIdxX];
    for  (int32 varIdxY = varIdxX;  varIdxY < numOfCols;  ++varIdxY)
    {
      // Calc the covariance between chanIdx0 and chanIdx1

      double*  varYs = centeredVals[varIdxY];
      double total = 0.0f;
      for  (row = 0;  row < numOfRows;  ++row)
        total += varXs[row] * varYs[row];
      (*covariances)[varIdxX][varIdxY] = total / (numOfRows - 1);
      (*covariances)[varIdxY][varIdxX]  = (*covariances)[varIdxX][varIdxY];
    }
  }

  for  (col = 0;  col < numOfCols;  col++)
  {
    delete  centeredVals[col];
    centeredVals[col] = NULL;
  }
  delete  centeredVals;   centeredVals = NULL;
  delete  means;          means  = NULL;
  delete  totals;         totals = NULL;

  return  covariances;
}  /* Covariance */




/**
 *@brief  Householder reduction of a real symmetric matrix a[0..n-1][0..n-1].
 *@details  From Numerical Recipes in c++, page 479.  Tred2 is designed to work with Tqli.  Its purpose 
 *          is to reduce the matrix 'a' from a symmetric matrix to a orthogonal matrix.
 *@param[in,out]  a  On output a is replaced by the orthogonal matrix Q effecting the transform.
 *@param[in]      n  Size of matrix is (n x n).
 *@param[out]     d  Returns the the diagonal elements of the tridiagonal matrix,
 *@param[out]     e  The off-diagonal elements with e[0] = 0.
 */
void  Matrix::Tred2 (double** a, 
                     int32    n, 
                     double*  d, 
                     double*  e
                    )  const
{
  int32  i, j, k, l;

  double  scale, hh, h, g, f;

  for (i = n - 1; i > 0;  i--)
  {
    l = i - 1;
    h = scale = 0.0;

    if  (l > 0)
    {
      for (k = 0;  k < l + 1;  k++)
        scale += fabs (a[i][k]);

      if  (scale == 0.0)
      {
        e[i] = a[i][l];
      }

      else
      {
        for (k = 0;  k < l + 1;  k++)
        {
          a[i][k] /= scale;       // Use scaled a's for transformation
          h+= a[i][k] * a[i][k];  // h = sigma
        }

        f = a[i][l];
        g = (f > 0 ? -sqrt(h) : sqrt(h));
        e[i] = scale * g;
        h -= f * g;
        a[i][l] = f - g;
        f = 0.0;

        for  (j = 0;  j < l + 1;  j++)
        {
          a[j][i] = a[i][j] / h;
          g = 0.0;
          for  (k = 0;  k < j + 1;  k++)
            g += a[j][k] * a[i][k];
          
          for  (k = j + 1;  k < l + 1;  k++)
            g += a[k][j] * a[i][k];

          e[j] = g / h;

          f += e[j] * a[i][j];
        } /* for  (j = 0; */

        hh = f / (h + h);
        
        for  (j = 0;  j < l + 1;  j++)
        {
          f = a[i][j];
          e[j] = g = e[j] - hh * f;
          for  (k = 0;  k < j + 1;  k++)
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

  d[0] = 0.0;
  e[0] = 0.0;

  for  (i = 0;  i < n;  i++)
  {
    l = i;

    if  (d[i] != 0.0)
    {
      for  (j = 0;  j < l;  j++)
      {
        g = 0.0;
        for  (k = 0;  k < l;  k++)
          g += a[i][k] * a[k][j];

        for  (k = 0;  k < l;  k++)
          a[k][j] -= g * a[k][i];
      }
    }

    d[i] = a[i][i];

    // Reset row and column of 'a' to identity matrix;
    a[i][i] = 1.0;
    for  (j = 0;  j < l;  j++)
      a[j][i] = a[i][j] = 0.0;
  }  /* for (i) */

}  /* Tred2 */




template<class T>
inline const T SIGN (const T& a,
                     const T& b
                    )
{
  if  (b >= 0)
  {
    if  (a >= 0)
      return  a;
    else
      return -a;
  }

  else
  {
    if  (a >- 0)
      return -a;
    else
      return  a;
  }
}  /* SIGN */


#define  SQR(X)  ((X) * (X))

// Computes sqrt (a^2 + b^2)  without destructive underflow or overflow
double  Matrix::Pythag (const double a,
                        const double b
                       )  const
{
  double  absa, absb;

  absa = fabs (a);
  absb = fabs (b);

  if  (absa > absb)
  {
    return  absa * sqrt (1.0 + SQR (absb / absa));
  }
  else
  {
    if  (absb == 0.0)
      return 0.0;
    else 
      return absb * sqrt (1.0 + SQR (absa / absb));
  }
}  /* pythag */



//#undef SIGN

//#define SIGN(a,b) ((b)<0 ? -fabs(a) : fabs(a))



/**
 *@brief  Determines the eigenvalues and eigen-vectors of a real symmetric, tridiagonal matrix previously reduced by Tred2.
 *@details TQLI = "Tridiagonal QL Implicit".
 *@param[in,out]   d contains the diagonal elements of the tridiagonal matrix.  On output will contain the eigenvalues.
 *@param[in]       e On input contains the sub-diagonal of the tridiagonal matrix, with e[0] arbitrary.  On output e is destroyed.
 *@param[in]       n Size of matrix side.
 *@param[in,out]   z If eigen-vectors of Tridiagonal matrix are required input as a identity matrix[0..n-1][o..n-1].  If
 *                   eigen-vectors of input matrix to "Tred2" are required then z should equal the output matrix from
 *                   Tred2 "a".
 */
int32  Matrix::Tqli (double*  d, 
                   double*  e, 
                   int32    n, 
                   double** z
                  ) const
{
  int32  m, l, iter, i, k;
  double  s, r, p,g, f, dd, c, b;

  for  (i = 1;  i < n;  i++)
    e[i - 1] = e[i];

  e[n - 1] = 0.0;

  for  (l = 0;  l < n;  l++)
  {
    iter = 0;

    do
    {
      for  (m = l;  m < n - 1;  m++)
      {
        // Looking for a single small sub-diagonal element
        // to split the matrix
        dd = fabs (d[m]) + fabs (d[m + 1]);
        if  (fabs (e[m]) + dd == dd)
          break;
      }

      if  (m != l)
      {
        if  (iter == 100)
        {
          cerr << std::endl << std::endl
               << "EigenVector::tqli    **** ERROR ****            To many iterations in tqli" << std::endl
               << std::endl;
          //osWaitForEnter ();
          //exit (-1);
          return  0;
        }
        iter++;

        g = (d[l + 1] - d[l]) / (2.0 * e[l]);
        r = Pythag (g, 1.0);
        g = d[m] - d[l] + e[l] / (g + SIGN (r, g));
        s = c = 1.0;
        p = 0.0;

        for  (i = m - 1;  i >= l;  i--)
        {
          f = s * e[i];
          b = c * e[i];
          e[i + 1] = (r = Pythag (f, g));
          if  (r == 0.0)
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

          for  (k = 0;  k < n;  k++)
          {
            f = z[k][i + 1];
            z[k][i + 1] = s * z[k][i] + c * f;
            z[k][i] = c * z[k][i] - s * f;
          }  /* for (k) */

        }  /* for (i) */

        if  ((r == 0.0)  &&  (i >= l))
          continue;

        d[l] -= p;
        e[l] = g;
        e[m] = 0.0;
      }
    } 
    while  (m != l);
  
  }  /* for (l) */

  return (1);
}  /* Tqli*/

