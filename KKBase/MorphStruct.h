#if  !defined(_MORPHSTRUCT_)
#define  _MORPHSTRUCT_

#include "Raster.h"
using namespace KKB;

/**
 *@brief  Base class for all Morphological Structures; to be used with general purtpose Morphological operations.  
 *@details Meant to operate on one pixel at a time.
 */
class MorphStruct
{
public:
  MorphStruct ();
  virtual ~MorphStruct ();

  void  initalize(RasterPtr  _src, RasterPtr _dest);
  

private:
   RasterPtr  src;
   RasterPtr  dest;

};

#endif