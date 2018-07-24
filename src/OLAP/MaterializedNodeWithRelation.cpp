//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../OLAP/MaterializedNodeWithRelation.h"

MaterializedNodeWithRelation::MaterializedNodeWithRelation(int nodeID, int level, int ordinal, int nodeSizeInNumOfRows, std::vector<std::string>& dimensionNames)
{
    this->nodeID = nodeID;
    this->latticeLevel = level;
    this->latticeOrdinal = ordinal;
    this->dimAttributeNames = dimensionNames;
    this->nodeSizeInRows = nodeSizeInNumOfRows;
    this->oldestTupleTimestampInVertex = -1;

    // this->mNodeRelation is a relation just like a relation synopsis
    this->mNodeRelation.reset(new MaterializedNodeRelation());
    this->rowIterator.reset(new RowIterator(this->mNodeRelation));
}

MaterializedNodeWithRelation::~MaterializedNodeWithRelation()
{
    //dtor
}

int MaterializedNodeWithRelation::getNodeID()
{
    return nodeID;
}

int MaterializedNodeWithRelation::getNodeLevel()
{
    return latticeLevel;
}

int MaterializedNodeWithRelation::getNodeOrdinal()
{
    return latticeOrdinal;
}

int MaterializedNodeWithRelation::getNodeSizeInRows()
{
    return nodeSizeInRows;
}

void MaterializedNodeWithRelation::getNodeDimensions(std::vector<DimensionKeyName>& nodeDimensions)
{
    nodeDimensions = this->dimAttributeNames;
}

void MaterializedNodeWithRelation::insertKeyTimestampValue(std::vector<DimensionKey> keys, Timestamp timestamp, double value, Mark mark, Timestamp IoA, std::vector<Element>& plusMinusElements, bool generateElements, std::vector< std::vector<DimensionKeyName> >& generateElementsFor)
{
    Row row;
    std::vector<Row> deletedRows;
    std::vector<Row> completedRows;
    std::vector<Row> aggregatedRows;
    std::vector<Row>::iterator rowIt;
    recentCompletedTimestampInVertex = -1;

    //row.id = elementID;
    row.timestamp = timestamp;
    row.factValue = value;
    row.dimensionKey = keys;

    // ADDITION of Rows to Relation
    if(this->mNodeRelation->isFull())
    {
        throw runtime_error("Materialized Node Relation is Full");
    }
    this->mNodeRelation->insertRow(row); // insert or update row
    //std::cout << row << std::endl;

    // Required for generation of insertion(+) elements, of the completed timestamp
    if( std::find(timestampList.begin(), timestampList.end(), timestamp) == timestampList.end() ) //if timestamp not already in the list then insert
    {
        timestampList.push_back(timestamp); // pushing new timestamp
        //std::cout << "inserted timestamp " << timestamp << std::endl;
        //std::cout << "list size " << timestampList.size() << std::endl;

        if(timestampList.size() > 1)
        {
            recentCompletedTimestampInVertex = timestampList.front();
            timestampList.pop_front();
        }
    }

    //Generating PLUS elements that need to be sent to downstream opeartors.
    std::vector< std::vector<DimensionKeyName> >::iterator it;
    if(recentCompletedTimestampInVertex != -1)
    {
        //std::cout << "completed timestamp " << recentCompletedTimestampInVertex << std::endl;
        completedRows = this->mNodeRelation->getRowsByTimestamp(recentCompletedTimestampInVertex); // Insertion rows
        //std::cout << "completedRows.size " << completedRows.size() << std::endl;

        // Need to generate output rows if size > 0
        if(generateElements && generateElementsFor.size() > 0)
        {
            // A node may need to generate output for multiple queried vertices
            for(it = generateElementsFor.begin(); it != generateElementsFor.end(); it++)
            {
                aggregatedRows.clear();
                if(this->dimAttributeNames.size() != (*it).size() ) // if the output required node and the current nodes are not same
                {
                    getAggregatedOutputRows(completedRows, aggregatedRows, *it);
                }
                else
                    aggregatedRows = completedRows;

                generateElementsFromRows(aggregatedRows, plusMinusElements, PLUS_MARK);
            }
        }

        /*
        // Direct elements generation without aggregation
        if(generateElements == true)
        {
            generateElementsFromRows(aggregatedRows, plusMinusElements, PLUS_MARK);
        }
        */
    }

    // DELETION of Row from Relation
    oldestTupleTimestampInVertex = this->mNodeRelation->getOldestTupleTimestamp(); // Maintain the oldestTupleTimestampInVertex for IoA calculation

    // IoA check
    if ( (timestamp - oldestTupleTimestampInVertex) >= IoA ) // If rows exceeds IoA limit
    {
        //std::cout << "oldestTupleTimestampInVertex " << oldestTupleTimestampInVertex << std::endl;
        //std::cout << "timestamp - oldestTupleTimestampInVertex " << timestamp - oldestTupleTimestampInVertex << std::endl;
        deletedRows = this->mNodeRelation->deleteRowsByTimestamp(oldestTupleTimestampInVertex); // delete all the rows which exceeds IoA limit // Deletion rows
        oldestTupleTimestampInVertex = this->mNodeRelation->getOldestTupleTimestamp();
        //std::cout << "updated oldestTupleTimestampInVertex " << oldestTupleTimestampInVertex << std::endl;

        // Need to generate output rows if size > 0
        if(generateElements && generateElementsFor.size() > 0)
        {
            // A node may need to generate output for multiple queried vertices
            for(it = generateElementsFor.begin(); it != generateElementsFor.end(); it++)
            {
                aggregatedRows.clear();
                if(this->dimAttributeNames.size() != (*it).size() ) // if the output required node and the current nodes are not same
                {
                    getAggregatedOutputRows(deletedRows, aggregatedRows, *it);
                }
                else
                    aggregatedRows = deletedRows;

                generateElementsFromRows(aggregatedRows, plusMinusElements, MINUS_MARK);
            }
        }

        /*
        // Direct elements generation without aggregation
        if(generateElements == true)
        {
            generateElementsFromRows(deletedRows, plusMinusElements, MINUS_MARK);
        }
        */
    }

    /*
    //boost::mutex::scoped_lock lock(m_oMutex);
    // Mapping elementID to timestamp
    if(mark == PLUS_MARK)
    {
        // Adding Rows to Relation
        if(this->mNodeRelation->isFull())
        {
            throw runtime_error("Materialized Node Relation is Full");
        }
        this->mNodeRelation->insertRow(row); // insert or update row
    }
    else if(mark == MINUS_MARK)
    {
        this->mNodeRelation->deleteRow(row);
    }
    */
    //lock.unlock();
}

