#ifndef  _BMPHEADER_
#define  _BMPHEADER_

#include  "BasicTypes.h"

namespace  KKU
{
/**
 *@file  BMPheader.h
 *@brief  Definition of structures necessary to support BMP file format.
 *@details Because I could not find any data structures in UNIX based systems that would support the
 * reading of BMP files I created this one file that includes data structures that would normally be
 * found on a Windows based OS.
 */

#ifndef WIN32

//typedef  unsigned int32 uint32;

//typedef  unsigned char  uchar;

//typedef  unsigned short ushort;

//typedef unsigned  _int16  WORD;
//typedef unsigned  _int32  DWORD;

typedef  unsigned short  WORD;
typedef  uint32          DWORD;
typedef  int32           LONG;
typedef  unsigned char   BYTE;

#define  BI_RLE4  2
#define  BI_RGB   1
#define  BI_RLE8  3

/** 
 * @struct BITMAPFILEHEADER
 * @brief Windows bitmap header definition used for non-Windows systems
 */

typedef struct BITMAPFILEHEADER { // bmfh 
    WORD    bfType; 
    DWORD   bfSize; 
    WORD    bfReserved1; 
    WORD    bfReserved2; 
    DWORD   bfOffBits; 
} BITMAPFILEHEADER; 
 

/** 
 * @struct BITMAPINFOHEADER
 * @brief Windows bitmap information header definition used for non-Windows systems
 */

typedef struct { // bmih 
    DWORD  biSize;
    LONG   biWidth;
    LONG   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    LONG   biXPelsPerMeter;
    LONG   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} BITMAPINFOHEADER;

/** 
 * @struct RGBQUAD
 * @brief RGBQUAD definition for non-windows systems
 */

typedef struct {
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;


#endif // win32
}

#endif

