/* ImageDirTree.h -- Builds list of Image Files in a Sub Directory Tree.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _IMAGEDIRTREE_
#define  _IMAGEDIRTREE_
//*******************************************************************************
//*                                ImageDirTree                                 *
//*                                                                             *
//*                                                                             *
//* Purpose:  Given a SubDirectory Starting Point.  Will create a list of all   *
//*           image files in the sub directory tree.  This will allow us to     *
//*           quickly locate a image file by name.  Originally developed it     *
//*           to help in the recalculation of feature data.  Used in            *
//*           ImageFeaturesList.                                                *
//*                                                                             *
//*-----------------------------------------------------------------------------*
//*  Programmer     Date    Description                                         *
//*  ===========  ========  ==================================================  *
//*  Kurt Kramer  04-10-04  Created,  to help aid in Thesis data, feature cal-  *
//*                         culations.                                          *
//*                                                                             *
//*******************************************************************************

//class  DirEntry;
//typedef  DirEntry*  DirEntryPtr;

//typedef  vector<DirEntryPtr>  DirEntryList;


#include  "KKStr.h"


namespace  KKB
{
  /**
   *@brief  Creates a index of all images in a specified directory structure.
   *@details  Given a SubDirectory Starting Point.  Will create a list of all image files in the sub directory 
   * tree.  This will allow us to quickly locate a image file by name.  Originally developed it to help in the
   * recalculation of feature data.  Used in ImageFeaturesList.
   */
  class  ImageDirTree
  {
  public:
    /**
     *@brief  Construct a Index of images for a specified SubDirectory structure.
     * details Given a specified directory '_subDir' will scan for all images in all sub-directories below it.
     *@param[in] _subDir  Sub Directory to search for image files.
     */
    ImageDirTree (KKStr  _subDir);

    ~ImageDirTree ();

    /**
     *@brief  Locate image specified by 'fileName'  and return the directory where it is located.
     *@param[in]  fileName  Image File you are searching for.
     *@return Pointer to directory where 'fileName' is found.
     */
    const KKStrPtr  LocateImage (const KKStr&  fileName);


    /** @brief returns the number of image files found the sub-directory tree. */
    uint32  Size ()  {return  (uint32)entries.size ();}


    /**
     *@brief  Returns list of directories that 'fileName' occurs in.
     *@details The caller will own the list of directories and be responsible for deleting. if 'fileName' does not exist
     * in the sub-directory structure then NULL will be returned.
     *@param[in]  fileName  Name of image file that is being searched for.
     *@return  List of directories that contain a image with the name specified by 'fileName'.  If the file does not exist 
     * a NULL will be returned.  The caller will own the string list returned.
     */
    KKStrListPtr  Directories (const KKStr& fileName)  const;

  private:
    typedef  map<KKStr,int32>          DuplicateTable;
    typedef  multimap<KKStr,KKStrPtr>  EntryTable;
  
    void    Load (const KKStr&  _subDir);

    EntryTable      entries;
    DuplicateTable  duplicates;      /*!< List of Image File Names that are duplicated.  */

    vector<KKStrPtr>  directories;   /*!< List of directories that are searched.         */

    KKStr  subDir;
  };
}  /* namespace KKB */

#endif


