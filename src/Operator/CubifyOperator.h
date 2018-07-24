//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * CubifyOperator.h
 *
 *  Created on: August 24, 2016
 *      Author: salman
 */

#pragma once
#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Query/QueryEntity.h"
#include "../Utility/TimeCounterUtility.h"
#include "../OLAP/OLAPManager.h"
#include "../OLAP/MaterializedNode.h"
#include "../OLAP/MaterializedLattice.h"

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

class CubifyOperator:public Operator
{

private:

    /*
    struct vertexKeyValue
    {
        std::vector<unsigned int> keys;
        float value;
    };

    struct vertexInfo
    {
        int vertexID;
        unsigned int level;
        unsigned int ordinal;
        bool isMaterialized;

        bool operator<(const vertexInfo &vInfo)  const
        {
            return level < vInfo.level || (level == vInfo.level && ordinal < vInfo.ordinal);
        }
    };
    */


//bool operator< ( vertexInfo a, vertexInfo b ) { return std::make_pair(a.level,a.ordinal) < std::make_pair(b.level,b.ordinal) ; }

//bool operator< ( vertexInfo a) { return std::make_pair(a.level,a.ordinal) < std::make_pair(b.level,b.ordinal) ; }

    /*
    struct verticesToMaterialize
    {
        std::vector<unsigned int> keys;
    };
    */

    //QueryEntity* queryEntity;
    //std::vector<std::string> dimensions;
    //std::vector<std::string>::iterator dimensionsIt;

	//std::vector< std::vector<std::string> > mVertices;
	//std::vector< std::vector<std::string> >::iterator mVerticesIt;

	int queryId;
	std::string timeGrain;
	Timestamp IoA;
	QueryEntity* queryEntity;
	Timestamp latestElementTimestamp;

	boost::shared_ptr<MaterializedLattice> mLattice;

	std::vector<Element> integratedPlusMinusElements;

	std::map<vertexInfo, std::vector<std::string> > latticeVertices;
	std::map<vertexInfo, std::vector<std::string> >::iterator latticeVerticesIt;



	//void generateLatticeVertices(std::vector<std::string> s);
	//unsigned int combinations(unsigned int n, unsigned int k);


public:

    static int insertedTupleCounter;

	CubifyOperator();
	CubifyOperator(std::string configFile, int queryID);
	virtual ~CubifyOperator();

	void execution();

	void setQueryEntity(QueryEntity* queryEntity);
	QueryEntity* getQueryEntity();

	boost::shared_ptr<MaterializedLattice> getAssociatedMaterializedLattice();

	Timestamp getLatestElementTimestamp();
};
