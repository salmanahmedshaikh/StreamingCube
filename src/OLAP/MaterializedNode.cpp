//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../OLAP/MaterializedNode.h"

MaterializedNode::MaterializedNode(int nodeID, int level, int ordinal, int nodeSizeInNumOfRows, std::vector<std::string>& dimensionNames)
{
    this->nodeID = nodeID;
    this->latticeLevel = level;
    this->latticeOrdinal = ordinal;
    this->dimAttributeNames = dimensionNames;
    this->nodeSizeInRows = nodeSizeInNumOfRows;
}

MaterializedNode::~MaterializedNode()
{
    //dtor
}

int MaterializedNode::getNodeID()
{
    return nodeID;
}

int MaterializedNode::getNodeLevel()
{
    return latticeLevel;
}

int MaterializedNode::getNodeOrdinal()
{
    return latticeOrdinal;
}

int MaterializedNode::getNodeSizeInRows()
{
    return nodeSizeInRows;
}

void MaterializedNode::getNodeDimensions(std::vector<DimensionKeyName>& nodeDimensions)
{
    nodeDimensions = this->dimAttributeNames;
}

Timestamp MaterializedNode::getOLAPQueriedTimestamp(std::string OLAPTimeGrain, Timestamp tupleTimestamp)
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

// Returns aggregate of all the values for all the keys in this node, w.r.t. time grain, for the given aggregate function
void MaterializedNode::getAggregatedValueAllKeysWTimeGrain(std::string aggregateFunction, std::string OLAPTimeGrain, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap)
{
    boost::mutex::scoped_lock lock(m_oMutex);

    // Aggregating w.r.t. all key combinations and time grain required
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
}

void MaterializedNode::getAggregatedValueWTimeGrain(std::string aggregateFunction, std::string OLAPTimeGrain, std::map<Timestamp, double>& timestampValueMap, std::map<Timestamp, double>& OLAPTimestampValueMap)
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

// Returns aggregate of all the values for all the keys in this node, w.r.t. whole window, for the given aggregate function
void MaterializedNode::getAggregatedValueAllKeys(std::string aggregateFunction, std::map<std::vector<DimensionKey>, double >& keyValues)
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

double MaterializedNode::getAggregatedValue(std::vector<DimensionKey> keys, std::string aggregateFunction)
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

double MaterializedNode::getSUM(std::map<Timestamp, double >& innerMap)
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

double MaterializedNode::getMIN(std::map<Timestamp, double >& innerMap)
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

double MaterializedNode::getMAX(std::map<Timestamp, double >& innerMap)
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

double MaterializedNode::getValueAtTimestamp(std::vector<DimensionKey> keys, Timestamp timestamp)
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

