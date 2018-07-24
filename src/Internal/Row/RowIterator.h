//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma  once
#include "../../Common/stdafx.h"
#include "../../Internal/Buffer/RelationRowBuffer.h"
#include "../../Internal/Relation/MaterializedNodeRelation.h"
#include "../../Common/Types.h"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
//this is used to iterate the rows one by one in the relation buffer
class RowIterator
{
private:
	boost::shared_ptr<RelationRowBuffer> relationRowBuffer;
	//boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
	std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;

public:
	RowIterator(boost::shared_ptr<MaterializedNodeRelation> mNodeRelation)
	{
		this->relationRowBuffer = mNodeRelation->buffer;
	}

	void initial(void)
	{
		//it = relationRowBuffer->idPositionMap.begin();
		it = relationRowBuffer->timestampKeyPositionVector.begin();
	}

	bool getNext(Row& row)
	{
		/*
		if(it==relationRowBuffer->idPositionMap.end())
		{
			return false;
		}

		MemoryPosition& memoryPosition = it->second;
		relationRowBuffer->getRowByMemoryPosition(row, memoryPosition);
		it++;
		*/

		if(it==relationRowBuffer->timestampKeyPositionVector.end())
		{
			return false;
		}

		MemoryPosition& memoryPosition = it->second;
		relationRowBuffer->getRowByMemoryPosition(row, memoryPosition);
		it++;

		return true;
	}
};

//iterator the rows in the relation synopsis by a set of ducumentid
class SetRowIterator
{
private:
	boost::shared_ptr<RelationRowBuffer> relationRowBuffer;
	//std::set<DocumentId> documentIdSet;
	//std::set<DocumentId>::iterator it;
	std::set<RowKey> rowKeySet;
	std::set<RowKey>::iterator it;

public:
	SetRowIterator(boost::shared_ptr<MaterializedNodeRelation> mNodeRelation, std::set<RowKey> rowKeySet)
	{
		this->relationRowBuffer = mNodeRelation->buffer;
		//this->documentIdSet = documentIdSet;
		this->rowKeySet = rowKeySet;
	}

	void initial(void)
	{
		it = rowKeySet.begin();
	}

	bool getNext(Row& row)
	{
		if(it==rowKeySet.end())
		{
			return false;
		}

		//DocumentId id = *it;
		RowKey id = *it;

		//boost::unordered_map<DocumentId, MemoryPosition>::iterator itt = relationRowBuffer->idPositionMap.find(id);
		std::vector<std::pair<RowKey, MemoryPosition> >::iterator itt = relationRowBuffer->findRowKeyMemoryPosition(id.timestamp, id.dimensionKey);

		assert(itt!=relationRowBuffer->timestampKeyPositionVector.end());
		MemoryPosition& memoryPosition = itt->second;
		relationRowBuffer->getRowByMemoryPosition(row, memoryPosition);
		it++;

		return true;
	}

	bool getThis(Row& row)
	{
			if (it == rowKeySet.end())
			{
				return false;
			}

			//DocumentId id = *it;
            RowKey id = *it;

			//boost::unordered_map<DocumentId, MemoryPosition>::iterator itt = relationRowBuffer->idPositionMap.find(id);
			std::vector<std::pair<RowKey, MemoryPosition> >::iterator itt = relationRowBuffer->findRowKeyMemoryPosition(id.timestamp, id.dimensionKey);

			assert(itt != relationRowBuffer->timestampKeyPositionVector.end());
			MemoryPosition& memoryPosition = itt->second;
			relationRowBuffer->getRowByMemoryPosition(row, memoryPosition);

			return true;
		}
};
