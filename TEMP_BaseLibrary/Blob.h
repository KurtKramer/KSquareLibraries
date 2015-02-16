/* Blob.h -- Works with Raster class to track individual connected component in Raster.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef  _BLOB_
#define  _BLOB_
/**
 *@class KKU::Blob
 *@brief Used by the Raster object to identify a distinct blob; where it is in the raster and its unique id.
 *
 *@details It is used by the Raster object while performing a connected component analysis. For each distinct
 *         blob located an instance of this class will be created.  Its location in the raster and its unique
 *         blob ID will be stored here.  These blobs will later be able to be extracted from the image by
 *         referencing this blob.
 *
 *         The "id" field in this blob will be stored in a array in the associated Raster object. This allows
 *         the Raster object to identify the specific pixels that belong to this Blob. Using the fields "colLeft"
 *         "colRight", "rowBot", and "rowTop" the Raster object will be able to quickly locate the associated
 *         blob.
 * 
 *@see KKU::Raster
 *@see KKU::Raster::ExtractABlob
 *@see KKU::Raster::ExtractABlobTightly
 */                                                                     

#include  "KKQueue.h"
#include  "Point.h"

namespace  KKU
{
  class  Blob
  {
   friend  class BlobList;

  public:
    Blob (int32  _id);

    ~Blob ();

    int32 Id ()  {return  id;}                                      /**< @brief The unique ID assigned to this blob. @see BlobList::nextBlobId  */

    int32  ColLeft ()     {return colLeft;}                         /**< @brief Left most column in the raster object that this blob occupies.  */
    int32  ColRight ()    {return colRight;}                        /**< @brief Right most column in the raster object that this blob occupies. */

    int32  Height ()      {return (1 + abs (rowBot - rowTop));}     /**< @brief Number of rows that the blob occupies.                          */

    int32  RowBot ()      {return rowBot;}                          /**< @brief Bottom row in the raster object that this blob occupies.        */
    int32  RowTop ()      {return rowTop;}                          /**< @brief Top row in the raster object that this blob occupies.           */
    
    int32  Width ()       {return (1 + abs (colLeft - colRight));}  /**< @brief Number of columns that this blob occupies.                      */

    int32  PixelCount ()  {return pixelCount;}                      /**< @brief Number of pixels that are part of this blob.                    */

    Point  TopLeft  ()  const {return Point (rowTop, colLeft);}     /**< @brief Returns a point to the Top Left corner of the blob.             */
    Point  BotRight ()  const {return Point (rowBot, colRight);}    /**< @brief Returns a point to the Bottom Right corner of the blob.         */
    
    int32  colLeft;
    int32  colRight;
    int32  id;
    int32  pixelCount;
    int32  rowBot;
    int32  rowTop;
  };  /* Blob */


  typedef  Blob*  BlobPtr;


  /**
   *@class BlobList
   *@brief Maintains a list of blobs.  
   */                                                                     
  class  BlobList:  public KKQueue<Blob>
  {
  public:
    BlobList (bool  _owner);

    ~BlobList ();





    BlobPtr  LookUpByBlobId (int32  blobId);   /**< @brief  Will return pointer to blob with 'blobId'; if not found will return NULL. */

    BlobPtr  LocateLargestBlob ();             /**< @brief  Will return Blob with largest pixel count. */

    /**
     *@brief  Loactes the blob that covers the greatest part of the raster; that is the one who has the 
     * largest (Height  x  Width)
     */
    BlobPtr  LocateMostComplete ();


    /**
     *@brief  Used by the Connected component analysis to merge two blobs together.
     *@details When the connected component discovers that two separate blobs are actually connected
     *         if will call this method to merge them together into a single blob.
     */
    void     MergeBlobIds (BlobPtr  blob,
                           int32    blobId,
                           int32**  blobIds
                          );

    BlobPtr  MergeIntoSingleBlob (BlobPtr  blob1,
                                  int32    blob2Id,
                                  int32**  blobIds
                                 );

    BlobPtr  NewBlob (uint32  rowTop,
                      uint32  colLeft
                     );

    void  PushOnBack  (BlobPtr  blob);

    void  PushOnFront (BlobPtr  blob);

  private:
    map<int32,BlobPtr>  blobIndex;
    int32  nextBlobId;
  };


  typedef  BlobList*  BlobListPtr;

} /* namespace KKU; */

#endif
