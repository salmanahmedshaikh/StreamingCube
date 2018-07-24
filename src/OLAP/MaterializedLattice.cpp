//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../OLAP/MaterializedLattice.h"
#include <algorithm>

MaterializedLattice::MaterializedLattice(std::map<vertexInfo, std::vector<std::string> >& latNodes)
{
    this->latticeNodes = latNodes;
    this->queryAnsweringNodeID = -1;

    int queryID = 0;
    OLAPManager::getInstance()->getConfigValue(queryID, "LatticeOutputVertices", latticeOutputVertices);
}

MaterializedLattice::MaterializedLattice()
{
    this->queryAnsweringNodeID = -1;
}

MaterializedLattice::~MaterializedLattice()
{
    //dtor
}

int MaterializedLattice::getQueryAnsweringNodeID()
{
    return this->queryAnsweringNodeID;
}

void MaterializedLattice::materializeSelectedNodes(void)
{
    // Materializing selected vertices
    // std::cout << "From materializedLattice.cpp " << std::endl;
    for(latticeNodesIt = latticeNodes.begin(); latticeNodesIt != latticeNodes.end(); latticeNodesIt++)
    {
        // Populate latticeNodesIDMap with ID and dimensions
        latticeNodesIDMap.insert(std::make_pair(latticeNodesIt->first.vertexID, latticeNodesIt->second));

        if(latticeNodesIt->first.isMaterialized)
        {
            vertexID VID;

            VID.level = latticeNodesIt->first.level;
            VID.ordinal = latticeNodesIt->first.ordinal;

            //boost::shared_ptr<MaterializedNode> mNode (new MaterializedNode(latticeNodesIt->first.vertexID, latticeNodesIt->first.level, latticeNodesIt->first.ordinal, latticeNodesIt->first.vRows, latticeNodesIt->second));
            boost::shared_ptr<MaterializedNodeWithRelation> mNodeWRelation (new MaterializedNodeWithRelation(latticeNodesIt->first.vertexID, latticeNodesIt->first.level, latticeNodesIt->first.ordinal, latticeNodesIt->first.vRows, latticeNodesIt->second));

            //mapMVertex(VID, mNode);
            mapMVertexWRelation(VID, mNodeWRelation);

            //Printing all MVertices
            //std::vector<std::string> dimensions;
            //std::vector<std::string>::iterator dimensionsIt;

            //dimensions = latticeNodesIt->second;

            //std::cout << std::endl << "mNode ID " << latticeNodesIt->first.vertexID << ": "  << latticeNodesIt->first.level << ", " << latticeNodesIt->first.ordinal << std::endl;
            //for(dimensionsIt = dimensions.begin(); dimensionsIt != dimensions.end(); dimensionsIt++)
                //std::cout << *dimensionsIt << std::endl;
        }

    //Printing all MVertices
    //std::vector<std::string> dimensions;
    //std::vector<std::string>::iterator dimensionsIt;

    //dimensions = latticeNodesIt->second;

    //std::cout << std::endl << "Vertex ID " << latticeNodesIt->first.vertexID << ": "  << latticeNodesIt->first.level << ", " << latticeNodesIt->first.ordinal << std::endl;
    //if(latticeNodesIt->first.isMaterialized)
        //std::cout << "M VERTEX:                 ";
    //for(dimensionsIt = dimensions.begin(); dimensionsIt != dimensions.end(); dimensionsIt++)
        //std::cout << *dimensionsIt << ", ";
    }

    // Populating latticeOutputVerticesID
    for(latticeOutputVerticesIt = latticeOutputVertices.begin(); latticeOutputVerticesIt != latticeOutputVertices.end(); latticeOutputVerticesIt++)
    {
        std::vector<std::string> vertexDims = *latticeOutputVerticesIt;
        latticeOutputVerticesID.push_back(getNodeIDByDimensionNames(vertexDims));
        //std::cout << "OLAP lattice queried nodes " << getNodeIDByDimensionNames(vertexDims) << std::endl;
    }

    // Finding the Lattice Vertices which can answer requested lattice output vertices
    // Sorting the nodes according to their size
    std::map<int,  boost::shared_ptr<MaterializedNodeWithRelation> > mNodeWNumRowsAsKeyMap;
    std::map<int,  boost::shared_ptr<MaterializedNodeWithRelation> >::iterator mNodeWNumRowsAsKeyMapIt;
    for(mNodesWRelationMapIt = mNodesWRelationMap.begin(); mNodesWRelationMapIt != mNodesWRelationMap.end(); mNodesWRelationMapIt++)
    {
        mNodeWNumRowsAsKeyMap.insert(std::make_pair( (mNodesWRelationMapIt->second)->getNodeSizeInRows(), mNodesWRelationMapIt->second));
    }

    //Looping through the latticeOutput nodes
    for(latticeOutputVerticesIt = latticeOutputVertices.begin(); latticeOutputVerticesIt != latticeOutputVertices.end(); latticeOutputVerticesIt++)
    {
        std::vector<std::string> vertexDims = *latticeOutputVerticesIt;
        // Loop from nodes with smaller number of rows to higher number of rows
        for(mNodeWNumRowsAsKeyMapIt = mNodeWNumRowsAsKeyMap.begin(); mNodeWNumRowsAsKeyMapIt != mNodeWNumRowsAsKeyMap.end(); mNodeWNumRowsAsKeyMapIt++)
        {
            boost::shared_ptr<MaterializedNodeWithRelation> mNodeWRelation = mNodeWNumRowsAsKeyMapIt->second;

            std::vector<DimensionKeyName> mNodeDimensions;
            std::vector<DimensionKeyName>::iterator mNodeDimensionsIt;

            mNodeWRelation->getNodeDimensions(mNodeDimensions);

            // If query belongs to the same or lower level than mNode, only then it can be answered through mNode
            if(mNodeDimensions.size() >= vertexDims.size() && containAllDimensions(vertexDims, mNodeDimensions))
            {
                latticeOutputVerticesAnsweringID.push_back(mNodeWRelation->getNodeID());
                //std::cout << "OLAP lattice answering nodes " << mNodeWRelation->getNodeID() << std::endl;
                break;
            }
        }
    }
}

