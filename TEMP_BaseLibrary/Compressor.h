/* Compressor.h -- Compresses and decompresses data using zLib Library
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _COMPRESSOR_
#define  _COMPRESSOR_

#include  "BasicTypes.h"

/**
 * @class  KKU::Compressor
 * @brief  Simple class that will compress and decompress specified buffers using the routines provided in zlib.
 * @author  Kurt Kramer
 */
namespace  KKU
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
                        uint32&        unCompressedLen
                       );
  };
}


#endif
