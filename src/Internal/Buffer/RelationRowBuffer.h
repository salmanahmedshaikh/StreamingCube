//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Buffer/StreamPage.h"
#include "../../Common/Types.h"
#include "../../Internal/Buffer/BufferConstraint.h"
#include <boost/shared_ptr.hpp>
#include "../../Internal/Row/Row.h"
#include "../../Internal/Buffer/AbstractRowBuffer.h"
#include <boost/unordered_map.hpp>

/* a buffer to saves the relation information
 insert row in, and delete row by rowkey
 */
class RowIterator;
class SetRowIterator;
class RelationRowBuffer: public AbstractRowBuffer
{

private:
//boost::unordered_map<DocumentId, MemoryPosition> idPositionMap;
MemoryPosition freePositionHead;
unsigned int trueRowCount;
boost::mutex m_oMutex;

std::vector<std::pair<RowKey, MemoryPosition> > timestampKeyPositionVector;

inline std::vector<std::pair<RowKey, MemoryPosition> >::reverse_iterator reverseFindRowKeyMemoryPosition(Timestamp timestamp, std::vector<DimensionKey> dimensionKey)
{
    std::vector<std::pair<RowKey, MemoryPosition> >::reverse_iterator rIt;
    for(rIt = timestampKeyPositionVector.rbegin(); rIt != timestampKeyPositionVector.rend(); rIt++)
    {
        // if timestamp matches, only then search keys
        if(timestamp == rIt->first.timestamp)
        {
            int matchCount = 0;
            //for(int j = 0; j < rIt->first.dimensionKey.size(); j++)
            for(int i = 0; i < dimensionKey.size(); i++)
                if(dimensionKey[i] == rIt->first.dimensionKey[i])
                        matchCount++; // if all keys match

            if(matchCount == rIt->first.dimensionKey.size())
                return rIt;
        }
    }
    return timestampKeyPositionVector.rend();
}

inline std::vector<std::pair<RowKey, MemoryPosition> >::iterator findRowKeyMemoryPosition(Timestamp timestamp, std::vector<DimensionKey> dimensionKey)
{
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;
    for(it = timestampKeyPositionVector.begin(); it != timestampKeyPositionVector.end(); it++)
    {
        if(timestamp == it->first.timestamp)
        {
            int matchCount = 0;
            //for(int j = 0; j < it->first.dimensionKey.size(); j++)
            for(int i = 0; i < dimensionKey.size(); i++)
                if(dimensionKey[i] == it->first.dimensionKey[i])
                        matchCount++;

            if(matchCount == it->first.dimensionKey.size())
                return it;
        }
    }
    return timestampKeyPositionVector.end();
}

inline std::vector<std::pair<RowKey, MemoryPosition> > findAllRowKeyMemoryPositionsByTimestamp(Timestamp timestamp)
{
    std::vector<std::pair<RowKey, MemoryPosition> > rowKeyMemoryPositionVec;
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;

    for(it = timestampKeyPositionVector.begin(); it != timestampKeyPositionVector.end(); it++)
    {
        if(it->first.timestamp <= timestamp)
        {
            rowKeyMemoryPositionVec.push_back(*it);
        }
        else // The rows with oldest timestamps are in the beginning of the relation
            break;
    }
    return rowKeyMemoryPositionVec;
}


inline std::vector<std::pair<RowKey, MemoryPosition> > reverseFindAllRowKeyMemoryPositionsByTimestamp(Timestamp timestamp)
{
    std::vector<std::pair<RowKey, MemoryPosition> > rowKeyMemoryPositionVec;
    std::vector<std::pair<RowKey, MemoryPosition> >::reverse_iterator rIt;

    for(rIt = timestampKeyPositionVector.rbegin(); rIt != timestampKeyPositionVector.rend(); rIt++)
    {
        if(rIt->first.timestamp == timestamp)
        {
            rowKeyMemoryPositionVec.push_back(*rIt);
        }
        else if(rIt->first.timestamp < timestamp)
        {
            break;
        }
    }
    return rowKeyMemoryPositionVec;
}

public:
RelationRowBuffer(void)
{
    this->freePositionHead.pageIndex = 0;
    this->freePositionHead.chunkIndex = 0;
    //this->trueRowCount = 0; // trueRowCount is the true count of the number of rows before aggregation for the sake of computing average
}

~RelationRowBuffer(void)
{
}

inline void getMax(Row& row)
{
}

inline void getMin(Row& row)
{
}

inline void getSum(Row& row)
{
}

inline void getAvg(Row& row)
{
}

/*
inline unsigned int getRowCount()
{
    assert(this->trueRowCount >= 0);
    if(this->trueRowCount < 0)
    {
        std::cout << "RelationRowBuffer row count can not be negative." << std::endl;
        exit(1);
    }

    return this->trueRowCount;
}
*/

inline void getRowKeyPositionVector(std::vector<std::pair<RowKey, MemoryPosition> >& rowKeyPositionVector)
{
    rowKeyPositionVector = this->timestampKeyPositionVector;
}

inline void insertRow(Row& row) // update or insert row
{
    bool bl = isFull();
    assert(bl==false);

    if (bl == true)
    {
        std::cout << "RelationRowBuffer memory not enough" << std::endl;
        bool bl = isFull();
        exit(1);
    }

    std::vector<std::pair<RowKey, MemoryPosition> >::reverse_iterator rIt;
    boost::mutex::scoped_lock lock(m_oMutex);
    rIt = reverseFindRowKeyMemoryPosition(row.timestamp, row.dimensionKey); // if a row exists with the given timestamp and key update it else insert it

    if (rIt != this->timestampKeyPositionVector.rend()) // if a row exists with the given timestamp and key update it else insert it
    {
        // retrieve the existing row
        Row existingRow;
        MemoryPosition rowPosition = rIt->second;
        this->getRowByMemoryPosition(existingRow, rowPosition);
        //std::cout << "new row " << row << std::endl;
        //std::cout << "existing row " << existingRow << std::endl;

        // update the new row from the contents of existing row
        row.factValue += existingRow.factValue;

        // delete the existing row
        rowPosition = rIt->second;
        this->setNextPosition(rowPosition, freePositionHead);
        this->freePositionHead = rowPosition;

        // todo Check if the following solution (base) really works???
        this->timestampKeyPositionVector.erase(--(rIt.base())); // erase the existing row

        // insert the updated row
        RowKey rowKey;
        rowKey.timestamp = row.timestamp;
        for(int i = 0; i < row.dimensionKey.size(); i++)
            rowKey.dimensionKey.push_back(row.dimensionKey[i]);

        rowPosition = this->freePositionHead;
        this->setRowByMemoryPosition(row, rowPosition);
        this->timestampKeyPositionVector.push_back(make_pair(rowKey, rowPosition)); // new vector insert
        this->getNextPosition(rowPosition, freePositionHead);
        //std::cout << "updated row " << row << std::endl;
        //sleep(1);
    }
    else // insert new row
    {
        RowKey rowKey;
        rowKey.timestamp = row.timestamp;
        for(int i = 0; i < row.dimensionKey.size(); i++)
            rowKey.dimensionKey.push_back(row.dimensionKey[i]);

        MemoryPosition rowPosition = this->freePositionHead;

        this->setRowByMemoryPosition(row, rowPosition);
        //this->idPositionMap.insert(make_pair(row.id, rowPosition));
        this->timestampKeyPositionVector.push_back(make_pair(rowKey, rowPosition)); // new vector insert
        this->getNextPosition(rowPosition, freePositionHead);
        //this->trueRowCount++;
    }
    lock.unlock();
}

inline Timestamp getOldestTupleTimestamp()
{
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;
    it = timestampKeyPositionVector.begin();

    if(it != timestampKeyPositionVector.end()) // some tuple found in timestampKeyPositionVector
    {
        return it->first.timestamp;
    }
    else
        return -1;
}


inline bool deleteRow(Row& row)
{
    bool bl = false;
    //DocumentId id = row.id;
    //boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;

    boost::mutex::scoped_lock lock(m_oMutex);

    it = findRowKeyMemoryPosition(row.timestamp, row.dimensionKey);
    if (it != this->timestampKeyPositionVector.end()) // if row found
    {
        // retrieve the existing row
        MemoryPosition rowPosition = it->second;
        //this->getRowByMemoryPosition(existingRow, rowPosition);

        bl = true;
        //MemoryPosition rowPosition = it->second;
        this->setNextPosition(rowPosition, freePositionHead);
        this->freePositionHead = rowPosition;
        this->timestampKeyPositionVector.erase(it);


    }
    else
    {
        //the row is not in the relation synopsis
    }

    /*
    if (it != this->timestampKeyPositionVector.end()) // if row found
    {
        // retrieve the existing row
        Row existingRow;
        MemoryPosition rowPosition = it->second;
        this->getRowByMemoryPosition(existingRow, rowPosition);

        //std::cout << "arrived row " << row << std::endl;
        //std::cout << "retrieved row " << existingRow << std::endl;

        if(existingRow.factValue-row.factValue <= 0) // if factvalues are equal, delete the row directly
        {
            //std::cout << "direct deletion" << std::endl;

            bl = true;
            //MemoryPosition rowPosition = it->second;
            this->setNextPosition(rowPosition, freePositionHead);
            this->freePositionHead = rowPosition;
            this->timestampKeyPositionVector.erase(it);
        }
        else // subtract the expired row fact values from the existing row fact values
        {
            // create a new row from the existing row by subtracting fact value
            Row newRow;
            newRow.factValue = existingRow.factValue - row.factValue;
            newRow.timestamp = row.timestamp;
            newRow.dimensionKey = row.dimensionKey;

            //delete the existing row
            bl = true;
            this->setNextPosition(rowPosition, freePositionHead);
            this->freePositionHead = rowPosition;
            this->timestampKeyPositionVector.erase(it);

            // insert the updated row
            RowKey rowKey;
            rowKey.timestamp = newRow.timestamp;
            for(int i = 0; i < newRow.dimensionKey.size(); i++)
                rowKey.dimensionKey.push_back(newRow.dimensionKey[i]);

            rowPosition = this->freePositionHead;
            this->setRowByMemoryPosition(newRow, rowPosition);
            this->timestampKeyPositionVector.push_back(make_pair(rowKey, rowPosition));
            this->getNextPosition(rowPosition, freePositionHead);

            //std::cout << "updated row " << newRow << std::endl;
        }
    }
    else
    {
        //the row is not in the relation synopsis
    }
    */

    lock.unlock();

    return bl;
}

inline std::vector<Row> getRowsByTimestamp(Timestamp timestamp)
{
    std::vector<Row> insertedRows;
    std::vector<std::pair<RowKey, MemoryPosition> > rowKeyMemoryPositionVec;
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;

    boost::mutex::scoped_lock lock(m_oMutex);
    rowKeyMemoryPositionVec = reverseFindAllRowKeyMemoryPositionsByTimestamp(timestamp); // Finding all rows whose timestamp is equal or less than timestamp
    lock.unlock();

    if (rowKeyMemoryPositionVec.size() > 0) // if row(s) found
    {
        for(it = rowKeyMemoryPositionVec.begin(); it != rowKeyMemoryPositionVec.end(); it++) // Iterating through all the rows whose timestamp is equal or less than timestamp in order to delete them
        {
            // retrieve the existing row
            Row existingRow;
            MemoryPosition rowPosition = it->second;
            this->getRowByMemoryPosition(existingRow, rowPosition);

            //std::cout << "inserted Row " << std::endl << existingRow << std::endl;
            // Generate -ve tuples for the rows to delete for incremental computing
            insertedRows.push_back(existingRow);
        }
    }
    else
    {
        //the row is not in the relation synopsis
    }

    return insertedRows;
}

inline std::vector<Row> deleteRowsByTimestamp(Timestamp timestamp)
{
    bool bl = false;
    std::vector<Row> deletedRows;

    std::vector<std::pair<RowKey, MemoryPosition> > rowKeyMemoryPositionVec;
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator it;

    //std::cout << "Node size before deletion " << timestampKeyPositionVector.size() << std::endl;
    boost::mutex::scoped_lock lock(m_oMutex);
    rowKeyMemoryPositionVec = findAllRowKeyMemoryPositionsByTimestamp(timestamp); // Finding all rows whose timestamp is equal or less than timestamp
    lock.unlock();

    //std::cout << "rowKeyMemoryPositionVec.size() " << rowKeyMemoryPositionVec.size() << std::endl;
    if (rowKeyMemoryPositionVec.size() > 0) // if row(s) found
    {
        for(it = rowKeyMemoryPositionVec.begin(); it != rowKeyMemoryPositionVec.end(); it++) // Iterating through all the rows whose timestamp is equal or less than timestamp in order to delete them
        {
            // retrieve the existing row
            Row existingRow;
            MemoryPosition rowPosition = it->second;
            this->getRowByMemoryPosition(existingRow, rowPosition);

            //std::cout << "Deleting Row " << std::endl << existingRow << std::endl;
            // Generate -ve tuples for the rows to delete for incremental computing
            Row row = existingRow;
            deletedRows.push_back(row);

            bl = deleteRow(existingRow);

            //this->setNextPosition(rowPosition, freePositionHead);
            //this->freePositionHead = rowPosition;
            //this->timestampKeyPositionVector.erase(it);
        }
    }
    else
    {
        //the row is not in the relation synopsis
    }
    //std::cout << "Node size after deletion " << timestampKeyPositionVector.size() << std::endl;
    return deletedRows;
}



inline bool isFull(void)
{
    bool ret;
    if (!isValidMemoryPosition(freePositionHead))
    {
        ret = this->allocateNewPage();
        if (ret == false)
        {
            return true;
        }
        this->freePositionHead.pageIndex = this->getPageNumber() - 1;
        this->freePositionHead.chunkIndex = 0;

    }

    return false;
}

inline void clear(void)
{
    /*
    boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
    for (it = idPositionMap.begin(); it != idPositionMap.end();)
    {
        MemoryPosition rowPosition = it->second;
        this->setNextPosition(rowPosition, freePositionHead);
        this->freePositionHead = rowPosition;
        it = this->idPositionMap.erase(it);
    }
    */
}

void findRowByRowKey(Row& row, RowKey rowKey)
{
    /*
    boost::unordered_map<DocumentId, MemoryPosition>::iterator it;
    it = this->idPositionMap.find(id);
    //assert(it!=this->idPositionMap.end());
    if (it != this->idPositionMap.end())
    {
        MemoryPosition rowPosition = it->second;
        this->getRowByMemoryPosition(row, rowPosition);
    }
    else
    {
        //the row is not in the relation synopsis
    }
    */
}

//  void getAllRows(std::list<Row>&rowList);
friend class RowIterator;
friend class SetRowIterator;
};