bool MaterializedLattice::mapMVertex(vertexID VID, boost::shared_ptr<MaterializedNode> mNode)
{
    // If VID not found
    if((mNodesMapIt = mNodesMap.find(VID)) == mNodesMap.end())
    {
        this->mNodesMap.insert(std::make_pair(VID, mNode));
        return true;
    }
    else
    {
        assert(false);
        return false;
    }
}

bool MaterializedLattice::mapMVertexWRelation(vertexID VID, boost::shared_ptr<MaterializedNodeWithRelation> mNode)
{
    // If VID not found
    if((mNodesWRelationMapIt = mNodesWRelationMap.find(VID)) == mNodesWRelationMap.end())
    {
        this->mNodesWRelationMap.insert(std::make_pair(VID, mNode));
        return true;
    }
    else
    {
        assert(false);
        return false;
    }
}

boost::shared_ptr<MaterializedNode> MaterializedLattice::getMVertexByID(vertexID VID)
{
	mNodesMapIt = mNodesMap.find(VID);
	assert(mNodesMapIt!=mNodesMap.end());

	return mNodesMapIt->second;
}

boost::shared_ptr<MaterializedNodeWithRelation> MaterializedLattice::getMVertexWRelationByID(vertexID VID)
{
	mNodesWRelationMapIt = mNodesWRelationMap.find(VID);
	assert(mNodesWRelationMapIt!=mNodesWRelationMap.end());

	return mNodesWRelationMapIt->second;
}

bool MaterializedLattice::mapDimNameKeyValue(std::vector<DimensionKeyName>& allDimKeyNames, std::vector<DimensionKey>& allDimKeys, std::vector<DimensionKeyValueName>& allDimKeyValuesNames, std::vector<DimensionKeyValue>& allDimKeyValues)
{
    boost::mutex::scoped_lock lock(m_oMutex);

    for(int i = 0; i < allDimKeyValues.size(); i++)
    {
        dimNameKeyValueMapIt = dimNameKeyValueMap.find(allDimKeyNames[i]);
        if(dimNameKeyValueMapIt == dimNameKeyValueMap.end()) // If the DimKeyName does not exist in the map, only then insert it
        {
            dimNameKeyValueMap[allDimKeyNames[i]][allDimKeys[i]] = allDimKeyValues[i];
            // Changed temporarily
            //dimNameKeyValueMap[allDimKeyValuesNames[i]][allDimKeys[i]] = allDimKeyValues[i];
        }
        else // If the DimKeyName exists in the map
        {
            std::map<DimensionKey, DimensionKeyValue>& dimKeyValueMap = dimNameKeyValueMapIt->second;
            std::map<DimensionKey, DimensionKeyValue>::iterator dimKeyValueMapIt;

            //dimKeyValueMap = dimNameKeyValueMapIt->second;
            dimKeyValueMapIt = dimKeyValueMap.find(allDimKeys[i]);

            if(dimKeyValueMapIt == dimKeyValueMap.end()) // If the key value does not already exist, insert it
            {
                dimKeyValueMap.insert(std::make_pair(allDimKeys[i], allDimKeyValues[i]));
            }
        }
    }
    //std::cout << allDimKeyNames[0] << allDimKeys[0] << allDimKeyValuesNames[0] << allDimKeyValues[0] << std::endl;
    lock.unlock();
}

DimensionKeyValue MaterializedLattice::getDimValue(DimensionKeyName dimKeyName, DimensionKey dimKey)
{
    boost::mutex::scoped_lock lock(m_oMutex);

    dimNameKeyValueMapIt = dimNameKeyValueMap.find(dimKeyName);
    if(dimNameKeyValueMapIt != dimNameKeyValueMap.end()) // If the DimKeyName exists in the map
    {
        std::map<DimensionKey, DimensionKeyValue>& dimKeyValueMap = dimNameKeyValueMapIt->second;
        std::map<DimensionKey, DimensionKeyValue>::iterator dimKeyValueMapIt;

        dimKeyValueMapIt = dimKeyValueMap.find(dimKey);

        if(dimKeyValueMapIt != dimKeyValueMap.end()) // If the key value exist, return it
        {
            return dimKeyValueMapIt->second;
        }
        else
        {
            std::cout<<"MLattice: Dimension key value does not exist in dimNameKeyValueMap." << dimKeyName << ", " << dimKey << std::endl;
            assert(false);
            exit(0);
        }
    }
    else
    {
        std::cout<<"MLattice: Dimension does not exist in dimNameKeyValueMap." << dimKeyName << ", " << dimKey << std::endl;
        assert(false);
        exit(0);
    }

    lock.unlock();
}

/*
std::vector<std::pair<DocumentId, Timestamp> >::iterator MaterializedLattice::getItThruDocumentID(DocumentId id)
{
    int i = 0;
    for(rowIdTimestampVectorIt = rowIdTimestampVector.begin(); rowIdTimestampVectorIt != rowIdTimestampVector.end(); rowIdTimestampVectorIt++)
    {
        //std::cout << "? " << i++ << std::endl;
        if(rowIdTimestampVectorIt->first == id)
        {
            return rowIdTimestampVectorIt;
        }
    }
    return rowIdTimestampVector.end();
}
*/

