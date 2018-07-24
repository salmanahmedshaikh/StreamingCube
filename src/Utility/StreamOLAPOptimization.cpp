//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/StreamOLAPOptimization.h"
//#include "../OLAP/OLAPManager.h"

#include <iostream>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <map>
#include <math.h>

StreamOLAPOptimization* StreamOLAPOptimization::streamOLAPOptimization = NULL;

StreamOLAPOptimization::StreamOLAPOptimization()
{
    //ctor
}

StreamOLAPOptimization::~StreamOLAPOptimization()
{
    //dtor
}

StreamOLAPOptimization* StreamOLAPOptimization::getInstance()
 {
	 if(streamOLAPOptimization == NULL)
	 {
		 streamOLAPOptimization = new StreamOLAPOptimization();
	 }
	 return streamOLAPOptimization;
 }

 void StreamOLAPOptimization::getOptimizedVerticesToMaterialize(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, std::string optMethod, int maxStorageNumTuples, int numVerticesToMaterialize, int queryID, int IoA)
 {
    std::vector<vertexInfo> tmpMVertices;
    vertexInfo finestVertex;
    int finestVertexRows = INT_MIN;

    VIDsForM.clear();
    VIDsToIgnore.clear();

    // Finding the finest vertex
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        // Always materialize most granular node
        std::vector<std::string> dimensionsVector;
        OLAPManager::getInstance()->getConfigValue(queryID, "Dimensions", dimensionsVector);
        if(latticeVerticesIt->first.level == dimensionsVector.size())
        {
            latticeVerticesIt->first.isMaterialized = true;
            finestVertex = latticeVerticesIt->first;
            continue;
        }
    }

    tmpMVertices.push_back(finestVertex);
    VIDsForM.push_back(finestVertex.vertexID); // Vertex chosen for materialization
    VIDsToIgnore.push_back(finestVertex.vertexID); // Vertex chosen for materialization


    if(optMethod == "NumVertices") // selection of nodes to be materialized is based on number of vertices
    {
        getNOptimizedVertices(latticeVertices, streamArrivalRate, queryWindowSize, numVerticesToMaterialize, tmpMVertices, finestVertex);
    }
    else if (optMethod == "MaxStorage") // selection of nodes to be materialized is based on max available storage
    {
        getMaxStorageOptimizedVertices(latticeVertices, streamArrivalRate, queryWindowSize, maxStorageNumTuples, tmpMVertices, finestVertex, IoA);
    }
    else
    {
        std::cout << "Invalid Optimization Method." << std::endl;
        exit(0);
    }

    //marking the vertices to materialize
    //std::cout << "tmpMVertices size " << tmpMVertices.size() << std::endl;
    for(tmpMVerticesIt = tmpMVertices.begin(); tmpMVerticesIt != tmpMVertices.end(); tmpMVerticesIt++)
    {
        //std::cout << "(*tmpMVerticesIt).vertexID " << (*tmpMVerticesIt).vertexID << std::endl;
        for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
        {
            if( (*tmpMVerticesIt).vertexID == latticeVerticesIt->first.vertexID)
            {
                latticeVerticesIt->first.isMaterialized = true;

                //vertexInfo newVInfo = latticeVerticesIt->first;
                //std::vector<std::string> newVertexDimensions = latticeVerticesIt->second;

                //newVInfo.isMaterialized = true;

                //latticeVertices.erase(latticeVerticesIt);
                //latticeVertices.insert(std::make_pair(newVInfo, newVertexDimensions));

                continue;
            }
        }
    }
 }


 void StreamOLAPOptimization::getNOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, int numVerticesToMaterialize, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex)
 {
    // Outer loop to select the numVerticesToMaterialize
    for(int i = 0; i < numVerticesToMaterialize; i++)
    {
        double minTotalQueryingCost = INT_MAX;
        vertexInfo vertexWSoFarMinQueryingCost;
        //Select a vertex once at a time (greedy way) to check if it results in reduction in computation cost
        for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
        {
            // Select a vertex temporarily for materialization
            // Finding the IDs which have been chosen for materialization
            VIDsForMIt = std::find(VIDsForM.begin(), VIDsForM.end(), latticeVerticesIt->first.vertexID);

            if(VIDsForMIt != VIDsForM.end()) // ID found
                continue;
            else
                tmpMVertices.push_back(latticeVerticesIt->first); // Select a vertex temporarily for materialization
            //tmpMVertices.push_back(latticeVerticesIt->first);
            //std::cout << "latticeVerticesIt->first.vertexID " << latticeVerticesIt->first.vertexID << std::endl;

            double totalQueryingCost = 0;
            // Iterate through all the vertices to check if the temporary selected vertex can minimize the overall query processing cost
            for(latticeVerticesItt = latticeVertices.begin(); latticeVerticesItt != latticeVertices.end(); latticeVerticesItt++)
            {
                double minPerNodeQueryingCost = INT_MAX;

                for(tmpMVerticesIt = tmpMVertices.begin(); tmpMVerticesIt != tmpMVertices.end(); tmpMVerticesIt++)
                {
                    std::vector<std::string> MVertexDimensionVector = getDimensionVectorByVertexID(latticeVertices, (*tmpMVerticesIt).vertexID);
                    //if materialized vertex contains all the dimensions only then it can answer the query
                    if(containAllDimensions(MVertexDimensionVector, latticeVerticesItt->second))
                    {
                        double queryingCost = getQueryingCostFromMVertex(queryWindowSize, (*tmpMVerticesIt).vRows, finestVertex.vRows, latticeVerticesItt->first.refFrequency);
                        //std::cout << "queryingCost " << queryingCost << std::endl;

                        if(queryingCost < minPerNodeQueryingCost)
                        {
                            minPerNodeQueryingCost = queryingCost;
                        }
                    }
                }

                //std::cout << "minPerNodeQueryingCost " << minPerNodeQueryingCost << std::endl;
                totalQueryingCost += minPerNodeQueryingCost;
                //std::cout << "minPerNodeQueryingCost " << minPerNodeQueryingCost << std::endl;
            }

            //std::cout << "totalQueryingCost if the vertexID " << latticeVerticesIt->first.vertexID << " is selected for materialization: " << totalQueryingCost << std::endl;
            if(totalQueryingCost < minTotalQueryingCost)
            {
                minTotalQueryingCost = totalQueryingCost;
                vertexWSoFarMinQueryingCost = latticeVerticesIt->first;
            }
            // Remove the temporary vertex for materialization
            tmpMVertices.pop_back();
        }
        // After each outer loop iteration, we find a vertex with min querying cost
        tmpMVertices.push_back(vertexWSoFarMinQueryingCost);
        VIDsForM.push_back(vertexWSoFarMinQueryingCost.vertexID);
        //std::cout << "Finalized verted for materialization after " << i+1 << " iteration " << vertexWSoFarMinQueryingCost.vertexID << std::endl;
        //for(int j = 0; j < tmpMVertices.size(); j++)
            //std::cout << "Finalized vertices after first iteration " << tmpMVertices[j].vertexID << std::endl;
        //sleep(1);
    }

    //std::cout << "tmpMVertices size " << tmpMVertices.size() << std::endl;
    //for(tmpMVerticesIt = tmpMVertices.begin(); tmpMVerticesIt != tmpMVertices.end(); tmpMVerticesIt++)
    //{
    //    std::cout << "Finalized Vertex ID " << (*tmpMVerticesIt).vertexID << std::endl;
    //}
 }

 void StreamOLAPOptimization::getMaxStorageOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, int maxStorageNumTuples, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex, int IoA)
 {
    int storageConsumed = 0;
    if(maxStorageNumTuples <= 0) // no stoarge for materialization
        return;

    // Outer while loop to check the maxStorage
    while(storageConsumed < maxStorageNumTuples)
    {
        double minCandidVertexCost = DBL_MAX;
        int minCandidVertexID = -1;
        vertexInfo vertexWSoFarMinQueryingCost;

        //Select a vertex once at a time (greedy way) to check if it results in reduction in computation cost
        for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
        {
            // Finding the IDs which have been chosen for materialization
            VIDsForMIt = std::find(VIDsForM.begin(), VIDsForM.end(), latticeVerticesIt->first.vertexID);
            // Finding the IDs which must be ignored
            VIDsToIgnoreIt = std::find(VIDsToIgnore.begin(), VIDsToIgnore.end(), latticeVerticesIt->first.vertexID);

            // if the selection of this vertex results in violation of storage limites then ignore it
            if(storageConsumed + latticeVerticesIt->first.vRows > maxStorageNumTuples)
            {
                // Insert if it is not already in the list of vertices to ignore
                if(VIDsToIgnoreIt == VIDsToIgnore.end())
                    VIDsToIgnore.push_back(latticeVerticesIt->first.vertexID);

                //If all the vertices have been selected to ignore then return
                if(VIDsToIgnore.size() == latticeVertices.size())
                {
                    //std::cout << "storageConsumed: " << storageConsumed << " | maxStorage: " << maxStorageNumTuples << std::endl;
                    return;
                }

                continue;
            }
            if(VIDsForMIt != VIDsForM.end() || VIDsToIgnoreIt != VIDsToIgnore.end()) // If a vertex already chosen for materlization or chosen to ignore then continue with other IDs
            {
                continue;
            }
            else
            {
                // Select a vertex temporarily for materialization
                tmpMVertices.push_back(latticeVerticesIt->first);
            }

            double candidVertexCost = getCandidVertexCost(latticeVertices, tmpMVertices, streamArrivalRate, finestVertex, IoA);
            //std::cout << candidVertexCost << std::endl;

            if(candidVertexCost < minCandidVertexCost)
            {
                minCandidVertexCost = candidVertexCost;
                minCandidVertexID = latticeVerticesIt->first.vertexID;
                vertexWSoFarMinQueryingCost = latticeVerticesIt->first;
            }

            // Removing the temporary selected vertex
            tmpMVertices.pop_back();
        }

        if(minCandidVertexID != -1)
        {
            // After completion of each for loop, we find a vertex for materialization with min cost
            tmpMVertices.push_back(vertexWSoFarMinQueryingCost);
            VIDsForM.push_back(minCandidVertexID);
            VIDsToIgnore.push_back(minCandidVertexID);
            storageConsumed += vertexWSoFarMinQueryingCost.vRows;
        }

        //Testing loop
        //std::cout << "VIDsForM.size():" << VIDsForM.size() << " | VIDsToIgnore.size():" << VIDsToIgnore.size() << " | latticeVertices.size():" << latticeVertices.size() << std::endl;
        //for(VIDsToIgnoreIt = VIDsToIgnore.begin(); VIDsToIgnoreIt != VIDsToIgnore.end(); VIDsToIgnoreIt++)
            //std::cout << VIDsToIgnoreIt - VIDsToIgnore.begin() << ":"<< *VIDsToIgnoreIt << std::endl;

        // If all the vertices have been selected for materialization or to ignore or all the storage has been consumed then exit this function
        if(VIDsToIgnore.size() == latticeVertices.size() || tmpMVertices.size() == latticeVertices.size() || storageConsumed == maxStorageNumTuples)
        {
            //std::cout << "storageConsumed: " << storageConsumed << " | maxStorage: " << maxStorageNumTuples << std::endl;
            return;
        }
    }
 }

 double StreamOLAPOptimization::getCandidVertexCost(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, std::vector<vertexInfo> &tmpMVertices, int streamArrivalRate, vertexInfo finestVertex, int IoA)
 {
    double totalQueryingCost = 0;
    // Iterating through all the vertices as queries to find the querying cost.
    for(latticeVerticesItt = latticeVertices.begin(); latticeVerticesItt != latticeVertices.end(); latticeVerticesItt++)
    {
        double minPerNodeQueryingCost = DBL_MAX;

        // Finding the best materialized vertex (with min queryingCost) to answer the query
        for(tmpMVerticesIt = tmpMVertices.begin(); tmpMVerticesIt != tmpMVertices.end(); tmpMVerticesIt++)
        {
            std::vector<std::string> MVertexDimensionVector = getDimensionVectorByVertexID(latticeVertices, (*tmpMVerticesIt).vertexID);
            //if materialized vertex contains all the dimensions only then it can answer the query
            if(containAllDimensions(MVertexDimensionVector, latticeVerticesItt->second))
            {
                //std::cout << "Materialized lattice vertex which can answer the query (latticeVerticesItt): " << (*tmpMVerticesIt).vertexID << std::endl;
                double queryingCost = 0;
                bool isQueriedNodeMaterialized = false;
                // If querying node (latticeVerticesItt) is materialized
                if(latticeVerticesItt->first.vertexID == (*tmpMVerticesIt).vertexID)
                    isQueriedNodeMaterialized = true;

                //double queryingCost = getQueryingCostFromMVertex(queryWindowSize, (*tmpMVerticesIt).vRows, finestVertex.vRows, latticeVerticesItt->first.refFrequency);
                queryingCost = getUpdateAndQueryingCost(streamArrivalRate, (*tmpMVerticesIt).vRows, finestVertex.vRows, latticeVerticesItt->first.vRows, latticeVerticesItt->first.refFrequency, isQueriedNodeMaterialized, IoA);
                //std::cout << "queryingCost of node ID " << latticeVerticesItt->first.vertexID << " from nodeID " << (*tmpMVerticesIt).vertexID << ", is: " << queryingCost << std::endl;

                if(queryingCost < minPerNodeQueryingCost)
                {
                    minPerNodeQueryingCost = queryingCost;
                }
            }
        }

        //std::cout << "minPerNodeQueryingCost " << minPerNodeQueryingCost << std::endl;
        totalQueryingCost += minPerNodeQueryingCost;
        //std::cout << "totalQueryingCost " << totalQueryingCost << std::endl;
    }

    return totalQueryingCost;
 }

 /*
 void StreamOLAPOptimization::getMaxStorageOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, int maxStorageNumTuples, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex, int IoA)
 {
    if(maxStorageNumTuples == 0) // no stoarge for materialization
        return;

    int storageConsumed = 0;
    // Outer while loop to check the maxStorage
    while(storageConsumed < maxStorageNumTuples)
    {
        double minTotalQueryingCost = DBL_MAX;
        vertexInfo vertexWSoFarMinQueryingCost;
        //std::cout << "storageConsumed: " << storageConsumed << " | maxStorage: " << maxStorageNumTuples << std::endl;

        //Select a vertex once at a time (greedy way) to check if it results in reduction in computation cost
        for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
        {
            // Finding the IDs which have been chosen for materialization
            VIDsForMIt = std::find(VIDsForM.begin(), VIDsForM.end(), latticeVerticesIt->first.vertexID);
            // Finding the IDs which must be ignored
            VIDsToIgnoreIt = std::find(VIDsToIgnore.begin(), VIDsToIgnore.end(), latticeVerticesIt->first.vertexID);

            // If a vertex already chosen for materlization or chosen to ignore then continue with other IDs
            if(VIDsForMIt != VIDsForM.end() || VIDsToIgnoreIt != VIDsToIgnore.end())
                continue;
            else if(storageConsumed + latticeVerticesIt->first.vRows > maxStorageNumTuples)
            {
                // if the selection of this vertex results in violation of storage limites then ignore it
                VIDsToIgnore.push_back(latticeVerticesIt->first.vertexID);
                continue;
            }
            else
                tmpMVertices.push_back(latticeVerticesIt->first); // Select a vertex temporarily for materialization

            //std::cout << "tmpMVertices size " << tmpMVertices.size() << std::endl;

            double totalQueryingCost = 0;
            // Iterate through all the vertices to check if the temporary selected vertex can minimize the overall query processing cost
            for(latticeVerticesItt = latticeVertices.begin(); latticeVerticesItt != latticeVertices.end(); latticeVerticesItt++)
            {
                double minPerNodeQueryingCost = DBL_MAX;

                for(tmpMVerticesIt = tmpMVertices.begin(); tmpMVerticesIt != tmpMVertices.end(); tmpMVerticesIt++)
                {
                    std::vector<std::string> MVertexDimensionVector = getDimensionVectorByVertexID(latticeVertices, (*tmpMVerticesIt).vertexID);
                    //if materialized vertex contains all the dimensions only then it can answer the query
                    if(containAllDimensions(MVertexDimensionVector, latticeVerticesItt->second))
                    {
                        // Finding if the latticeVerticesIt->first.vertexID has already been chosen for materialization it has been under consideration for materialization during this iteration
                        VIDsForMIt = std::find(VIDsForM.begin(), VIDsForM.end(), latticeVerticesIt->first.vertexID);

                        bool isQueriedNodeMaterialized;
                        if( VIDsForMIt != VIDsForM.end() || latticeVerticesIt->first.vertexID == latticeVerticesItt->first.vertexID ) // ID found
                        {
                            isQueriedNodeMaterialized = true;
                            //std::cout << ">>>>>>>>>>>>>>>>>> Materialized node is being queried >>>>>>>>>>>>>>>>>> " << std::endl;
                        }
                        else
                        {
                            isQueriedNodeMaterialized = false;
                        }

                        //double queryingCost = getQueryingCostFromMVertex(queryWindowSize, (*tmpMVerticesIt).vRows, finestVertex.vRows, latticeVerticesItt->first.refFrequency);
                        double queryingCost = getUpdateAndQueryingCost(streamArrivalRate, (*tmpMVerticesIt).vRows, finestVertex.vRows, latticeVerticesItt->first.vRows, latticeVerticesItt->first.refFrequency, isQueriedNodeMaterialized, IoA);
                        //std::cout << "queryingCost of node ID " << latticeVerticesItt->first.vertexID << " from nodeID " << (*tmpMVerticesIt).vertexID << ", is: " << queryingCost << std::endl;

                        if(queryingCost < minPerNodeQueryingCost)
                        {
                            minPerNodeQueryingCost = queryingCost;
                        }
                    }
                }

                //std::cout << "minPerNodeQueryingCost " << minPerNodeQueryingCost << std::endl;
                totalQueryingCost += minPerNodeQueryingCost;
                //std::cout << "totalQueryingCost " << totalQueryingCost << std::endl;
            }

            //std::cout << "totalQueryingCost if the vertexID " << latticeVerticesIt->first.vertexID << " is chosen for materialization : " << totalQueryingCost << std::endl;
            //std::cout << "totalQueryingCost " << totalQueryingCost << " | minTotalQueryingCost" << minTotalQueryingCost << std::endl;
            if(totalQueryingCost < minTotalQueryingCost)
            {
                //std::cout << "totalQueryingCost " << totalQueryingCost << " | " << latticeVerticesItt->first.vertexID << std::endl;
                minTotalQueryingCost = totalQueryingCost;
                vertexWSoFarMinQueryingCost = latticeVerticesIt->first;

            }
            // Remove the temporary vertex for materialization
            tmpMVertices.pop_back();
        }
        // After each outer loop iteration, we find a vertex with min querying cost
        //std::cout << "Finalized Vertex for Materialization " << vertexWSoFarMinQueryingCost.vertexID << std::endl << std::endl;
        tmpMVertices.push_back(vertexWSoFarMinQueryingCost);
        VIDsForM.push_back(vertexWSoFarMinQueryingCost.vertexID);
        storageConsumed += vertexWSoFarMinQueryingCost.vRows;

        //std::cout << "tmpMVertices.size(): " << tmpMVertices.size() << std::endl;

        // If all the vertices have been selected for materialization then exit this function
        if(tmpMVertices.size() == latticeVertices.size() || storageConsumed == maxStorageNumTuples)
        {
            //std::cout << "tmpMVertices.size(): " << tmpMVertices.size() << " | latticeVertices.size(): " << latticeVertices.size() << std::endl;
            //std::cout << "All the vertices have been selected for materialization." << std::endl;
            return;
        }

        //std::cout << std::endl;
        //for(int j = 0; j < tmpMVertices.size(); j++)
            //std::cout << "Finalized vertices after first iteration " << tmpMVertices[j].vertexID << std::endl;

        //std::cout << "Storage Consumed So Far: " << storageConsumed << " | Available Storage: " << maxStorageNumTuples - storageConsumed << std::endl;

        // Choosing the vertices to ignore for materialization due to their large size resulting in violation of condition (storageConsumed > maxStorageNumTuples)
        if(storageConsumed > maxStorageNumTuples)
        {
            //std::cout << "This code must not be exeucted" << std::endl;
            //exit(0);

            // Removing the last vertex which resulted in the violation of condition storageConsumed < maxStorageNumTuples
            tmpMVertices.pop_back();
            VIDsForM.pop_back();
            VIDsToIgnore.push_back(vertexWSoFarMinQueryingCost.vertexID);
            storageConsumed -= vertexWSoFarMinQueryingCost.vRows;

            // Check if some smaller vertex can be materialized
            std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesIttt;
            for(latticeVerticesIttt = latticeVertices.begin(); latticeVerticesIttt != latticeVertices.end(); latticeVerticesIttt++)
            {
                // Finding the IDs which have been chosen for materialization or to be ignored
                VIDsForMIt = std::find(VIDsForM.begin(), VIDsForM.end(), latticeVerticesIttt->first.vertexID);
                VIDsToIgnoreIt = std::find(VIDsToIgnore.begin(), VIDsToIgnore.end(), latticeVerticesIttt->first.vertexID);

                if(VIDsForMIt != VIDsForM.end() || VIDsToIgnoreIt != VIDsToIgnore.end()) // ID found either in the list of vertices to materialize or in the list of vertices to ignore
                    continue;
                else if ( (latticeVerticesIttt->first.vRows + storageConsumed) > maxStorageNumTuples ) // if some vertex chosen for materialization results in violation of maxStorageNumTuples then add it to the list of vertices to ignore and continue
                {
                    VIDsToIgnore.push_back(latticeVerticesIttt->first.vertexID);
                    continue;
                }
                else  // found a vertex which can be materialized within available memory i.e., which satisfies (latticeVerticesIttt->first.vRows + storageConsumed) <= maxStorageNumTuples
                {
                    break; // break the latticeVerticesIttt for loop as soon as we find a vertex to materialize within available memory.
                }
            }
        }


    }
 }
 */

 double StreamOLAPOptimization::getVertexStorageCost(int windowSizeNumTuples, int vertexSizeNumTuples, int finestVertexSizeNumTuples, int tupleSizeBytes)
 {
    double vertexStorageCost = (windowSizeNumTuples*1.0) * ((vertexSizeNumTuples*1.0) / (finestVertexSizeNumTuples*1.0)) * (tupleSizeBytes*1.0);
    if (!(isnan(vertexStorageCost)))
        return vertexStorageCost;
    else
    {
        std::cout << "vertexStorageCost is NaN" << std::endl;
        exit(0);
    }
 }

 double StreamOLAPOptimization::getQueryingCostFromMVertex(int windowSizeNumTuples, int MVertexSizeNumTuples, int finestVertexSizeNumTuples, int queryingVertxeRefFrequency)
 {
     //std::cout << "windowSizeNumTuples " << windowSizeNumTuples << "| MVertexSizeNumTuples " << MVertexSizeNumTuples << "| finestVertexSizeNumTuples " << finestVertexSizeNumTuples << "| queryingVertxeRefFrequency " << queryingVertxeRefFrequency << std::endl;
     double queryingCost = ((windowSizeNumTuples*1.0)/(finestVertexSizeNumTuples*1.0)) * (MVertexSizeNumTuples*1.0) * (queryingVertxeRefFrequency*1.0);

    if (!(isnan(queryingCost)))
        return queryingCost;
    else
    {
        std::cout << "queryingCost is NaN" << std::endl;
        exit(0);
    }
 }

 double StreamOLAPOptimization::getUpdateAndQueryingCost(int streamArrivalRate, int MVertexSizeNumTuples, int finestVertexNumTuples, int queryingVertexNumTuples, int queryingVertxeRefFrequency, bool isQueriedNodeMaterialized, int IoA)
 {
    //std::cout << "streamArrivalRate " << streamArrivalRate << "| MVertexSizeNumTuples " << MVertexSizeNumTuples << "| queryingVertxeRefFrequency " << queryingVertxeRefFrequency << "| isQueriedNodeMaterialized " << isQueriedNodeMaterialized << "| IoA " << IoA << std::endl;

    int queryingCostCoefficient;
    if(isQueriedNodeMaterialized)
        queryingCostCoefficient = 1;
    else
        queryingCostCoefficient = 2;


    double updateCostCoefficient = 0.5;

    double alpha = (queryingVertexNumTuples * 1.0) / (finestVertexNumTuples * 1.0);
    double insertionCost = (streamArrivalRate * 1.0) * ( ( (streamArrivalRate * 1.0) * alpha) / 2.0 );
    double deletionCost = (streamArrivalRate * 1.0) * (alpha * 1.0);
    double queryingCost = ( (queryingVertxeRefFrequency * 1.0) * IoA * (streamArrivalRate * 1.0) * (MVertexSizeNumTuples * 1.0) ) / (finestVertexNumTuples * 1.0);

    //std::cout << "alpha " << alpha << "| insertionCost " << insertionCost << "| deletionCost " << deletionCost << "| queryingCost " << queryingCost << std::endl;
    double updateAndQueryingCost = updateCostCoefficient * ( insertionCost + deletionCost) +  (1 - updateCostCoefficient) * queryingCost * queryingCostCoefficient;

    //double queryingCost = ( 2*streamArrivalRate*streamArrivalRate*queryingVertexNumTuples*1.0 )/( finestVertexNumTuples*1.0 ) + ( queryingVertxeRefFrequency*1.0 * (queryingCostCoefficient * MVertexSizeNumTuples) );
    //double queryingCost = ( (streamArrivalRate*1.0) * log(MVertexSizeNumTuples*1.0) ) + ( queryingVertxeRefFrequency*1.0 * (queryingCostCoefficient * MVertexSizeNumTuples) );
    //double queryingCost = (queryingVertxeRefFrequency * 1.0) * (queryingCostCoefficient * MVertexSizeNumTuples);
    //std::cout << "Update cost " << alpha * (streamArrivalRate*1.0) * log(MVertexSizeNumTuples*1.0) << std::endl;
    //std::cout << "Querying cost " << (1-alpha) * queryingVertxeRefFrequency*1.0 * (queryingCostCoefficient * MVertexSizeNumTuples) << std::endl;
    //std::cout << "updateAndQueryingCost " << updateAndQueryingCost << std::endl;

    if ( !(isnan(updateAndQueryingCost)) )
        return updateAndQueryingCost;
    else
    {
        std::cout << "queryingCost is NaN" << std::endl;
        exit(0);
    }
 }

