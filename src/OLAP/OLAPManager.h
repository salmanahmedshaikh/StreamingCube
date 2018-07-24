//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Utility/StreamOLAPOptimization.h"
#include "../Common/Types.h"

//#include "../Operator/CubifyOperator.h"
#include <boost/property_map/property_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

class CubifyOperator; // Forward decl

//static const std::string CUBIFY_CONFIGURE_FILE_PATH = "./configure/cubify.conf";
/*
struct vertexKeyValue
{
    std::vector<unsigned int> keys;
    float value;
};

struct vertexInfo
{
    int vertexID;
    int level;
    int ordinal;
    int vRows;
    int refFrequency;
    bool isMaterialized;

    bool operator<(const vertexInfo &vInfo)  const
    {
        return level < vInfo.level || (level == vInfo.level && ordinal < vInfo.ordinal);
    }
};

struct vertexID
{
    int level;
    int ordinal;

    bool operator<(const vertexID &vID)  const
    {
        return level < vID.level || (level == vID.level && ordinal < vID.ordinal);
    }
};
*/

class OLAPManager:private boost::noncopyable
{
    public:
        OLAPManager();
        //OLAPManager(std::string configFile);
        ~OLAPManager();

        static OLAPManager* getInstance(void);
        //void getLatticeVertices(std::string configFile, std::map<vertexInfo, std::vector<std::string> >& latVertices);
        std::map<vertexInfo, std::vector<std::string> > getLatticeVertices(std::string configFile, int queryID);
        std::map<vertexInfo, std::vector<std::string> > getLatticeVertices(int queryID);
        int getNumOfMaterializedVertices(int queryID);

        std::string getConfigValue(int queryID, std::string key);
        void getConfigValue(int queryID, std::string key, std::vector<std::string>& configValueVector);
        void getConfigValue(int queryID, std::string key, std::vector< std::vector<std::string> >& configValueVector);


        void mapQueryIDCubifyOp(int queryID, boost::shared_ptr<CubifyOperator> cubifyOp);
        boost::shared_ptr<CubifyOperator> getCubifyOpByQueryID(int queryID);

        void markVerticesToMaterializeThruConfigFile();
        void markVerticesToMaterializeThruRefFreq(int queryID);
        void markVerticesToMaterializeRandomly(int queryID);
        void markNVerticesToMaterializeThruRefFreq(int queryID);
        void markNVerticesToMaterializeRandomly(int queryID);
        void markVerticesToMaterializeThruOptScheme(int queryID);
        void markSmallestVerticesToMaterialize(int queryID);
        void markLargestVerticesToMaterialize(int queryID);

        void printVertices();
        //std::vector<std::string> getCubifyDimensions();
        //std::vector< std::vector<std::string> > getCubifyMVertices();

    protected:
    private:
        static OLAPManager * OLAP;
        int numVerticesMaterialized;
        //std::vector<std::string> cubifyDimensions;
        //std::vector< std::vector<std::string> > cubifyMVertices;
        //std::string OLAPConfigFile;

        //QueryEntity* queryEntity;
        std::vector<std::string> dimensions;
        std::vector<std::string>::iterator dimensionsIt;

        std::vector<std::string> dimensionNames;
        std::vector<std::string> dimensionSizes;
        std::vector<std::string> refFrequencies;

        std::vector< std::vector<std::string> > mVertices;
        std::vector< std::vector<std::string> >::iterator mVerticesIt;

        std::vector< std::vector<std::string> > latticeOutputVertices;

        std::map<vertexInfo, std::vector<std::string> > latticeVertices;
        std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesIt;

        void generateLatticeVertices(std::vector<std::string>);
        //unsigned int combinations(unsigned int n, unsigned int k);
        void readCubifyConfiguration(std::string configFile, int queryID);
        void parseCSVString(std::string csvStr, std::vector<std::string>& strVector);

        std::map<int, std::map<std::string, std::string> > cubifyConfigMap;
        std::map<int, std::map<std::string, std::string> >::iterator cubifyConfigMapIt;

        std::map<int, std::map<std::string, std::vector<std::string> > > cubifyConfigVectorMap;
        std::map<int, std::map<std::string, std::vector<std::string> > >::iterator cubifyConfigVectorMapIt;
        std::map<std::string, std::vector<std::string> >::iterator cubifyConfigVectorMapInnerIt;

        std::map<int, std::map<std::string, std::vector< std::vector<std::string> > > > cubifyConfigNestedVectorMap;
        std::map<int, std::map<std::string, std::vector< std::vector<std::string> > > >::iterator cubifyConfigNestedVectorMapIt;
        std::map<std::string, std::vector< std::vector<std::string> > >::iterator cubifyConfigNestedVectorMapInnerIt;

        std::map<int, boost::shared_ptr<CubifyOperator> > queryIDCubifyOpMap;

        // map of lattice vertces with respect to queryID
        std::map<int, std::map<vertexInfo, std::vector<std::string> > > queryIDLatticeVerticesMap;
        void assignFrequency(int queryID, std::string RFMode);
        std::map<vertexInfo, std::vector<std::string> >::iterator getLatticeVertexByID(int VID);
};