bool MaterializedLattice::insertIntoLatticeNodes(std::vector<DimensionKeyName> allDimKeyNames, std::vector<DimensionKey> allDimKeys, std::vector<DimensionKeyValueName> allDimKeyValuesNames, std::vector<DimensionKeyValue> allDimKeyValues, Timestamp timestamp, double value, DocumentId elementID, Mark mark, Timestamp IoA, std::vector<Element>& integratedPlusMinusElements)
{
    mapDimNameKeyValue(allDimKeyNames, allDimKeys, allDimKeyValuesNames, allDimKeyValues);

    /*
    if(mark == PLUS_MARK)
    {
        // inserting into rowIdTimestampVector
        this->rowIdTimestampVector.push_back(make_pair(elementID, timestamp) );
    }
    else if(mark == MINUS_MARK)
    {
        boost::mutex::scoped_lock lock(m_oMutex);

        std::vector<std::pair<DocumentId, Timestamp> >::iterator it = getItThruDocumentID(elementID);

        if(it != rowIdTimestampVector.end()) // if tuple with given id found
        {
            timestamp = it->second;
            rowIdTimestampVector.erase(it);
        }
        else
        {
            std::cout << "No tuple with given ID found" << std::endl;
            exit(0);
        }

        lock.unlock();
    }
    */

    // Insert into all the materialized nodes with std::maps
    /*
    for(mNodesMapIt = mNodesMap.begin(); mNodesMapIt != mNodesMap.end(); mNodesMapIt++)
    {
        // Insert into all the materialized nodes
        boost::shared_ptr<MaterializedNode> mNode = mNodesMapIt->second;

        std::vector<std::string> nodeDimensions;
        mNode->getNodeDimensions(nodeDimensions);

        std::vector<unsigned int> keysToInsert;
        std::string keysStr = "";
        std::stringstream keysSS;

        // Iterate through all the incoming dimension strings (from the raw data) and existing vertexDimensions
        for(int j = 0; j < nodeDimensions.size(); j++)
        {
            for(int i = 0; i < allDimKeyNames.size(); i++)
                if(nodeDimensions[j] == allDimKeyNames[i])
                {
                    keysToInsert.push_back(allDimKeys[i]); // Push_back inserts at the end of the list
                    //std::cout << "vertexDimensions " << vertexDimensions[j] << std::endl;
                    //std::cout << "keysToInsert " << dimensions[i] << " :" << keys[i] << std::endl;
                    keysStr += allDimKeyNames[j];
                    keysSS << allDimKeys[i];
                }
        }

        //std::cout << "keysStr: " << keysSS.str() << ", Value: " << value << ", Timestamp: " << timestamp << ", Mark: " << mark << std::endl;
        mNode->insertKeyTimestampValue(keysToInsert, elementID, timestamp, value, mark);
    }
    */


    bool generateElements = true;

    if(mark == PLUS_MARK) // Only adding tuples with PLUS_MARK while ignoring tuples with MINUS_MARK
    {
        // Insert into all the materialized nodes with relations
        for(mNodesWRelationMapIt = mNodesWRelationMap.begin(); mNodesWRelationMapIt != mNodesWRelationMap.end(); mNodesWRelationMapIt++)
        {
            // Insert into all the materialized nodes
            boost::shared_ptr<MaterializedNodeWithRelation> mNodeWRelation = mNodesWRelationMapIt->second;

            std::vector<std::string> nodeDimensions;
            mNodeWRelation->getNodeDimensions(nodeDimensions);
            int nodeID = mNodeWRelation->getNodeID();
            //std::cout << "NodeID " << nodeID << std::endl;

            std::vector<unsigned int> keysToInsert;
            std::string keysStr = "";
            std::stringstream keysSS;

            // Iterate through all the incoming dimension strings (from the raw data) and existing vertexDimensions
            for(int j = 0; j < nodeDimensions.size(); j++)
            {
                for(int i = 0; i < allDimKeyNames.size(); i++)
                    if(nodeDimensions[j] == allDimKeyNames[i])
                    {
                        keysToInsert.push_back(allDimKeys[i]); // Push_back inserts at the end of the list
                        //std::cout << "vertexDimensions " << vertexDimensions[j] << std::endl;
                        //std::cout << "keysToInsert " << dimensions[i] << " :" << keys[i] << std::endl;
                        keysStr += allDimKeyNames[j];
                        keysSS << allDimKeys[i];
                    }
            }


            //if(latticeOutputVerticesID)
            //latticeOutputVertices
            //generateElements = true;
            generateElements = false;
            int i = 0;
            //std::vector<int> generateElementsFor;


            std::vector< std::vector<DimensionKeyName> > generateElementsFor;

            for(latticeOutputVerticesIDIt = latticeOutputVerticesAnsweringID.begin(); latticeOutputVerticesIDIt != latticeOutputVerticesAnsweringID.end(); latticeOutputVerticesIDIt++)
            {
                //std::cout << "latticeOutputVerticesAnsweringID " << *latticeOutputVerticesIDIt << std::endl;
                //std::cout << "NodeID " << nodeID << std::endl;

                if(*latticeOutputVerticesIDIt == nodeID)
                {
                    std::vector<DimensionKeyName> nodeDimNames;
                    getLatticeNodeByID(latticeOutputVerticesID[i], nodeDimNames);
                    generateElementsFor.push_back(nodeDimNames);

                    //generateElementsFor.push_back(latticeOutputVerticesID[i]); // A lattice may need to generate output for multiple queried vertices
                    //std::cout << "NodeID " << nodeID << " generateElements for " << latticeOutputVerticesID[i] << std::endl;
                    generateElements = true;
                    //break;
                }
                i++;
            }

            std::vector<Element> plusMinusElements;
            std::vector<Element>::iterator plusMinusElementsIt;

            mNodeWRelation->insertKeyTimestampValue(keysToInsert, timestamp, value, mark, IoA, plusMinusElements, generateElements, generateElementsFor);

            for(plusMinusElementsIt = plusMinusElements.begin(); plusMinusElementsIt != plusMinusElements.end(); plusMinusElementsIt++)
                integratedPlusMinusElements.push_back(*plusMinusElementsIt);
                //std::cout << *plusMinusElementsIt << std::endl;


            /*
            std::vector<int>::iterator it;
            i = 0;
            for(it = latticeOutputVerticesID.begin(); it != latticeOutputVerticesID.end(); it++)
            {
                //std::cout << "Queried Node " << latticeOutputVerticesID[i] << ", Answering Node " << latticeOutputVerticesAnsweringID[i] << std::endl;
                i++;
            }
            */


        }
    }
}

