#ifndef  _ATTRIBUTE_
#define  _ATTRIBUTE_

/**
 *@class  KKMachineLearning::Attribute
 *@author  Kurt Kramer
 *@brief describes a single Feature, Type and possible values.
 *@details Used to support 'FileDesc', 'FeatureVector', and 'PostLarvaeFV' classes.  FileDesc 
 *         will maintain a list of 'Attribute' objects to describe each separate field in a given
 *         FeatureFile.  A given Feature can be one of several types, Numeric, Nominal, Ordinal, 
 *         and Symbolic.
 *
 *@code
 * Numeric  - Any floating point value.
 * Nominal  - Feature must be one of a specified possible values.  The Attribute class will
 *            maintain a list of possible values.  This will be saved as a integer value from
 *            in the FeatureVector object.  When training a classifier it would be best to
 *            bit encode these features.  See the FeatureEncoder class.
 * Ordinal  - Similar to Nominal except there is a definite ordering of value.
 * Symbolic - Similar to Nominal except you do not know all the possible values this attribute
 *            can take on.
 *@endcode
 *@see  KKMachineLearning::FeatureEncoder, KKMachineLearning::FeatureVector, KKMachineLearning::FeatureFileIO
*/



#include <map>
#include <vector>

#include "KKStr.h"


namespace KKMachineLearning 
{
  typedef  enum 
  {
    NULLAttribute,
    IgnoreAttribute,
    NumericAttribute,
    NominalAttribute,
    OrdinalAttribute,
    SymbolicAttribute    /**< Same as NominalAttribute, except the names file does not
                          * list all possible values.  They have to be determined from
                          * the data file.
                          */
  } 
  AttributeType;

  typedef  std::vector<AttributeType>  AttributeTypeVector;

  typedef  AttributeTypeVector*        AttributeTypeVectorPtr;


  class  Attribute
  {
  public:
    Attribute (const KKStr&   _name,
               AttributeType  _type,
               kkint32        _fieldNum
              );

    Attribute (const Attribute&  a);

    ~Attribute ();

    void           AddANominalValue (const KKStr&  nominalValue,
                                     bool&         alreadyExists
                                    );

    kkint32        Cardinality (); 

    kkint32        FieldNum ()  const  {return  fieldNum;}

    kkint32        GetNominalCode  (const KKStr&  nominalValue)  const;  // -1 means not found.

    const  
    KKStr&         GetNominalValue (kkint32 code) const;

    kkint32        MemoryConsumedEstimated ()  const;

    const
    KKStr&         Name () const {return name;}

    const
    KKStr&         NameUpper () const {return nameUpper;}
  
    AttributeType  Type () const {return  type;}

    KKStr          TypeStr () const;

    Attribute&     operator= (const Attribute&  right);

    bool  operator== (const Attribute&  rightSide) const;
    bool  operator!= (const Attribute&  rightSide) const;

  private:
    void    ValidateNominalType (const KKStr&  funcName)  const;

    kkint32        fieldNum;
    KKStr          name;
    KKStr          nameUpper;
    KKStrListPtr   nominalValues;
    KKStrListPtr   nominalValuesUpper;
    AttributeType  type;
  };  /* Attribute */

  typedef  Attribute*  AttributePtr;


  class  AttributeList: public KKQueue<Attribute>
  {
  public:
    AttributeList (bool owner);

    ~AttributeList ();

    AttributeTypeVectorPtr  CreateAttributeTypeVector ()  const;

    const
    AttributePtr  LookUpByName (const KKStr&  attributeName)  const;

    kkint32 MemoryConsumedEstimated ()  const;

    void  PushOnBack   (AttributePtr  attribute);

    void  PushOnFront  (AttributePtr  attribute);

    /**
     *@brief  Determines if two different attribute lists are the same.  Compares 
     *        each respective attribute, name and type.                                                
     */
    friend  bool  operator== (const AttributeList&  left,
                              const AttributeList&  right
                             );

    /**
     *@brief  Determines if two different attribute lists are different.  Compares 
     *        each respective attribute, name and type.                                                
     */
    friend  bool  operator!= (const AttributeList&  left,
                              const AttributeList&  right
                             );

  private:
    void  AddToNameIndex (AttributePtr  attribute);
  
    std::map<KKStr, AttributePtr>  nameIndex;
  };  /* AttributeList */

  KKStr  AttributeTypeToStr (AttributeType  type);


  // Created this declaration because MinGW compiler wanted one declared in the namespace;  2013-12-06
  bool  operator== (const AttributeList&  left,
                    const AttributeList&  right
                   );

  bool  operator!= (const AttributeList&  left,
                    const AttributeList&  right
                   );



}  /* namespace KKMachineLearning */

#endif
