/* SimpleCompressor.h -- A simple Run Length compression algorithm.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _SIMPLECOMPRESSOR_
#define  _SIMPLECOMPRESSOR_

namespace  KKB
{
  class  SimpleCompressor
  {
  public:
    SimpleCompressor (kkuint32  estimatedMaxBuffSize);

    SimpleCompressor (uchar*  compressdBuff);

    ~SimpleCompressor ();

    void  AddByte (uchar  b);

  
    void  Add16BitInt (kkuint32  i);

    uchar*   CreateCompressedBuffer (kkuint32&  compressedBuffserSize);


    static
    uchar*   Decompress (const uchar*  compressedBuff,
                         kkuint32      compressedBuffLen,
                         kkuint32&     unCompressedSize
                        );


  private:
    void    AddByteToCmpressedBuffer (uchar*    compressedBuff, 
                                      kkuint32& compressedBuffUsed,
                                      uchar     b
                                     );


    kkuint32  CalcCompressedBytesNeeded ();


    uchar*   buffBytes;
    uchar*   buffLens;
    kkuint32 buffSize;
    kkuint32 buffSpaceUsed;

    kkuint32 lastBuffByteUsed;

    kkuint32 growthRate;
  };
}  /* KKB */



#endif