void MaterializedLattice::getDrillDownNodes(std::vector<DimensionKeyName> queryDimensions, std::map<int, std::vector<DimensionKeyName> >& drillDownNodes)
{
    for(latticeNodesIt = latticeNodes.begin(); latticeNodesIt != latticeNodes.end(); latticeNodesIt++)
    {
        std::vector<DimensionKeyName> latticeNodeDimensions = latticeNodesIt->second;
        // Once we drill down, we move from higher level of grain to lower level of grain
        if(latticeNodeDimensions.size() > queryDimensions.size() && containAllDimensions(queryDimensions, latticeNodeDimensions))
        {
            drillDownNodes.insert(std::make_pair(latticeNodesIt->first.vertexID ,latticeNodeDimensions));
        }
    }
}

void MaterializedLattice::getRollUpNodes(std::vector<DimensionKeyName> queryDimensions, std::map<int, std::vector<DimensionKeyName> >& rollUpNodes)
{
    for(latticeNodesIt = latticeNodes.begin(); latticeNodesIt != latticeNodes.end(); latticeNodesIt++)
    {
        std::vector<DimensionKeyName> latticeNodeDimensions = latticeNodesIt->second;
        // Once we roll up, we move from lower level grain to higher level grain
        if(latticeNodeDimensions.size() < queryDimensions.size() && containAnyDimension(queryDimensions, latticeNodeDimensions))
        {
            rollUpNodes.insert(std::make_pair(latticeNodesIt->first.vertexID ,latticeNodeDimensions));
        }
    }
}

void MaterializedLattice::getAllLatticeNodes(std::map<int, std::vector<DimensionKeyName> >& allLatticeNodes)
{
    for(latticeNodesIt = latticeNodes.begin(); latticeNodesIt != latticeNodes.end(); latticeNodesIt++)
    {
        std::vector<DimensionKeyName> latticeNodeDimensions = latticeNodesIt->second;
        allLatticeNodes.insert(std::make_pair(latticeNodesIt->first.vertexID ,latticeNodeDimensions));
    }
}

void MaterializedLattice::getAllLatticeNodesWithInfo(std::map<vertexInfo, std::vector<DimensionKeyName> >& allLatticeNodesWithInfo)
{
    allLatticeNodesWithInfo = latticeNodes;
}

// getLatticeNodeByID
void MaterializedLattice::getLatticeNodeByID(int nodeID, std::vector<DimensionKeyName>& queryDimensions)
{
    latticeNodesIDMapIt = latticeNodesIDMap.find(nodeID);

    if(latticeNodesIDMapIt != latticeNodesIDMap.end())
    {
        queryDimensions =  latticeNodesIDMapIt->second;
    }
    else
    {
        std::cout << "MLattice: Lattice node dimensions with given ID not found." << std::endl;
        exit(0);
    }
}

// get lattice nodeID by dimension names
int MaterializedLattice::getNodeIDByDimensionNames(std::vector<DimensionKeyName>& nodeDimensions)
{
    for(latticeNodesIDMapIt = latticeNodesIDMap.begin(); latticeNodesIDMapIt != latticeNodesIDMap.end(); latticeNodesIDMapIt++)
    {
        //std::cout << latticeNodesIDMapIt->first << std::endl;
        std::vector<DimensionKeyName> latticeNodeDimensions = latticeNodesIDMapIt->second;
        if(nodeDimensions.size() != latticeNodeDimensions.size()) // can get nodeID only both are equal and contain all dimensions
            continue;
        else if( containAllDimensions(nodeDimensions, latticeNodeDimensions) )
            return latticeNodesIDMapIt->first;
    }
    return -1;
}