void MaterializedNodeWithRelation::generateElementsFromRows(std::vector<Row>& rows, std::vector<Element>& plusMinusElements, Mark mark)
{
    std::vector<Row>::iterator rowIt;
    for(rowIt = rows.begin(); rowIt != rows.end(); rowIt++)
    {
        Row row = *rowIt;
        //creating element out of row
        Element element;
        DocumentBuilder documentBuilder;

        for(int i = 0; i < row.dimensionKey.size(); i++)
            documentBuilder.append("key", boost::lexical_cast<int>(row.dimensionKey[i]));
            //s<<r.dimensionKey[i]<<", ";
        documentBuilder.append("timestamp", boost::lexical_cast<long long>(row.timestamp) );
        documentBuilder.append("factValue",row.factValue);

        Document document = documentBuilder.obj();

        element.timestamp = TimestampGenerator::getCurrentTime();
        element.mark = mark;
        element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
        element.document = document;
        element.masterTag = false;

        //std::cout << "elements " << element << std::endl;
        plusMinusElements.push_back(element);
    }
}

void MaterializedNodeWithRelation::getAggregatedOutputRows(std::vector<Row>& rows, std::vector<Row>& aggregatedRows, std::vector<DimensionKeyName> dimKeyNames)
{
    std::map<std::vector<DimensionKey>, double> aggregatedRowsMap;
    std::map<std::vector<DimensionKey>, double>::iterator aggregatedRowsMapIt;
    Timestamp ts;
    std::vector<Row>::iterator rowIt;
    std::vector<int> requiredIndicesInMap;

    for(std::vector<DimensionKeyName>::iterator it = dimKeyNames.begin(); it != dimKeyNames.end(); it++)
        for(std::vector<DimensionKeyName>::iterator itt = this->dimAttributeNames.begin(); itt != this->dimAttributeNames.end(); itt++)
            if(*it == *itt)
            {
                requiredIndicesInMap.push_back(itt - this->dimAttributeNames.begin());
            }

    // Inserting rows into map for aggreagtaion
    for(rowIt = rows.begin(); rowIt != rows.end(); rowIt++)
    {
        std::vector<DimensionKey> dimKey;
        Row row = *rowIt;
        ts = row.timestamp;
        //std::cout << row << std::endl;

        for(int i = 0; i < row.dimensionKey.size(); i++)
        {
            if( std::find(requiredIndicesInMap.begin(), requiredIndicesInMap.end(), i) !=  requiredIndicesInMap.end() ) // if i found in the requiredIndicesInMap
                dimKey.push_back(row.dimensionKey[i]);
        }


        if(aggregatedRowsMap.find(dimKey) !=  aggregatedRowsMap.end()) // if key found in map
            aggregatedRowsMap[dimKey] += row.factValue;
        else
            aggregatedRowsMap.insert(std::make_pair(dimKey,row.factValue));
    }

    // generating aggregated rows
    for(aggregatedRowsMapIt = aggregatedRowsMap.begin(); aggregatedRowsMapIt != aggregatedRowsMap.end(); aggregatedRowsMapIt++)
    {
        Row row;

        row.timestamp = ts;
        row.factValue = aggregatedRowsMapIt->second;
        row.dimensionKey = aggregatedRowsMapIt->first;

        aggregatedRows.push_back(row);
        //std::cout << "Aggregated " << std::endl << row << std::endl;
    }

    //std::cout << "rows.size() before aggregation " << rows.size() << std::endl;
    //std::cout << "rows.size() after aggregation " << aggregatedRows.size() << std::endl;
    //sleep(1);
}

