//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
//#include "../OLAP/OLAPManager.h"
#include "../Common/Types.h"
#include "../OLAP/OLAPManager.h"


//struct vertexInfo; // Forward decl

class StreamOLAPOptimization
{

    private:
        static StreamOLAPOptimization* streamOLAPOptimization;
        //void getNOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int queryWindowSize, int numVerticesToMaterialize, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex);
        void getNOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, int numVerticesToMaterialize, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex);
        //void getMaxStorageOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int queryWindowSize, int maxStorageNumTuples, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex);
        void getMaxStorageOptimizedVertices(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, int maxStorageNumTuples, std::vector<vertexInfo> &tmpMVertices, vertexInfo finestVertex, int IoA);
        double getCandidVertexCost(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, std::vector<vertexInfo> &tmpMVertices, int streamArrivalRate, vertexInfo finestVertex, int IoA);

        std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesIt;
        std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesItt;
        std::vector<vertexInfo>::iterator tmpMVerticesIt;
        std::vector<int> VIDsForM;
        std::vector<int>::iterator VIDsForMIt;
        std::vector<int> VIDsToIgnore;
        std::vector<int>::iterator VIDsToIgnoreIt;

    public:
        StreamOLAPOptimization();
        ~StreamOLAPOptimization();
        static StreamOLAPOptimization* getInstance();

        // optMode = numVertices or maxStorage
        void getOptimizedVerticesToMaterialize(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int streamArrivalRate, int queryWindowSize, std::string optMode, int maxStorageNumTuples, int numVerticesToMaterialize, int queryID, int IoA);
        double getVertexStorageCost(int windowSizeNumTuples, int vertexSizeNumTuples, int finestVertexSizeNumTuples, int tupleSizeBytes);
        double getQueryingCostFromMVertex(int windowSizeNumTuples, int MVertexSizeNumTuples, int finestVertexSizeNumTuples, int queryingVertxeRefFrequency);
        double getUpdateAndQueryingCost(int streamArrivalRate, int MVertexSizeNumTuples, int finestVertexNumTuples, int queryingVertexNumTuples, int queryingVertxeRefFrequency, bool isQueriedNodeMaterialized, int IoA);
        bool containAllDimensions(std::vector<std::string>& materializedVertex, std::vector<std::string>& queriedVertex);
        std::vector<std::string> getDimensionVectorByVertexID(std::map<vertexInfo, std::vector<std::string> > &latticeVertices, int vertexID);
};