// Returns aggregate of all the values for all the keys in a particular node, for the whole window, whose dimKeyNames are provided for the given aggregate function
void MaterializedLattice::getAllAggregatedValuesForANode(std::vector<DimensionKeyName> queryDimensions, std::string aggregateFunction, std::string OLAPTimeGrain, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap)
{
    /*
    boost::shared_ptr<MaterializedNode> selectedMNode;
    double mNodeWMinRows = DBL_MAX;

    //boost::mutex::scoped_lock lock(m_oMutex);
    // find the materialized node with the least number of rows to answer the query
    int loopCounter = 0;
    for(mNodesMapIt = mNodesMap.begin(); mNodesMapIt != mNodesMap.end(); ++mNodesMapIt)
    {
        std::cout << " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> loopCounter " << ++loopCounter << std::endl;

        boost::shared_ptr<MaterializedNode> mNode;
        std::vector<DimensionKeyName> mNodeDimensions;

        mNode = mNodesMapIt->second;
        mNode->getNodeDimensions(mNodeDimensions);

        // If query belongs to the same or lower level than mNode, only then it can be answered through mNode
        std::cout << "mNodeDimensions.size() | queryDimensions.size() " << mNodeDimensions.size() << " | " << queryDimensions.size() << " | mNodesMap.size(): " << mNodesMap.size() << std::endl;

        for(int i = 0; i < mNodeDimensions.size(); i++)
            std::cout << mNodeDimensions[i] << ", ";
        std::cout << endl;

        for(int i = 0; i < queryDimensions.size(); i++)
            std::cout << queryDimensions[i] << ", ";
        std::cout << endl;

        if(mNodeDimensions.size() >= queryDimensions.size() && containAllDimensions(queryDimensions, mNodeDimensions))
        {
            //std::cout << "Passing MNode " << mNode->getNodeID() << std::endl;
            int mNodeRows = mNode->getNodeSizeInRows();
            //std::cout << "mNodeRows: " << mNodeRows << " | " << "mNodeWMinRows:" << mNodeWMinRows << std::endl;
            if(mNodeRows <= mNodeWMinRows)
            {
                std::cout << "All test passing MNode " << mNode->getNodeID() << std::endl;
                mNodeWMinRows = mNodeRows;
                //selectedMNode = mNodesMapIt->second;
                selectedMNode = mNode;
                //std::cout << "Selected MNode is set with ID " << selectedMNode->getNodeID() << std::endl;
            }
        }
    }
    //lock.unlock();
    //std::cout << "Selected MNode ";
    //std::cout << selectedMNode->getNodeID() << std::endl;

    // Checking if node is assinged
    assert(mNodeWMinRows != DBL_MAX);
    if(mNodeWMinRows == DBL_MAX)
    {
        //std::cout << "queryDimensions.size() " << queryDimensions.size() << std::endl;
        std::cout << "At-least one node must be selected for querying." << std::endl;
        exit(0);
    }
    */

    std::map<int,  boost::shared_ptr<MaterializedNode> > mNodeWNumRowsAsKeyMap;
    std::map<int,  boost::shared_ptr<MaterializedNode> >::iterator mNodeWNumRowsAsKeyMapIt;
    for(mNodesMapIt = mNodesMap.begin(); mNodesMapIt != mNodesMap.end(); mNodesMapIt++)
    {
        mNodeWNumRowsAsKeyMap.insert(std::make_pair( (mNodesMapIt->second)->getNodeSizeInRows(), mNodesMapIt->second));
    }

    // For testing purpose only
    //for(mNodeWNumRowsAsKeyMapIt = mNodeWNumRowsAsKeyMap.begin(); mNodeWNumRowsAsKeyMapIt != mNodeWNumRowsAsKeyMap.end(); mNodeWNumRowsAsKeyMapIt++)
    //{
        //std::cout << "Num Rows: " << mNodeWNumRowsAsKeyMapIt->first << " Node ID: " << (mNodeWNumRowsAsKeyMapIt->second)->getNodeID() << std::endl;
    //}

    // Loop from nodes with smaller number of rows to higher number of rows
    for(mNodeWNumRowsAsKeyMapIt = mNodeWNumRowsAsKeyMap.begin(); mNodeWNumRowsAsKeyMapIt != mNodeWNumRowsAsKeyMap.end(); mNodeWNumRowsAsKeyMapIt++)
    {
        boost::shared_ptr<MaterializedNode> mNode = mNodeWNumRowsAsKeyMapIt->second;

        std::vector<DimensionKeyName> mNodeDimensions;
        std::vector<DimensionKeyName>::iterator mNodeDimensionsIt;

        //std::vector<DimensionKeyName> mNodeDimensions;
        //selectedMNode->getNodeDimensions(mNodeDimensions);
        mNode->getNodeDimensions(mNodeDimensions);

        // If query belongs to the same or lower level than mNode, only then it can be answered through mNode
        if(mNodeDimensions.size() >= queryDimensions.size() && containAllDimensions(queryDimensions, mNodeDimensions))
        {

            int levelDiff = mNodeDimensions.size() - queryDimensions.size();

            // Case when the mNode is available to answer the query directly (levelDiff = 0)
            if(levelDiff == 0)
            {
                //mNode->getAggregatedValueAllKeys(aggregateFunction, keyValues);
                mNode->getAggregatedValueAllKeysWTimeGrain(aggregateFunction, OLAPTimeGrain, OLAPKeyTimeValueMap);
                //selectedMNode->getAggregatedValueAllKeysWTimeGrain(aggregateFunction, OLAPTimeGrain, OLAPKeyTimeValueMap);
                std::cout << "Query Answered directly from Queried Node " << mNode->getNodeID() << std::endl;
                this->queryAnsweringNodeID = mNode->getNodeID();
                //std::cout << "Query Answered directly from Queried Node " << selectedMNode->getNodeID() << std::endl;
                //this->queryAnsweringNodeID = selectedMNode->getNodeID();
                return;
            }
            else // Case when the selectedMNode is not available for the query and higher level is aggregated to answer the query (levelDiff > 0)
            {
                this->queryAnsweringNodeID = mNode->getNodeID();
                //this->queryAnsweringNodeID = selectedMNode->getNodeID();
                //Finding queryDimension indices in the mVertexDimensions
                std::vector<int> queryDimensionIndices;
                getQueryDimensionIndices(queryDimensions, mNodeDimensions, queryDimensionIndices);

                //std::map<std::vector<DimensionKey>, double> mNodeKeyValues;
                std::map<std::vector<DimensionKey>, std::map<Timestamp, double > > mNodeKeyTimeValueMap;

                // Get values of materialized node
                //mNode->getAggregatedValueAllKeys(aggregateFunction, mNodeKeyValues);
                //selectedMNode->getAggregatedValueAllKeysWTimeGrain(aggregateFunction, OLAPTimeGrain, mNodeKeyTimeValueMap);
                mNode->getAggregatedValueAllKeysWTimeGrain(aggregateFunction, OLAPTimeGrain, mNodeKeyTimeValueMap);

                std::cout << "Answering query from other Node ID:" << mNode->getNodeID() << ", with num. rows need aggregation: " << mNodeKeyTimeValueMap.size() << std::endl;
                //std::cout << "Answering query from other Node ID:" << selectedMNode->getNodeID() << ", with num. rows need aggregation: " << mNodeKeyTimeValueMap.size() << std::endl;

                // Generate result of non-materialized node from materialized node
                generateAggregateFromMNode(queryDimensionIndices, aggregateFunction, mNodeKeyTimeValueMap, OLAPKeyTimeValueMap);
                return;
            }

            assert(false);
            std::cout << "MLattice: Query can not be answered." << std::endl;
            exit(0);
        }
    }
}

