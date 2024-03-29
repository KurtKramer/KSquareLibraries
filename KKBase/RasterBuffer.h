/* RasterBuffer.h -- Implements buffering of raster instances allowing for multiple threads to access the queue safely.
 * Copyright (C) 2011-2014  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_RASTERBUFFER_X_)
#define  _RASTERBUFFER_X_

#include <queue>


#include "KKStr.h"
#include "GoalKeeper.h"

namespace KKB
{
  #if  !defined(_RASTER_)
  class  Raster;
  typedef  Raster*  RasterPtr;

  class  RasterList;
  typedef  RasterList* RasterListPtr;
  #endif

  /**
   *@class  RasterBuffer
   *@brief  Will manage a buffer that will allow multiple threads to add and remove instances of 'Raster' objects.
   *@details
   * A 'GoalKeepeer' object 'gateKeeper' will be used to enforce integrity in the Multi-Threaded environment.  It
   * will guarantee that only one thread at a time can access the Raster Queue.  This queue will take ownership of
   * 'Raster' instances added to it via 'AddRaster'.  It will pass ownership of the 'Raster' instances when it
   * returns them in GetNextRaster.  When a 'RasterBuffer' instance is deleted it will delete all the 'Raster'
   * instances it still contains.
   */
  class  RasterBuffer 
  {
  public:
    typedef  RasterBuffer*  RasterBufferPtr;

    /**
     *@brief  Constructor.
     *@param[in] _name  Name of the buffer; this will be used by the associated 'GateKeeper' instance; should be unique.
     *@param[in] _maxNumOfBuffers The maximum number of raster instances that can be added to this buffer.  When
     *            this limit has been reached the oldest entries in the list will be deleted when a new one is
     *            added (AddRaster).
     */
    RasterBuffer (const KKStr&  _name,
                  size_t        _maxNumOfBuffers
                 );

    ~RasterBuffer ();

    /** @brief  Returns the number of 'Raster' instances that had to be deleted because the size of the queue had reached 'maxNumOfBuffers'. */
    size_t  RastersDropped          () const {return rastersDropped;}

    size_t  MaxNumOfBuffers         () const {return maxNumOfBuffers;}

    /** @brief  The number of entries that are left in the buffer before 'maxNumOfBuffers' is reached. */
    size_t  NumAvailable            () const;

    size_t  NumPopulated            () const;


    /** @brief Returns an estimate of the amount of memory consumed in bytes.
     * @details  This will help managed objects keep track of how much memory they are using in the unmanaged world.
     */
    size_t  MemoryConsumedEstimated () const;


    void  MaxNumOfBuffers (size_t _maxNumOfBuffers)  {maxNumOfBuffers = _maxNumOfBuffers;}


    /** @brief Adds 'raster' to the end of the queue giving the queue ownership of the instance.
     * @details
     *         If the number of entries in the queue are already equal or greater than 'MaxNumOfBuffers' specified
     *         then the oldest instances in the queue(back of the queue) will be removed and deleted until the
     *         size of the queue is less than 'MaxNumOfBuffers' before adding this new instance.
     */
    void  AddRaster (RasterPtr  raster);

    /** @brief  Removes from the buffer the oldest instance of 'Raster' and returns it to caller; if buffer is
     *          empty will return NULL.
     */
    RasterPtr  GetNextRaster ();

    /**@brief Returns a copy of the last Raster instance added to the queue; if buffer is empty will return NULL.
     * @details  Caller will get ownership and be responsible for deleting it.
     */
    RasterPtr  GetCopyOfLastImage ();


  private:
    /** @brief  Remove the oldest 'Raster' instance from the buffer.  */
     void  ThrowOutOldestOccupiedBuffer ();

    std::queue<RasterPtr> buffer;
    GoalKeeperPtr         gateKeeper;
    size_t                maxNumOfBuffers;
    size_t                memoryConsumed;
    KKStr                 name;             /**< Name of buffer. */
    size_t                rastersDropped;   /**< The number of raster instances that had to be deleted because 'maxNumOfBuffers' was reached. */
  };  /* RasterBuffer */


  typedef  RasterBuffer::RasterBufferPtr  RasterBufferPtr;
}  /* KKB */

#endif

