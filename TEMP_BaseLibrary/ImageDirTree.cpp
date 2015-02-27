/* ImageDirTree.cpp -- Builds list of Image Files in a Sub Directory Tree.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"

#include  <stdio.h>

#include  <vector>
#include  <map>
#include  <algorithm>
#include  <functional>
#include  <iostream>

#include  "MemoryDebug.h"

using namespace std;

#include  "ImageDirTree.h"
#include  "ImageIO.h"
#include  "OSservices.h"
#include  "Str.h"
using namespace KKU;




ImageDirTree::ImageDirTree (KKStr  _subDir):
  subDir     (_subDir)
{
  Load (_subDir);
}



ImageDirTree::~ImageDirTree ()
{
  uint32  x;
  for  (x = 0;  x < directories.size ();  x++)
    delete  directories[x];
}





void  ImageDirTree::Load (const KKStr&  _subDir)
{
  if  (!osValidDirectory (_subDir))
    return;

  EntryTable::const_iterator  existingIdx;
  DuplicateTable::iterator    dupIdx;

  KKStrPtr dirPath = new KKStr (_subDir);
  osAddLastSlash (*dirPath);
  directories.push_back (dirPath);

  KKStr  fileSpec (dirPath);
  fileSpec << "*.*";

  KKStrListPtr  files = osGetListOfFiles (fileSpec);

  if  (files)
  {
    KKStrPtr  fileName = NULL;

    KKStrList::iterator  fIDX;

    for  (fIDX = files->begin ();  fIDX != files->end ();  ++fIDX)
    {
      fileName = *fIDX;
      if  (SupportedImageFileFormat (*fileName))
      {
        existingIdx = entries.find (*fileName);
        if  (existingIdx != entries.end ())
        {
          dupIdx = duplicates.find (fileName);
          if  (dupIdx == duplicates.end ())
            duplicates.insert (pair<KKStr, int32> (*fileName, 2));
          else
            (dupIdx->second)++;
        }

        entries.insert (pair<KKStr, KKStrPtr> (*fileName, dirPath));
      }
    }

    delete  files;
    files = NULL;
  }


  KKStrListPtr  dirs = osGetListOfDirectories (*dirPath);

  if  (dirs)
  {
    KKStrPtr  subDirName = NULL;
    KKStrList::iterator  dIDX;
    for  (dIDX = dirs->begin ();  dIDX != dirs->end ();  ++dIDX)
    {
      subDirName = *dIDX;
      if  ((*subDirName == ".")  ||  (*subDirName == ".."))
        continue;

      Load (*dirPath + *subDirName);
    }

    delete  dirs;
    dirs = NULL;
  }
}  /* Load */






KKStrConstPtr  ImageDirTree::LocateImage (const KKStr&  fileName)
{
  EntryTable::const_iterator p;

  p = entries.find (fileName);
  if  (p == entries.end ())
    return  NULL;

  if  (p->first == fileName)
    return  p->second;

  return  NULL;
}  /* LocateImage */




KKStrListPtr  ImageDirTree::Directories (const KKStr& fileName)  const
{
  EntryTable::const_iterator p;
  p = entries.find (fileName);
  if  (p == entries.end ())
    return NULL;

  KKStrListPtr dirList = new KKStrList (true);
  while  (p != entries.end ())
  {
    if  (p->first != fileName)
      break;
    dirList->PushOnBack (new KKStr ((*p->second)));
  }      

  return  dirList;
}  /* Directories */