int MaterializedLattice::getAllAggregatedValuesForANodeWRelation(std::vector<DimensionKeyName> queryDimensions, std::string aggregateFunction, std::string OLAPTimeGrain, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap)
{
    // Sorting the nodes according to their size
    std::map<int,  boost::shared_ptr<MaterializedNodeWithRelation> > mNodeWNumRowsAsKeyMap;
    std::map<int,  boost::shared_ptr<MaterializedNodeWithRelation> >::iterator mNodeWNumRowsAsKeyMapIt;
    for(mNodesWRelationMapIt = mNodesWRelationMap.begin(); mNodesWRelationMapIt != mNodesWRelationMap.end(); mNodesWRelationMapIt++)
    {
        mNodeWNumRowsAsKeyMap.insert(std::make_pair( (mNodesWRelationMapIt->second)->getNodeSizeInRows(), mNodesWRelationMapIt->second));
    }

    // For testing purpose only
    //for(mNodeWNumRowsAsKeyMapIt = mNodeWNumRowsAsKeyMap.begin(); mNodeWNumRowsAsKeyMapIt != mNodeWNumRowsAsKeyMap.end(); mNodeWNumRowsAsKeyMapIt++)
    //{
    //    std::cout << "Num Rows: " << mNodeWNumRowsAsKeyMapIt->first << " Node ID: " << (mNodeWNumRowsAsKeyMapIt->second)->getNodeID() << std::endl;
    //}

    // Loop from nodes with smaller number of rows to higher number of rows
    for(mNodeWNumRowsAsKeyMapIt = mNodeWNumRowsAsKeyMap.begin(); mNodeWNumRowsAsKeyMapIt != mNodeWNumRowsAsKeyMap.end(); mNodeWNumRowsAsKeyMapIt++)
    {
        boost::shared_ptr<MaterializedNodeWithRelation> mNodeWRelation = mNodeWNumRowsAsKeyMapIt->second;

        std::vector<DimensionKeyName> mNodeDimensions;
        std::vector<DimensionKeyName>::iterator mNodeDimensionsIt;

        mNodeWRelation->getNodeDimensions(mNodeDimensions);

        // If query belongs to the same or lower level than mNode, only then it can be answered through mNode
        if(mNodeDimensions.size() >= queryDimensions.size() && containAllDimensions(queryDimensions, mNodeDimensions))
        {
            int levelDiff = mNodeDimensions.size() - queryDimensions.size();

            // Case when the mNode is available to answer the query directly (levelDiff = 0)
            if(levelDiff == 0)
            {
                mNodeWRelation->getAggregatedValueAllKeysWTimeGrain(aggregateFunction, OLAPTimeGrain, OLAPKeyTimeValueMap);
                int currentNodeSize = OLAPKeyTimeValueMap.size();
                //std::cout << "Query Answered directly from Queried Node " << mNodeWRelation->getNodeID() << ", with num. rows: " << currentNodeSize << std::endl;
                this->queryAnsweringNodeID = mNodeWRelation->getNodeID();
                return currentNodeSize;
            }
            else // Case when the selectedMNode is not available for the query and higher level is aggregated to answer the query (levelDiff > 0)
            {
                this->queryAnsweringNodeID = mNodeWRelation->getNodeID();
                std::vector<int> queryDimensionIndices;
                getQueryDimensionIndices(queryDimensions, mNodeDimensions, queryDimensionIndices);

                boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > > mNodeKeyTimeValueMap;

                // Get values of materialized node
                mNodeWRelation->getAggregatedValueAllKeysWTimeGrain(aggregateFunction, OLAPTimeGrain, mNodeKeyTimeValueMap);
                int currentNodeSize = mNodeKeyTimeValueMap.size();
                //std::cout << "Answering query from other Node ID:" << mNodeWRelation->getNodeID() << ", with num. rows need aggregation: " << currentNodeSize << std::endl;

                // Generate result of non-materialized node from materialized node
                generateAggregateFromMNode(queryDimensionIndices, aggregateFunction, mNodeKeyTimeValueMap, OLAPKeyTimeValueMap);
                return -1 * currentNodeSize;
            }

            assert(false);
            std::cout << "MLattice: Query can not be answered." << std::endl;
            exit(0);
        }
    }
}

// Returns aggregate of all the values for all the keys in a particular node, for the whole window, whose dimKeyNames are provided for the given aggregate function
void MaterializedLattice::getAllAggregatedValuesForANode(std::vector<DimensionKeyName> queryDimensions, std::string aggregateFunction, std::map<std::vector<DimensionKey>, double>& keyValues)
{
    // Loop from lower levels to higher levels of materialized vertices
    for(mNodesMapIt = mNodesMap.begin(); mNodesMapIt != mNodesMap.end(); mNodesMapIt++)
    {
        boost::shared_ptr<MaterializedNode> mNode = mNodesMapIt->second;

        std::vector<DimensionKeyName> mNodeDimensions;
        std::vector<DimensionKeyName>::iterator mNodeDimensionsIt;

        mNode->getNodeDimensions(mNodeDimensions);

        // If query belongs to the same or lower level than mNode, only then it can be answered through mNode
        if(mNodeDimensions.size() >= queryDimensions.size() && containAllDimensions(queryDimensions, mNodeDimensions))
        {

            int levelDiff = mNodeDimensions.size() - queryDimensions.size();

            // Case when the mNode is available to answer the query directly (levelDiff = 0)
            if(levelDiff == 0)
            {
                mNode->getAggregatedValueAllKeys(aggregateFunction, keyValues);
                return;
            }
            else // Case when the mNode is not available for the query and higher level is aggregated to answer the query (levelDiff > 0)
            {
                std::cout << "Answering query from other level of the lattice" << std::endl;
                //Finding queryDimension indices in the mVertexDimensions
                std::vector<int> queryDimensionIndices;
                getQueryDimensionIndices(queryDimensions, mNodeDimensions, queryDimensionIndices);

                // Find the keys not present in the query key
                //std::vector<DimensionKeyName> missingDim;
                //std::vector<int> missingDimIndices;
                //findMissingDimensions(queryDimensions, mNodeDimensions, missingDim, missingDimIndices);

                std::map<std::vector<DimensionKey>, double> mNodeKeyValues;
                // Get values of materialized node
                mNode->getAggregatedValueAllKeys(aggregateFunction, mNodeKeyValues);
                // Generate result of non-materialized node from materialized node
                generateAggregateFromMNode(queryDimensionIndices, aggregateFunction, mNodeKeyValues, keyValues);
                return;
            }

            assert(false);
            std::cout << "MLattice: Query can not be answered." << std::endl;
            exit(0);
        }

    }
}

