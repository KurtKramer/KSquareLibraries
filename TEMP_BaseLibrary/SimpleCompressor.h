/* SimpleCompressor.h -- A simple Run Length compression algorithm.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _SIMPLECOMPRESSOR_
#define  _SIMPLECOMPRESSOR_

namespace  KKU
{
  class  SimpleCompressor
  {
  public:
    SimpleCompressor (uint32  estimatedMaxBuffSize);

    SimpleCompressor (uchar*  compressdBuff);

    ~SimpleCompressor ();

    void  AddByte (uchar  b);

  
    void  Add16BitInt (uint32  i);

    uchar*   CreateCompressedBuffer (uint32&  compressedBuffserSize);


    static
    uchar*   Decompress (const uchar*  compressedBuff,
                         uint32        compressedBuffLen,
                         uint32&         unCompressedSize
                        );


  private:
    void    AddByteToCmpressedBuffer (uchar*  compressedBuff, 
                                      uint32&   compressedBuffUsed,
                                      uchar   b
                                     );


    uint32  CalcCompressedBytesNeeded ();


    uchar*   buffBytes;
    uchar*   buffLens;
    uint32   buffSize;
    uint32   buffSpaceUsed;

    uint32   lastBuffByteUsed;

    uint32   growthRate;
  };
}  /* KKU */



#endif