// Returns aggregate of all the values for all the keys in this node, w.r.t. time grain, for the given aggregate function
void MaterializedNodeWithRelation::getAggregatedValueAllKeysWTimeGrain(std::string aggregateFunction, std::string OLAPTimeGrain, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap)
{
    boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator OLAPKeyTimeValueMapIt;

    // Iterate through all the rows of the materialized node to aggregate w.r.t. timestamp required and the aggregation function
    boost::mutex::scoped_lock lock(m_oMutex);
    this->rowIterator->initial();
	Row row;
	while(rowIterator->getNext(row))
	{
        //row.dimensionKey.clear();
        Timestamp OLAPQueriedTimestamp = getOLAPQueriedTimestamp(OLAPTimeGrain, row.timestamp);

        if((OLAPKeyTimeValueMapIt = OLAPKeyTimeValueMap.find(row.dimensionKey)) != OLAPKeyTimeValueMap.end()) // if record found for the given key, update existing record
        {
            if(aggregateFunction == "sum")
            {
                OLAPKeyTimeValueMap[row.dimensionKey][OLAPQueriedTimestamp] += row.factValue;
            }
            else if(aggregateFunction == "min")
            {
                if(row.factValue < OLAPKeyTimeValueMap[row.dimensionKey][OLAPQueriedTimestamp])
                    OLAPKeyTimeValueMap[row.dimensionKey][OLAPQueriedTimestamp] = row.factValue;
            }
            else if(aggregateFunction == "max")
            {
                if(row.factValue > OLAPKeyTimeValueMap[row.dimensionKey][OLAPQueriedTimestamp])
                    OLAPKeyTimeValueMap[row.dimensionKey][OLAPQueriedTimestamp] = row.factValue;
            }
            else if(aggregateFunction == "avg")
            {
                std::cout << "MNode: Function not supported!" << std::endl;
            }
            else if(aggregateFunction == "count")
            {
                std::cout << "MNode: Function not supported!" << std::endl;
            }
            else
            {
                std::cout << "MNode: Not a valid Aggregation Function!" << std::endl;
            }

        }
        else // insert new record
        {
            OLAPKeyTimeValueMap[row.dimensionKey][OLAPQueriedTimestamp] = row.factValue;
        }
	}
	lock.unlock();

    /*
    std::vector<std::pair<RowKey, MemoryPosition> > rowKeyPositionVector;
    std::vector<std::pair<RowKey, MemoryPosition> >::iterator rowKeyPositionVectorIt;
    this->mNodeRelation->getRowKeyPositionVector(rowKeyPositionVector);

    for(rowKeyPositionVectorIt = rowKeyPositionVector.begin(); rowKeyPositionVectorIt != rowKeyPositionVector.end(); rowKeyPositionVectorIt++)
    {
        //this->mNodeRelation->
    }
    */

    /*
    boost::mutex::scoped_lock lock(m_oMutex);
    // Aggregate for each key w.r.t. time grain required
    for(keyTimestampValueMapIt = keyTimestampValueMap.begin(); keyTimestampValueMapIt != keyTimestampValueMap.end(); keyTimestampValueMapIt++)
    {
        std::map<Timestamp, double> timestampValueMap = keyTimestampValueMapIt->second;
        std::map<Timestamp, double> OLAPTimeValueMap;
        std::map<Timestamp, double>::iterator OLAPTimeValueMapIt;

        getAggregatedValueWTimeGrain(aggregateFunction, OLAPTimeGrain, timestampValueMap, OLAPTimeValueMap);

        // Inserting the returned values into the OLAPKeyTimeValueMap
        for(OLAPTimeValueMapIt = OLAPTimeValueMap.begin(); OLAPTimeValueMapIt != OLAPTimeValueMap.end(); OLAPTimeValueMapIt++)
        {
            OLAPKeyTimeValueMap[keyTimestampValueMapIt->first][OLAPTimeValueMapIt->first] = OLAPTimeValueMapIt->second;
        }
    }
    lock.unlock();
    */
}