void MaterializedNode::manageElementWithMinusMark(std::vector<DimensionKey> keys, double value)
{
    //boost::mutex::scoped_lock lock(m_oMutex);
    if(keyTimestampValueMap.size() > 0)
    {
        // if keys found
        if((keyTimestampValueMapIt = keyTimestampValueMap.find(keys))!= keyTimestampValueMap.end())
        {
            // timestamp is ever increasing, so the oldest timestamp is at the peek
            Timestamp oldestTimestampInMap = keyTimestampValueMap[keys].begin()->first;
            //std::cout << "oldestTimestampInMap " << oldestTimestampInMap << std::endl;
            //std::cout << "latestTimestampInMap " << timestamp << std::endl;
            //std::cout << "Value before negation " << keyTimestampValueMap[keys][oldestTimestampInMap] << std::endl;
            // subtract value from the oldest timestamp
            keyTimestampValueMap[keys][oldestTimestampInMap] -= value;
            //std::cout << "Value after negation " << keyTimestampValueMap[keys][oldestTimestampInMap] << std::endl;

            // If the value of some timestamp becomes 0 after subtraction then delete the tuple
            if(keyTimestampValueMap[keys][oldestTimestampInMap] <= 0)
            {
                // If the inner map has only one entry then delete the outer map entry as well
                size_t innerMapSize = keyTimestampValueMap[keys].size();
                if(innerMapSize == 1)
                {
                    //std::cout << "Deleting the last entry of timestamp " << oldestTimestampInMap << std::endl;
                    //std::cout << "InnerMap size before deletion " << innerMapSize << std::endl;
                    keyTimestampValueMap.erase(keyTimestampValueMapIt);
                    //std::cout << "InnerMap size after deletion " << innerMapSize << std::endl;
                }
                else if(innerMapSize > 1) // If the inner map has more than one timestamp entries then delete the entry with the oldest timestamp
                {
                    std::map<Timestamp, double >::iterator innerIt = keyTimestampValueMapIt->second.find(oldestTimestampInMap);

                    if(innerIt != keyTimestampValueMapIt->second.end())
                    {
                        //std::cout << "InnerMap size before deletion " << innerMapSize << std::endl;
                        //for(std::map<Timestamp, double >::iterator innerIter = keyTimestampValueMapIt->second.begin(); innerIter != keyTimestampValueMapIt->second.end(); innerIter++)
                            //std::cout << innerIter->first << std::endl;

                        keyTimestampValueMapIt->second.erase(innerIt);

                        //std::cout << "InnerMap size after deletion " << keyTimestampValueMap[keys].size() << std::endl;
                        //for(std::map<Timestamp, double >::iterator innerIter = keyTimestampValueMapIt->second.begin(); innerIter != keyTimestampValueMapIt->second.end(); innerIter++)
                            //std::cout << innerIter->first << std::endl;

                        //sleep(1);
                    }
                    else
                    {
                        assert(false);
                        std::cout << "MNode: Inner Key with oldest timestamp not found." << std::endl;
                        exit(0);
                    }
                }
                else
                {
                    assert(false);
                    std::cout << "MNode: Key with oldest timestamp not found." << std::endl;
                    exit(0);
                }

            }
        }
        else
        {
            assert(false);
            std::cout << "MNode: Key not found." << std::endl;
            exit(0);
        }
        //std::cout << "keyTimestampValueMap[" << keys[0] << "][" << timestamp<<"]" << keyTimestampValueMap[keys][timestamp] << std::endl;
    }
    else
    {
        assert(false);
        std::cout << "MNode: Cannot delete as keyTimestampValueMap is empty." << std::endl;
        exit(0);
    }
    //lock.unlock();
}

