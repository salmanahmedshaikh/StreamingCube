//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include <boost/thread.hpp>


//static const std::string SUM = "sum";
//static const std::string AVG = "average";
//static const std::string COUNT = "count";
//static const std::string MIN = "min";
//static const std::string MAX = "max";

class MaterializedNode
{
    public:
        MaterializedNode(int nodeID, int level, int ordinal, int nodeSizeInNumOfRows, std::vector<std::string>& dimensions);
        MaterializedNode();
        ~MaterializedNode();

        int getNodeID();
        int getNodeLevel();
        int getNodeOrdinal();
        int getNodeSizeInRows();

        void getNodeDimensions(std::vector<DimensionKeyName>& vertexDimensions);
        void insertKeyTimestampValue(std::vector<DimensionKey> keys, DocumentId elementID, Timestamp timestamp, double value, Mark mark);
        double getValueAtTimestamp(std::vector<DimensionKey> keys, Timestamp timestamp);
        void getAggregatedValueAllKeys(std::string aggregateFunction, std::map<std::vector<DimensionKey>, double >& keyValues);
        void getAggregatedValueAllKeysWTimeGrain(std::string aggregateFunction, std::string OLAPTimeGrain, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& keyTimeValueMap);

    protected:

    private:
        int nodeID;
        int latticeLevel;
        int latticeOrdinal;
        int nodeSizeInRows;
        std::vector<std::string> dimAttributeNames;
        boost::mutex m_oMutex;

        std::map<std::vector<DimensionKey>, std::map<Timestamp, double > > keyTimestampValueMap;
        std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator keyTimestampValueMapIt;
        std::map<DocumentId, Timestamp> elementIDTimestampMap;
        std::map<DocumentId, Timestamp>::iterator elementIDTimestampMapIt;

        double getSUM(std::map<Timestamp, double >& innerMap);
        double getMIN(std::map<Timestamp, double >& innerMap);
        double getMAX(std::map<Timestamp, double >& innerMap);
        void manageElementWithMinusMark(std::vector<DimensionKey> keys, double value);
        double getAggregatedValue(std::vector<DimensionKey> keys, std::string aggregateFunction);
        Timestamp getOLAPQueriedTimestamp(std::string OLAPTimeGrain, Timestamp tupleTimestamp);
        void getAggregatedValueWTimeGrain(std::string aggregateFunction, std::string OLAPTimeGrain, std::map<Timestamp, double>& timestampValueMap, std::map<Timestamp, double>& OLAPTimestampValueMap);
};
