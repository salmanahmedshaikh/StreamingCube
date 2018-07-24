//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../../Common/stdafx.h"
#include "../../Internal/Relation/MaterializedNodeRelation.h"

MaterializedNodeRelation::MaterializedNodeRelation()
{
	this->buffer.reset(new RelationRowBuffer());
}

MaterializedNodeRelation::~MaterializedNodeRelation(void)
{
}

// insertRow must check for existing row with same timestamp and keys to update, if not found then insert
void MaterializedNodeRelation::insertRow(Row& row)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation begin@@@@@@@@@@"<<std::endl;
	std::cout<<"insert row : "<<row<<std::endl;
#endif
	 this->buffer->insertRow(row);
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation end@@@@@@@@@@"<<std::endl;
#endif
}

bool MaterializedNodeRelation::deleteRow(Row& row)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation begin@@@@@@@@@@"<<std::endl;
	std::cout<<"delete row : "<<row<<std::endl;
#endif
	bool bl = this->buffer->deleteRow(row);
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}

Timestamp MaterializedNodeRelation::getOldestTupleTimestamp()
{
    Timestamp ts = this->buffer->getOldestTupleTimestamp();
    return ts;
}

std::vector<Row> MaterializedNodeRelation::deleteRowsByTimestamp(Timestamp timestamp)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation begin@@@@@@@@@@"<<std::endl;
	std::cout<<"delete rows with timestamp : "<<timestamp<<std::endl;
#endif
	std::vector<Row> deletedRows = this->buffer->deleteRowsByTimestamp(timestamp);
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation end@@@@@@@@@@"<<std::endl;
#endif
	return deletedRows;
}

std::vector<Row> MaterializedNodeRelation::getRowsByTimestamp(Timestamp timestamp)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation begin@@@@@@@@@@"<<std::endl;
	std::cout<<"get rows with timestamp : "<<timestamp<<std::endl;
#endif
	std::vector<Row> rowsByTimestamp = this->buffer->getRowsByTimestamp(timestamp);
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation end@@@@@@@@@@"<<std::endl;
#endif
	return rowsByTimestamp;
}

bool MaterializedNodeRelation::isFull(void)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@MaterializedNodeRelation begin@@@@@@@@@@"<<std::endl;
#endif
	bool bl =  this->buffer->isFull();
#ifdef DEBUG
	if(bl== true)
	{
		std::cout<<"the synopsis is full"<<std::endl;
	}
	else
	{
		std::cout<<"the synopsis is not full"<<std::endl;
	}
	std::cout<<"@@@@@@@@MaterializedNodeRelation end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}
void MaterializedNodeRelation::clear(void)
{
	this->buffer->clear();
}

void MaterializedNodeRelation::findRowByRowKey(Row& row, RowKey rowKey)
{
	this->buffer->findRowByRowKey(row, rowKey);
}

void MaterializedNodeRelation::getRowKeyPositionVector(std::vector<std::pair<RowKey, MemoryPosition> >& rowKeyPositionVector)
{
    this->buffer->getRowKeyPositionVector(rowKeyPositionVector);
}