void MaterializedNode::insertKeyTimestampValue(std::vector<DimensionKey> keys, DocumentId elementID, Timestamp timestamp, double value, Mark mark)
{
    boost::mutex::scoped_lock lock(m_oMutex);

    // Mapping elementID to timestamp
    if(mark == PLUS_MARK)
    {
        elementIDTimestampMap.insert(std::make_pair(elementID, timestamp));
    }
    else if(mark == MINUS_MARK)
    {
        elementIDTimestampMapIt = elementIDTimestampMap.find(elementID);
        timestamp = elementIDTimestampMapIt->second;

        if (elementIDTimestampMapIt != elementIDTimestampMap.end()) // if elementID found in map
        {
            elementIDTimestampMap.erase(elementIDTimestampMapIt);
        }
        else
        {
            /*
            keyTimestampValueMapIt = keyTimestampValueMap.find(keys);
            if(keyTimestampValueMapIt != keyTimestampValueMap.end())
            {
                for(std::map<Timestamp, double >::iterator innerIter = keyTimestampValueMapIt->second.begin(); innerIter != keyTimestampValueMapIt->second.end(); innerIter++)
                    std::cout << innerIter->first << std::endl;
            }

            assert(false);
            std::cout << "MNode: ElementID: " << elementID << ", not found in elementIDTimestampMap." << std::endl;
            exit(0);
            */
            return;
        }

    }

    // For the sake of testing only
    //std::cout << "Number of active ElementIDs " << elementIDTimestampMap.size() << std::endl;
    //for(elementIDTimestampMapIt = elementIDTimestampMap.begin(); elementIDTimestampMapIt != elementIDTimestampMap.end(); elementIDTimestampMapIt++)
    //{
    //    std::cout << elementIDTimestampMapIt->first << std::endl;
    //}
    // ~ Mapping elementID to timestamp

    // If keys found in the keyTimestampValueMap
    if((keyTimestampValueMapIt = keyTimestampValueMap.find(keys)) != keyTimestampValueMap.end())
    {
        // Managing MINUS_MARK
        if(mark == MINUS_MARK)
        {
            // timestamp is ever increasing, so the oldest timestamp is at the peek
            //Timestamp oldestTimestampInMap = keyTimestampValueMap[keys].begin()->first;

            keyTimestampValueMap[keys][timestamp] -= value;

            //std::cout << "oldestTimestampInMap " << oldestTimestampInMap << " | Value " << keyTimestampValueMap[keys][oldestTimestampInMap] << std::endl;
            //std::cout << "Value after deletion " << keyTimestampValueMap[keys][oldestTimestampInMap] << std::endl;

            // If the value of some timestamp becomes 0 after subtraction then delete the tuple
            if(keyTimestampValueMap[keys][timestamp] <= 0)
            {
                // If the inner map has only one entry then delete the outer map entry as well
                size_t innerMapSize = keyTimestampValueMap[keys].size();
                if(innerMapSize == 1)
                {
                    //std::cout << "Deleting the last entry of timestamp " << oldestTimestampInMap << std::endl;
                    //std::cout << "InnerMap size before deletion " << innerMapSize << std::endl;
                    keyTimestampValueMap.erase(keyTimestampValueMapIt);
                    //std::cout << "InnerMap size after deletion " << innerMapSize << std::endl;
                }
                else if(innerMapSize > 1) // If the inner map has more than one timestamp entries then delete the entry with the oldest timestamp
                {
                    std::map<Timestamp, double >::iterator innerIt = keyTimestampValueMapIt->second.find(timestamp);

                    // If entry with oldest timestamp found, and must be found
                    if(innerIt != keyTimestampValueMapIt->second.end())
                    {
                        //std::cout << "InnerMap size before deletion " << innerMapSize << std::endl;
                        //for(std::map<Timestamp, double >::iterator innerIter = keyTimestampValueMapIt->second.begin(); innerIter != keyTimestampValueMapIt->second.end(); innerIter++)
                            //std::cout << innerIter->first << std::endl;
                        keyTimestampValueMapIt->second.erase(innerIt);
                    }
                    else
                    {
                        assert(false);
                        std::cout << "MNode: Inner Key with oldest timestamp not found." << std::endl;
                        exit(0);
                    }
                }
                else
                {
                    assert(false);
                    std::cout << "MNode: Key with oldest timestamp not found." << std::endl;
                    exit(0);
                }
            }

            return;
        }


        std::map<Timestamp, double > timestampValueMap;
        std::map<Timestamp, double >::iterator timestampValueMapIt;

        timestampValueMap = keyTimestampValueMapIt->second;

        // If timestamp also found when there are more than one values for a key per timestamp in case of PLUS_MARK
        if((timestampValueMapIt = timestampValueMap.find(timestamp)) != timestampValueMap.end())
        {
            //if(mark == PLUS_MARK)
            //{
            keyTimestampValueMap[keys][timestamp] += value;
                //std::cout << "keyTimestampValueMap[" << keys[0] << "][" << timestamp<<"]" << keyTimestampValueMap[keys][timestamp] << std::endl;
            //}
            /*
            else if(mark == MINUS_MARK)
            {
                //manageElementWithMinusMark(keys, value);
                //keyTimestampValueMap[keys][timestamp] -= value;
            }
            else
            {
                assert(false);
                std::cout << "Mark can be either +/-" << std::endl;
                exit(0);
            }
            */
        }
        else // If timestamp not found but key found then add/insert the value
        {
            //if(mark == PLUS_MARK)
            //{
            keyTimestampValueMap[keys][timestamp] = value;
                //std::cout << "keyTimestampValueMap[" << keys[0] << "][" << timestamp<<"]" << keyTimestampValueMap[keys][timestamp] << std::endl;
            //}
            /*
            else if(mark == MINUS_MARK) // If timestamp not found even then we need to delete given keys with oldest timestamp
            {
                //manageElementWithMinusMark(keys, value);
                //std::cout << "MNode: Initial value can not be -ve" << std::endl;
                //assert(false);
                //exit(0);
            }
            else
            {
                std::cout << "MNode: Mark can be either +/-" << std::endl;
                assert(false);
                exit(0);
            }
            */
        }
    }
    else // Keys not found; insert it
    {
        keyTimestampValueMap[keys][timestamp] = value;
        //std::cout << "keyTimestampValueMap[keys][timestamp] " << keyTimestampValueMap[keys][timestamp] << std::endl;
    }
    lock.unlock();
}
