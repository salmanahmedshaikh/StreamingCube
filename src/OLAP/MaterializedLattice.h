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
#include "../OLAP/MaterializedNodeWithRelation.h"
#include "../OLAP/OLAPManager.h"
#include "../Common/Types.h"

#include <boost/thread.hpp>

class MaterializedLattice
{
    public:
        MaterializedLattice(std::map<vertexInfo, std::vector<std::string> >& latVertices);
        MaterializedLattice();
        virtual ~MaterializedLattice();

        void materializeSelectedNodes();

        bool mapMVertex(vertexID, boost::shared_ptr<MaterializedNode>);
        bool mapMVertexWRelation(vertexID, boost::shared_ptr<MaterializedNodeWithRelation>);

        boost::shared_ptr<MaterializedNode> getMVertexByID(vertexID VID);
        boost::shared_ptr<MaterializedNodeWithRelation> getMVertexWRelationByID(vertexID VID);

        bool insertIntoLatticeNodes(std::vector<DimensionKeyName> allDimKeyNames, std::vector<DimensionKey> allDimKeys, std::vector<DimensionKeyValueName> allDimKeyValuesNames, std::vector<DimensionKeyValue> allDimKeyValues, Timestamp timestamp, double value, DocumentId elementID, Mark mark, Timestamp IoA, std::vector<Element>& integratedPlusMinusElements);

        void getAllAggregatedValuesForANode(std::vector<DimensionKeyName> queryDimensions, std::string aggregateFunction, std::string OLAPTimeGrain, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap);
        int getAllAggregatedValuesForANodeWRelation(std::vector<DimensionKeyName> queryDimensions, std::string aggregateFunction, std::string OLAPTimeGrain, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap);

        void getAllAggregatedValuesForANode(std::vector<DimensionKeyName> queryDimensions, std::string aggregateFunction, std::map<std::vector<DimensionKey>, double>& keyValues);

        void getMVerticesMap(std::map<vertexID,  boost::shared_ptr<MaterializedNode> >& materializedVerticesMap);
        void getMVerticesWRelationMap(std::map<vertexID,  boost::shared_ptr<MaterializedNodeWithRelation> >& materializedVerticesMap);

        void getDrillDownNodes(std::vector<DimensionKeyName> queryDimensions, std::map<int, std::vector<DimensionKeyName> >& drillDownNodes);
        void getRollUpNodes(std::vector<DimensionKeyName> queryDimensions, std::map<int, std::vector<DimensionKeyName> >& rollUpNodes);
        void getAllLatticeNodes(std::map<int, std::vector<DimensionKeyName> >& latticeNodes);
        void getLatticeNodeByID(int nodeID, std::vector<DimensionKeyName>& queryDimensions);
        int getNodeIDByDimensionNames(std::vector<DimensionKeyName>& nodeDimensions);
        void getAllLatticeNodesWithInfo(std::map<vertexInfo, std::vector<DimensionKeyName> >& allLatticeNodesWithInfo);
        DimensionKeyValue getDimValue(DimensionKeyName dimKeyName, DimensionKey dimKey);
        int getQueryAnsweringNodeID();

    protected:

    private:
        int queryAnsweringNodeID;
        boost::mutex m_oMutex;

        std::vector<std::pair<DocumentId, Timestamp> > rowIdTimestampVector; // for mapping documentIds to timestamps; to help in delete rows
        std::vector<std::pair<DocumentId, Timestamp> >::iterator rowIdTimestampVectorIt;

        std::map<DimensionKeyName, std::map<DimensionKey, DimensionKeyValue> > dimNameKeyValueMap;
        std::map<DimensionKeyName, std::map<DimensionKey, DimensionKeyValue> >::iterator dimNameKeyValueMapIt;

        std::map<vertexID,  boost::shared_ptr<MaterializedNode> > mNodesMap;
        std::map<vertexID,  boost::shared_ptr<MaterializedNode> >::iterator mNodesMapIt;

        std::map<vertexID,  boost::shared_ptr<MaterializedNodeWithRelation> > mNodesWRelationMap;
        std::map<vertexID,  boost::shared_ptr<MaterializedNodeWithRelation> >::iterator mNodesWRelationMapIt;

        std::map<vertexInfo, std::vector<DimensionKeyName> > latticeNodes;
        std::map<vertexInfo, std::vector<DimensionKeyName> >::iterator latticeNodesIt;

        std::map<int, std::vector<DimensionKeyName> > latticeNodesIDMap;
        std::map<int, std::vector<DimensionKeyName> >::iterator latticeNodesIDMapIt;

        std::map<Timestamp, std::vector<DocumentId> > timestampDocIDMap;
        std::map<Timestamp, std::vector<DocumentId> >::iterator timestampDocIDMapIt;

        std::vector< std::vector<std::string> > latticeOutputVertices;
        std::vector<int> latticeOutputVerticesID;
        std::vector<int> latticeOutputVerticesAnsweringID; // The actual nodes which can answer the query
        std::vector<int>::iterator latticeOutputVerticesIDIt;
        std::vector< std::vector<std::string> >::iterator latticeOutputVerticesIt;

        // For testing purpose only
        std::vector<DocumentId> elementIDVec;

        //std::vector<std::pair<DocumentId, Timestamp> >::iterator getItThruDocumentID(DocumentId id);

        //int makeQueryKey(std::vector<std::string>& queryVertex, std::vector<std::string>& mVertex, std::vector<unsigned int>& currentKeys, unsigned int keyToInsert, std::vector<unsigned int>& newKeys);
        bool containAllDimensions(std::vector<std::string>& queryVertex, std::vector<std::string>& mVertex);
        bool containAnyDimension(std::vector<std::string>& queryNode, std::vector<std::string>& latticeNode);
        void getQueryDimensionIndices(std::vector<DimensionKeyName> queryDimensions, std::vector<DimensionKeyName> mNodeDimensions, std::vector<int>& queryDimensionIndices);
        void findMissingDimensions(std::vector<std::string>& queryVertex, std::vector<std::string>& mVertex, std::vector<std::string>& missingDim, std::vector<int>& missingDimIndices);

        void generateAggregateFromMNode(std::vector<int> queryDimensionIndices, std::string aggregateFunction, std::map<std::vector<DimensionKey>, double>& mNodeKeyValues, std::map<std::vector<DimensionKey>, double>& keyValues);
        void generateAggregateFromMNode(std::vector<int> queryDimensionIndices, std::string aggregateFunction, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& mNodeKeyTimeValueMap, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap);
        void generateAggregateFromMNode(std::vector<int> queryDimensionIndices, std::string aggregateFunction, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& mNodeKeyTimeValueMap, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap);

        bool mapDimNameKeyValue(std::vector<DimensionKeyName>& allDimKeyNames, std::vector<DimensionKey>& allDimKeys, std::vector<DimensionKeyValueName>& allDimKeyValuesNames, std::vector<DimensionKeyValue>& allDimKeyValues);
};
