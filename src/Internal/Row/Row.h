//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Common/Types.h"

/*
  OLAP Materialized Node Row contains three parts
  (1) Dimension keys (vector)
  //(2) Document ID - the document identifier generated for each element is also used for rows - removed as not used
  (2) Timestamp
  (3) Value
*/

struct Row
{
private:
	int rowSize;

public:

    std::vector<DimensionKey> dimensionKey;
	Timestamp timestamp;
	//DocumentId id;
	FactValue factValue;

	Row()
	{
		this->rowSize = -1;
		this->timestamp = -1;
		this->factValue = -1;
	}

	Row(Timestamp& timestamp, std::vector<DimensionKey>& dimensionKey, FactValue& factValue)
	{
		this->timestamp = timestamp;
		//this->id = id;
		this->factValue = factValue;
		this->dimensionKey = dimensionKey;

		//this->rowSize = TIMESTAMP_SIZE + DOCUMENT_IDENTIFIER_SIZE + dimensionKey.size() * DIMENSION_KEY_SIZE + FACT_VALUE_SIZE;
		this->rowSize = TIMESTAMP_SIZE + dimensionKey.size() * DIMENSION_KEY_SIZE + FACT_VALUE_SIZE;
	}

	friend ostream& operator<<(ostream&s,const Row&r)
	{
		s<<"Row Start"<<endl;
		s<<"Row Timestamp : "<<r.timestamp<<endl;
		//s<<"Row ID        : "<<r.id<<endl;
		s<<"Row Key       : ";

		for(int i = 0; i < r.dimensionKey.size(); i++)
            s<<r.dimensionKey[i]<<", ";

		s<<endl;
		s<<"Row FactValue : "<<r.factValue<<endl;
		s<<"Row End"<<endl;
		return s;
	}

	int getSize(void)
	{
		if(rowSize==-1)
		{
			//this->rowSize = TIMESTAMP_SIZE + DOCUMENT_IDENTIFIER_SIZE + dimensionKey.size() * DIMENSION_KEY_SIZE + FACT_VALUE_SIZE;
			this->rowSize = TIMESTAMP_SIZE + dimensionKey.size() * DIMENSION_KEY_SIZE + FACT_VALUE_SIZE;

		}
		return rowSize;
	}

};

