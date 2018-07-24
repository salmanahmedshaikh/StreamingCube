//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Row/Row.h"
#include <boost/shared_ptr.hpp>
#include "../../Internal/Buffer/RelationRowBuffer.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include "../../Internal/Synopsis/Synopsis.h"
#include "../../Internal/Buffer/RelationRowBuffer.h"

class RowIterator;
class SetRowIterator;

class MaterializedNodeRelation: public Synopsis
{
private:
	boost::shared_ptr<RelationRowBuffer>buffer;

public:
	MaterializedNodeRelation(void);
	virtual ~MaterializedNodeRelation(void);

	void insertRow(Row& row);
	bool deleteRow(Row& row);
	std::vector<Row> deleteRowsByTimestamp(Timestamp timestamp);
	std::vector<Row> getRowsByTimestamp(Timestamp timestamp);
	void findRowByRowKey(Row& row, RowKey rowKey);
	void getRowKeyPositionVector(std::vector<std::pair<RowKey, MemoryPosition> >& rowKeyPositionVector);
	void clear(void);
	bool isFull(void);
	Timestamp getOldestTupleTimestamp();


	friend class RowIterator;
	friend class SetRowIterator;
};