/*
void MaterializedNodeWithRelation::getAggregatedValueWTimeGrain(std::string aggregateFunction, std::string OLAPTimeGrain, std::map<Timestamp, double>& timestampValueMap, std::map<Timestamp, double>& OLAPTimestampValueMap)
{
    std::map<Timestamp, double>::iterator timestampValueMapIt;

    for(timestampValueMapIt = timestampValueMap.begin(); timestampValueMapIt != timestampValueMap.end(); timestampValueMapIt++)
    {
        Timestamp OLAPQueriedTimestamp = getOLAPQueriedTimestamp(OLAPTimeGrain, timestampValueMapIt->first);

        std::map<Timestamp, double>::iterator OLAPTimestampValueMapIt = OLAPTimestampValueMap.find(OLAPQueriedTimestamp);

        if(OLAPTimestampValueMapIt == OLAPTimestampValueMap.end()) // Timestamp not found
        {
            OLAPTimestampValueMap.insert(std::make_pair(OLAPQueriedTimestamp, timestampValueMapIt->second));
        }
        else // Timestamp found
        {
            if(aggregateFunction == "sum")
            {
                OLAPTimestampValueMap[OLAPQueriedTimestamp] += timestampValueMapIt->second;
            }
            else if(aggregateFunction == "min")
            {
                if(timestampValueMapIt->second < OLAPTimestampValueMapIt->second)
                    OLAPTimestampValueMapIt->second = timestampValueMapIt->second;
            }
            else if(aggregateFunction == "max")
            {
                if(timestampValueMapIt->second > OLAPTimestampValueMapIt->second)
                    OLAPTimestampValueMapIt->second = timestampValueMapIt->second;
            }
            else if(aggregateFunction == "avg")
            {
                std::cout << "MNode: Function not supported!" << std::endl;
            }
            else if(aggregateFunction == "count")
            {
                std::cout << "MNode: Function not supported!" << std::endl;
            }
            else
            {
                std::cout << "MNode: Not a valid Aggregation Function!" << std::endl;
            }
        }
    }
}
*/

Timestamp MaterializedNodeWithRelation::getOLAPQueriedTimestamp(std::string OLAPTimeGrain, Timestamp tupleTimestamp)
{
    if(OLAPTimeGrain == "Second" || OLAPTimeGrain == "second")
    {
        if(!(isnan(tupleTimestamp)))
        {
            return tupleTimestamp;
        }
    }
    else if(OLAPTimeGrain == "Minute" || OLAPTimeGrain == "minute")
    {
        if(!(isnan(tupleTimestamp/60)))
        {
            return (tupleTimestamp/60);
        }
    }
    else if(OLAPTimeGrain == "Hour" || OLAPTimeGrain == "hour")
    {
        if(!(isnan(tupleTimestamp/(60*60))))
        {
            return (tupleTimestamp/(60*60));
        }
    }
    else
    {
        std::cout << "MNode! Not a valid timestamp!" << std::endl;
        assert(false);
        exit(0);
    }
}

// Returns aggregate of all the values for all the keys in this node, w.r.t. whole window, for the given aggregate function
void MaterializedNodeWithRelation::getAggregatedValueAllKeys(std::string aggregateFunction, std::map<std::vector<DimensionKey>, double >& keyValues)
{
    boost::mutex::scoped_lock lock(m_oMutex);

    for(keyTimestampValueMapIt = keyTimestampValueMap.begin(); keyTimestampValueMapIt != keyTimestampValueMap.end(); keyTimestampValueMapIt++)
    {
        double aggregatedValue = getAggregatedValue(keyTimestampValueMapIt->first,aggregateFunction);

        if(!(isnan(aggregatedValue)))
            keyValues.insert(make_pair(keyTimestampValueMapIt->first, aggregatedValue));
    }

    lock.unlock();
}