void MaterializedLattice::generateAggregateFromMNode(std::vector<int> queryDimensionIndices, std::string aggregateFunction, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& mNodeKeyTimeValueMap, std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap)
{
    std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator mNodeKeyTimeValueMapIt;

    // Iterate through all the keys of the meterialized node
    for(mNodeKeyTimeValueMapIt = mNodeKeyTimeValueMap.begin(); mNodeKeyTimeValueMapIt != mNodeKeyTimeValueMap.end(); mNodeKeyTimeValueMapIt++)
    {
        std::vector<DimensionKey> queryDimensionKeys;
        for(int i = 0; i < queryDimensionIndices.size(); i++)
        {
            int dimIndex = queryDimensionIndices[i];

            if(isnan(dimIndex))
            {
                assert(false);
                std::cout << "MLattice: DimIndex is NaN!" << std::endl;
                exit(0);
            }
            queryDimensionKeys.push_back(mNodeKeyTimeValueMapIt->first[dimIndex]);
        }

        // Iterating through inner map of the materialized node
        std::map<Timestamp, double> mNodeTimeValueInnerMap = mNodeKeyTimeValueMapIt->second;
        std::map<Timestamp, double>::iterator mNodeTimeValueInnerMapIt;

        for(mNodeTimeValueInnerMapIt = mNodeTimeValueInnerMap.begin(); mNodeTimeValueInnerMapIt != mNodeTimeValueInnerMap.end(); mNodeTimeValueInnerMapIt++)
        {
            std::map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator OLAPKeyTimeValueMapIt;

            if( (OLAPKeyTimeValueMapIt = OLAPKeyTimeValueMap.find(queryDimensionKeys)) != OLAPKeyTimeValueMap.end() ) // if key found
            {
                //std::map<Timestamp, double> OLAPKeyTimeValueInnerMap = OLAPKeyTimeValueMapIt->second;
                // searcing the timestamp
                if( OLAPKeyTimeValueMapIt->second.find(mNodeTimeValueInnerMapIt->first) != OLAPKeyTimeValueMapIt->second.end() ) // if timestamp also found
                {
                    if(aggregateFunction == "sum")
                    {
                        OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] += mNodeTimeValueInnerMapIt->second;
                    }
                    else if(aggregateFunction == "max")
                    {
                        if( mNodeTimeValueInnerMapIt->second > OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] )
                            OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
                    }
                    else if(aggregateFunction == "min")
                    {
                        if(mNodeTimeValueInnerMapIt->second < OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first])
                            OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
                    }
                    else if(aggregateFunction == "avg")
                    {
                        std::cout << "MLattice: Avg not yet implemented!" << std::endl;
                        exit(0);
                    }
                    else if(aggregateFunction == "count")
                    {
                        std::cout << "MLattice: Count not yet implemented!" << std::endl;
                        exit(0);
                    }
                    else
                    {
                        std::cout << "MLattice: " << aggregateFunction << " is not a valid Aggregation Function!" << std::endl;
                        exit(0);
                    }
                }
                else // If timestamp not found
                {
                    OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
                }

            }
            else // if key not found
            {
                OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
            }
        }
    }
}

void MaterializedLattice::generateAggregateFromMNode(std::vector<int> queryDimensionIndices, std::string aggregateFunction, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& mNodeKeyTimeValueMap, boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >& OLAPKeyTimeValueMap)
{
    boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator mNodeKeyTimeValueMapIt;

    // Iterate through all the keys of the meterialized node
    for(mNodeKeyTimeValueMapIt = mNodeKeyTimeValueMap.begin(); mNodeKeyTimeValueMapIt != mNodeKeyTimeValueMap.end(); mNodeKeyTimeValueMapIt++)
    {
        std::vector<DimensionKey> queryDimensionKeys;
        for(int i = 0; i < queryDimensionIndices.size(); i++)
        {
            int dimIndex = queryDimensionIndices[i];

            if(isnan(dimIndex))
            {
                assert(false);
                std::cout << "MLattice: DimIndex is NaN!" << std::endl;
                exit(0);
            }
            queryDimensionKeys.push_back(mNodeKeyTimeValueMapIt->first[dimIndex]);
        }

        // Iterating through inner map of the materialized node
        std::map<Timestamp, double> mNodeTimeValueInnerMap = mNodeKeyTimeValueMapIt->second;
        std::map<Timestamp, double>::iterator mNodeTimeValueInnerMapIt;

        for(mNodeTimeValueInnerMapIt = mNodeTimeValueInnerMap.begin(); mNodeTimeValueInnerMapIt != mNodeTimeValueInnerMap.end(); mNodeTimeValueInnerMapIt++)
        {
            boost::unordered_map<std::vector<DimensionKey>, std::map<Timestamp, double > >::iterator OLAPKeyTimeValueMapIt;

            if( (OLAPKeyTimeValueMapIt = OLAPKeyTimeValueMap.find(queryDimensionKeys)) != OLAPKeyTimeValueMap.end() ) // if key found
            {
                //std::map<Timestamp, double> OLAPKeyTimeValueInnerMap = OLAPKeyTimeValueMapIt->second;
                // searcing the timestamp
                if( OLAPKeyTimeValueMapIt->second.find(mNodeTimeValueInnerMapIt->first) != OLAPKeyTimeValueMapIt->second.end() ) // if timestamp also found
                {
                    if(aggregateFunction == "sum")
                    {
                        OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] += mNodeTimeValueInnerMapIt->second;
                    }
                    else if(aggregateFunction == "max")
                    {
                        if( mNodeTimeValueInnerMapIt->second > OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] )
                            OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
                    }
                    else if(aggregateFunction == "min")
                    {
                        if(mNodeTimeValueInnerMapIt->second < OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first])
                            OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
                    }
                    else if(aggregateFunction == "avg")
                    {
                        std::cout << "MLattice: Avg not yet implemented!" << std::endl;
                        exit(0);
                    }
                    else if(aggregateFunction == "count")
                    {
                        std::cout << "MLattice: Count not yet implemented!" << std::endl;
                        exit(0);
                    }
                    else
                    {
                        std::cout << "MLattice: " << aggregateFunction << " is not a valid Aggregation Function!" << std::endl;
                        exit(0);
                    }
                }
                else // If timestamp not found
                {
                    OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
                }

            }
            else // if key not found
            {
                OLAPKeyTimeValueMap[queryDimensionKeys][mNodeTimeValueInnerMapIt->first] = mNodeTimeValueInnerMapIt->second;
            }
        }
    }

}

