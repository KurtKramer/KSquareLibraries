/* Blob.h -- Works with Raster class to track individual connected component in Raster.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _BLOB_
#define  _BLOB_
/**
 *@class KKB::Blob
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
 *@see KKB::Raster
 *@see KKB::Raster::ExtractABlob
 *@see KKB::Raster::ExtractABlobTightly
 */                                                                     

#include "KKQueue.h"
#include "Point.h"

namespace  KKB
{
  class  Blob
  {
   friend  class BlobList;

  public:
    Blob (kkint32  _id);

    ~Blob ();

    void  InitialzieAsNew (kkint32 _id);
 

    kkint32 Id ()  {return  id;}                                      /**< @brief The unique ID assigned to this blob. @see BlobList::nextBlobId  */

    kkint32  ColLeft ()     {return colLeft;}                         /**< @brief Left most column in the raster object that this blob occupies.  */
    kkint32  ColRight ()    {return colRight;}                        /**< @brief Right most column in the raster object that this blob occupies. */

    kkint32  Height ()      {return (1 + abs (rowBot - rowTop));}     /**< @brief Number of rows that the blob occupies.                          */

    kkint32  RowBot ()      {return rowBot;}                          /**< @brief Bottom row in the raster object that this blob occupies.        */
    kkint32  RowTop ()      {return rowTop;}                          /**< @brief Top row in the raster object that this blob occupies.           */
    
    kkint32  Width ()       {return (1 + abs (colLeft - colRight));}  /**< @brief Number of columns that this blob occupies.                      */

    kkint32  PixelCount ()  {return pixelCount;}                      /**< @brief Number of pixels that are part of this blob.                    */

    Point  TopLeft  ()  const {return Point (rowTop, colLeft);}       /**< @brief Returns a point to the Top Left corner of the blob.             */
    Point  BotRight ()  const {return Point (rowBot, colRight);}      /**< @brief Returns a point to the Bottom Right corner of the blob.         */
    
    kkint32  colLeft;
    kkint32  colRight;
    kkint32  id;
    kkint32  pixelCount;
    kkint32  rowBot;
    kkint32  rowTop;
  };  /* Blob */


  typedef  Blob*  BlobPtr;


  /**
   *@class BlobList
   *@brief Maintains a list of blobs.  
   */                                                                     
  class  BlobList: public  std::map<kkint32,BlobPtr>
  {
  public:
    BlobList (bool  _owner);

    ~BlobList ();

    
    void  DeleteEntry (BlobPtr  b);

    BlobPtr  LookUpByBlobId (kkint32  blobId);   /**< @brief  Will return pointer to blob with 'blobId'; if not found will return NULL. */

    BlobPtr  LocateLargestBlob ();               /**< @brief  Will return Blob with largest pixel count. */

    /**
     *@brief  Locates the blob that covers the greatest part of the raster; that is the one who has the 
     * largest (Height  x  Width)
     */
    BlobPtr  LocateMostComplete ();



    /**
     *@brief  Used by the Connected component analysis to merge two blobs together.
     *@details When the connected component discovers that two separate blobs are actually connected
     *         if will call this method to merge them together into a single blob.
     */
    void     MergeBlobIds (BlobPtr    blob,
                           kkint32    blobId,
                           kkint32**  blobIds
                          );

    BlobPtr  MergeIntoSingleBlob (BlobPtr    blob1,
                                  kkint32    blob2Id,
                                  kkint32**  blobIds
                                 );

    BlobPtr  NewBlob (kkuint32  rowTop,
                      kkuint32  colLeft
                     );

    void  PushOnBack  (BlobPtr  blob);

    void  PushOnFront (BlobPtr  blob);

  private:
    typedef  std::pair<kkint32,BlobPtr> BlobIndexPair;

    std::vector<BlobPtr>  availableBlobs;
    kkint32               nextBlobId;
    bool                  owner;
  };


  typedef  BlobList*  BlobListPtr;

} /* namespace KKB; */

#endif
