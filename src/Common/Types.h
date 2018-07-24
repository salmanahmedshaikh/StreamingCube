//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../BinaryJson/BinaryJson.h"

#include <boost/functional/hash.hpp>

/* this file is used for defining types
 */
typedef long long Timestamp;
typedef int DocumentId;
typedef char Mark;
typedef bool MasterTag;
typedef BinaryJsonObject Document;
typedef BinaryJsonObjectIterator DocumentIterator;
typedef BinaryJsonElement DocumentElement;
typedef BinaryJsonObjectBuilder DocumentBuilder;
typedef BinaryJsonArrayBuilder ArrayBuilder;
typedef unsigned int DimensionKey;
typedef std::string DimensionKeyName;
typedef std::string DimensionKeyValue;
typedef std::string DimensionKeyValueName;
typedef double FactValue;
//typedef long long Timeunit;// the same with timestamp
//typedef long long TimeDuration;
typedef long double Timeunit;// the same with timestamp
typedef long double TimeDuration;
const int TIMESTAMP_SIZE = sizeof(Timestamp);
const int DOCUMENT_IDENTIFIER_SIZE = sizeof(DocumentId);
const int MARK_SIZE = sizeof(Mark);
const int MASTER_TAG_SIZE = sizeof(MasterTag);
const int FACT_VALUE_SIZE = sizeof(FactValue);
const int DIMENSION_KEY_SIZE = sizeof(DimensionKey);

/* the type belongs to the JSON standard */
enum JSONTYPE
{
	JSON_STRING,
	JSON_NUMBER,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_TRUE,
	JSON_FALSE,
	JSON_NULL,
	JSON_INT,
	JSON_FLOAT
	//JSON_DOUBLE
};

struct vertexKeyValue
{
    std::vector<unsigned int> keys;
    float value;
};

struct vertexInfo
{
    mutable int vertexID;
    int level;
    int ordinal;
    int vRows;
    mutable int refFrequency;
    mutable bool isMaterialized;

    bool operator<(const vertexInfo &vInfo)  const
    {
        return level < vInfo.level || (level == vInfo.level && ordinal < vInfo.ordinal);
    }
};

struct vertexID
{
    int level;
    int ordinal;

    bool operator<(const vertexID &vID)  const
    {
        return level < vID.level || (level == vID.level && ordinal < vID.ordinal);
    }
};

struct RowKey
{
    Timestamp timestamp;
    std::vector<DimensionKey> dimensionKey;
};

/* generate JSON enumeration by type string
   example: if type is "string", return type has a value of JSON_STRING
*/
JSONTYPE generateJsonType(std::string type);

/*
  lineage is used to save the lineage information of which document generates the new document
  document identifier is used to specify which document
  for one document, the number of its lineage document is one or two.
*/
typedef struct _Lineage
{
public:
	int lineageNumber;  // one or two
	DocumentId lineageDocumentId[2];


	/* implement this to assure lineage can be used as map keys
	  (0,a)<(1,b)   (a,0)<(a,1)  (1,a)>(0,b)
	  it observes order
	*/
	_Lineage()
	{
		lineageNumber = -1;
		lineageDocumentId[0] = -1;
		lineageDocumentId[1] = -1;
	}

	bool operator < ( const _Lineage& rhs) const
	{
		assert(this->lineageNumber==rhs.lineageNumber);
		if(this->lineageNumber==1)
		{
			return this->lineageDocumentId[0]<rhs.lineageDocumentId[0];
		}
		else if(this->lineageNumber==2)
		{
			if(lineageDocumentId[0]<rhs.lineageDocumentId[0])
			{
				return true;
			}
			else if(lineageDocumentId[0]==rhs.lineageDocumentId[0])
			{
				return lineageDocumentId[1]<rhs.lineageDocumentId[1];
			}
			else
			{
				return false;
			}

		}
		assert(false);//never reached
		return false;
	}
	// output lineage to the standard output
	friend ostream& operator<<(ostream&s,const struct _Lineage&e)
	{
		s<<"lineage start"<<endl;
		s<<"lineage number    : "<<e.lineageNumber<<endl;
		if(e.lineageNumber==1)
		{
			s<<"lineage document0    : "<<e.lineageDocumentId[0]<<endl;
		}
		else
		{
			s<<"lineage document0    : "<<e.lineageDocumentId[0]<<endl;
			s<<"lineage document1    : "<<e.lineageDocumentId[1]<<endl;
		}
		s<<"lineage end"<<endl;
		return s;
	}
}Lineage;