double MaterializedNodeWithRelation::getAggregatedValue(std::vector<DimensionKey> keys, std::string aggregateFunction)
{
    std::map<Timestamp, double> innerMap;
     // Check if key exists
    if((keyTimestampValueMapIt = keyTimestampValueMap.find(keys)) != keyTimestampValueMap.end())
    {
        innerMap = keyTimestampValueMapIt->second;
    }
    else
    {
        std::cout << "MNode: Queried key does not exist." << std::endl;
        return -1;
    }

    if(aggregateFunction == "sum")
    {
        double sum = getSUM(innerMap);
        if(isnan(sum))
        {
            std::cout << "MNode: Returned SUM is NaN!" << std::endl;
            return -1;
        }
        else
        {
            return sum;
        }
    }
    else if(aggregateFunction == "min")
    {
        double minimum = getMIN(innerMap);
        if(isnan(minimum))
        {
            std::cout << "MNode: Returned MIN is NaN!" << std::endl;
            return -1;
        }
        else
        {
            return minimum;
        }
    }
    else if(aggregateFunction == "max")
    {
        double maximum = getMAX(innerMap);
        if(isnan(maximum))
        {
            std::cout << "MNode: Returned MAX is NaN!" << std::endl;
            return -1;
        }
        else
        {
            return maximum;
        }
    }
    else if(aggregateFunction == "avg")
    {
        double avg = getSUM(innerMap)/innerMap.size();
        if(isnan(avg))
        {
            std::cout << "MNode: Returned AVG is NaN!" << std::endl;
            return -1;
        }
        else
        {
            return avg;
        }
    }
    else if(aggregateFunction == "count")
    {
        return innerMap.size();
    }
    else
    {
        std::cout << "MNode: Not a valid Aggregation Function!" << std::endl;
        return -1;
    }
}

double MaterializedNodeWithRelation::getSUM(std::map<Timestamp, double >& innerMap)
{
    double sum = 0;
    std::map<Timestamp, double >::iterator innerMapIt;

    for(innerMapIt = innerMap.begin(); innerMapIt != innerMap.end(); innerMapIt++)
    {
        std::cout << "Timestamp " << innerMapIt->first << std::endl;
        sum += innerMapIt->second;
    }

    return sum;
}

double MaterializedNodeWithRelation::getMIN(std::map<Timestamp, double >& innerMap)
{
    double minimum = DBL_MAX;
    std::map<Timestamp, double >::iterator innerMapIt;

    for(innerMapIt = innerMap.begin(); innerMapIt != innerMap.end(); innerMapIt++)
    {
        if(innerMapIt->second < minimum)
            minimum = innerMapIt->second;
    }

    return minimum;
}

double MaterializedNodeWithRelation::getMAX(std::map<Timestamp, double >& innerMap)
{
    double maximum = -DBL_MAX;
    std::map<Timestamp, double >::iterator innerMapIt;

    for(innerMapIt = innerMap.begin(); innerMapIt != innerMap.end(); innerMapIt++)
    {
        if(innerMapIt->second > maximum)
            maximum = innerMapIt->second;
    }

    return maximum;
}

double MaterializedNodeWithRelation::getValueAtTimestamp(std::vector<DimensionKey> keys, Timestamp timestamp)
{
    // Check if key exists
    boost::mutex::scoped_lock lock(m_oMutex);

    if((keyTimestampValueMapIt = keyTimestampValueMap.find(keys)) != keyTimestampValueMap.end())
    {
        std::map<Timestamp, double >::iterator innerIt = keyTimestampValueMapIt->second.find(timestamp);

        //If key with the given timestamp found
        if(innerIt != keyTimestampValueMapIt->second.end())
        {
            if(isnan(innerIt->second))
            {
                std::cout << "MNode: Returned value is NaN!" << std::endl;
                return -1;
            }
            else
            {
                return innerIt->second;
            }
        }
        else
        {
            std::cout << "MNode: Key with the given timestamp not found." << std::endl;
            return -1;
        }
    }
    else
    {
        std::cout << "MNode: Queried key does not exist." << std::endl;
        return -1;
    }

    lock.unlock();
}