bool StreamOLAPOptimization::containAllDimensions(std::vector<std::string>& materializedVertex, std::vector<std::string>& queriedVertex)
{
    std::vector<std::string>::iterator queryVertexIt;
    std::vector<std::string>::iterator mVertexIt;

    if(materializedVertex.size() < queriedVertex.size())
    {
        //std::cout << "Can not answer query from lower level mVertex" << std::endl;
        //assert(false);
        //exit(0);
        return false;
    }
    else
    {
        int matchingDim = 0;

        for(queryVertexIt = queriedVertex.begin(); queryVertexIt != queriedVertex.end(); queryVertexIt++)
            for(mVertexIt = materializedVertex.begin(); mVertexIt != materializedVertex.end(); mVertexIt++)
                if(*queryVertexIt == *mVertexIt)
                    matchingDim++;

        // if all the querying dimensions matches the materialized vertex dimensions
        if(matchingDim == queriedVertex.size())
            return true;
        else if(matchingDim > queriedVertex.size())
        {
            std::cout << "# matching dims can not be > query vertex dims" << std::endl;
            assert(false);
            exit(0);
        }
    }
    return false;
}

std::vector<std::string> StreamOLAPOptimization::getDimensionVectorByVertexID(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int vertexID)
{
    std::vector<std::string> dimensionVector;
    std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesItr;

    for(latticeVerticesItr = latticeVertices.begin(); latticeVerticesItr != latticeVertices.end(); latticeVerticesItr++)
    {
        if(latticeVerticesItr->first.vertexID == vertexID)
        {
            dimensionVector = latticeVerticesItr->second;
            break;
        }
    }

    return dimensionVector;
}


 //std::vector<std::string> dimensionSizes;
 //int QueryID = 0;
 //OLAPManager::getInstance()->getConfigValue(QueryID, "DimensionSizes", dimensionSizes);