void MaterializedLattice::generateAggregateFromMNode(std::vector<int> queryDimensionIndices, std::string aggregateFunction, std::map<std::vector<DimensionKey>, double>& mNodeKeyValues, std::map<std::vector<DimensionKey>, double>& keyValues)
{
    std::map<std::vector<DimensionKey>, double>::iterator mNodeKeyValuesIt;
    // Iterate through all the keys of the meterialized node
    for(mNodeKeyValuesIt = mNodeKeyValues.begin(); mNodeKeyValuesIt != mNodeKeyValues.end(); mNodeKeyValuesIt++)
    {
        std::vector<DimensionKey> queryDimensionKeys;
        for(int i = 0; i < queryDimensionIndices.size(); i++)
        {
            int dimIndex = queryDimensionIndices[i];

            if(isnan(dimIndex))
            {
                assert(false);
                std::cout << "MLattice: DimIndex is NaN!" << std::endl;
                exit(0);
            }
            queryDimensionKeys.push_back(mNodeKeyValuesIt->first[dimIndex]);
        }

        if(keyValues.find(queryDimensionKeys) != keyValues.end()) // if key found
        {
            if(aggregateFunction == "sum")
            {
                keyValues[queryDimensionKeys] += mNodeKeyValuesIt->second;
            }
            else if(aggregateFunction == "max")
            {
                if(mNodeKeyValuesIt->second > keyValues[queryDimensionKeys])
                    keyValues[queryDimensionKeys] = mNodeKeyValuesIt->second;
            }
            else if(aggregateFunction == "min")
            {
                if(mNodeKeyValuesIt->second < keyValues[queryDimensionKeys])
                    keyValues[queryDimensionKeys] = mNodeKeyValuesIt->second;
            }
            else if(aggregateFunction == "avg")
            {
                std::cout << "MLattice: Avg not yet implemented!" << std::endl;
                exit(0);
            }
            else if(aggregateFunction == "count")
            {
                std::cout << "MLattice: Count not yet implemented!" << std::endl;
                exit(0);
            }
            else
            {
                std::cout << "MLattice: " << aggregateFunction << " is not a valid Aggregation Function!" << std::endl;
                exit(0);
            }
        }
        else // if key not found
        {
            keyValues[queryDimensionKeys] = mNodeKeyValuesIt->second;
        }
    }
}

bool MaterializedLattice::containAnyDimension(std::vector<std::string>& queryNode, std::vector<std::string>& latticeNode)
{
    std::vector<DimensionKeyName>::iterator queryNodeIt;
    std::vector<DimensionKeyName>::iterator latticeNodeIt;

    int matchingDim = 0;

    for(queryNodeIt = queryNode.begin(); queryNodeIt != queryNode.end(); queryNodeIt++)
        for(latticeNodeIt = latticeNode.begin(); latticeNodeIt != latticeNode.end(); latticeNodeIt++)
            if(*queryNodeIt == *latticeNodeIt)
                return true;
}

bool MaterializedLattice::containAllDimensions(std::vector<std::string>& queryVertex, std::vector<std::string>& mNode)
{
    std::vector<DimensionKeyName>::iterator queryVertexIt;
    std::vector<DimensionKeyName>::iterator mNodeIt;

    if(mNode.size() < queryVertex.size())
    {
        std::cout << "Can not answer query from lower level mNode" << std::endl;
        assert(false);
        exit(0);
    }
    else
    {
        int matchingDim = 0;

        for(queryVertexIt = queryVertex.begin(); queryVertexIt != queryVertex.end(); queryVertexIt++)
            for(mNodeIt = mNode.begin(); mNodeIt != mNode.end(); mNodeIt++)
            {
                //std::cout << *queryVertexIt << " " << *mNodeIt << std::endl;
                if(*queryVertexIt == *mNodeIt)
                    matchingDim++;
            }

        // if all the querying dimensions matches the materialized vertex dimensions
        if(matchingDim == queryVertex.size())
            return true;
        else if(matchingDim > queryVertex.size())
        {
            std::cout << "# matching dims can not be > query vertex dims" << std::endl;
            assert(false);
            exit(0);
        }
    }
    return false;
}

void MaterializedLattice::findMissingDimensions(std::vector<std::string>& queryVertex, std::vector<std::string>& mNode, std::vector<std::string>& missingDim, std::vector<int>& missingDimIndices)
{
    std::vector<std::string>::iterator mNodeIt;

    for(mNodeIt = mNode.begin(); mNodeIt != mNode.end(); mNodeIt++)
    {
        // If an element not found in queryVertex
        if ( std::find(queryVertex.begin(), queryVertex.end(), *mNodeIt) == queryVertex.end() )
        {
            missingDim.push_back(*mNodeIt);
            missingDimIndices.push_back(mNodeIt - mNode.begin());
            //std::cout << "Missing Dim " << *mNodeIt << ", " << mNodeIt - mNode.begin() << std::endl;
        }
    }
}

void MaterializedLattice::getQueryDimensionIndices(std::vector<DimensionKeyName> queryDimensions, std::vector<DimensionKeyName> mNodeDimensions, std::vector<int>& queryDimensionIndices)
{
    std::vector<std::string>::iterator mNodeDimensionsIt;

    for(mNodeDimensionsIt = mNodeDimensions.begin(); mNodeDimensionsIt != mNodeDimensions.end(); mNodeDimensionsIt++)
    {
        // If the mVertex dimension found in queryDimension vector
        if ( std::find(queryDimensions.begin(), queryDimensions.end(), *mNodeDimensionsIt) != queryDimensions.end() )
        {
            queryDimensionIndices.push_back(mNodeDimensionsIt - mNodeDimensions.begin());
            //std::cout << "Query Dim: at index: " << *mVertexDimensionsIt << ", " << mVertexDimensionsIt - mVertexDimensions.begin() << std::endl;
        }
    }

}

void MaterializedLattice::getMVerticesMap(std::map<vertexID,  boost::shared_ptr<MaterializedNode> >& materializedVerticesMap)
{
    materializedVerticesMap = mNodesMap;
}

void MaterializedLattice::getMVerticesWRelationMap(std::map<vertexID,  boost::shared_ptr<MaterializedNodeWithRelation> >& materializedVerticesMap)
{
    materializedVerticesMap = mNodesWRelationMap;
}
