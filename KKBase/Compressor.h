/* Compressor.h -- Compresses and decompresses data using zLib Library
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _COMPRESSOR_
#define  _COMPRESSOR_

#include  "KKBaseTypes.h"

/**
 * @class  KKB::Compressor
 * @brief  Simple class that will compress and decompress specified buffers using the routines provided in zlib.
 * @author  Kurt Kramer
 */
namespace  KKB
{
  class  Compressor
  {
  public:
    static
    void*   CreateCompressedBuffer (void*  source,
                                    uint32 sourceLen,
                                    uint32&  compressedBuffLen
                                   );


    static
    void*   Decompress (const void*  compressedBuff,
                        uint32       compressedBuffLen,
                        uint32&      unCompressedLen
                       );

    static
    void    Decompress (const void*  compressedBuff,
                        uint32       compressedBuffLen,
                        uchar*&      unCompressedBuff,
                        uint32&      unCompressedBuffSize,
                        uint32&      unCompressedBuffLen
                       );
  };
}


#endif
