//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Query/QueryManager.h"
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include "QueryEntity.h"
QueryManager*  QueryManager::queryManager = NULL;
QueryManager::QueryManager(void)
{
}

QueryManager::~QueryManager(void)
{
}

QueryManager* QueryManager::getInstance(void)
{
	if (queryManager==NULL)
	{
		queryManager = new QueryManager();
	}
	return queryManager;
}

void QueryManager::registerStream(boost::shared_ptr<IStreamInput> streamInput)
{
	this->registeredStreamMap.insert(make_pair(streamInput->getId(), streamInput));
}

boost::shared_ptr<IStreamInput>  QueryManager::getRegisteredStreamById(std::string id)
{
 	std::map<std::string, boost::shared_ptr<IStreamInput> >::iterator it;
	it = registeredStreamMap.find(id);
	assert(it!=registeredStreamMap.end());

	return it->second;
}

bool QueryManager::removeQuery(boost::shared_ptr<QueryEntity> queryEntity)
{
    this->queryEntityList.remove(queryEntity);
    return true;
}

boost::shared_ptr<QueryEntity> QueryManager::addQuery(boost::shared_ptr<QueryIntermediateRepresentation>queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput, std::string queryDuration)
{
	boost::shared_ptr<QueryEntity>queryEntity(new QueryEntity(queryIntermediateRepresentation,streamOutput,queryDuration));
	this->queryEntityList.push_back(queryEntity);
	return queryEntity;
}

boost::shared_ptr<QueryEntity> QueryManager::addQuery(boost::shared_ptr<QueryIntermediateRepresentation>queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput, std::string queryDuration, bool isDispatcher)
{
	boost::shared_ptr<QueryEntity>queryEntity(new QueryEntity(queryIntermediateRepresentation,streamOutput,queryDuration,isDispatcher));

	this->queryEntityList.push_back(queryEntity);
	return queryEntity;
}

std::list<boost::shared_ptr<QueryEntity> > QueryManager::getQueryEntityList(void)
{
    return this->queryEntityList;
}

void QueryManager::registerRelation(boost::shared_ptr<RelationInput> relationInput)
{
	this->registeredRelationMap.insert(make_pair(relationInput->getId(), relationInput));
}

boost::shared_ptr<RelationInput> QueryManager::getRegisteredRelationById(std::string id)
{
	std::map<std::string, boost::shared_ptr<RelationInput> >::iterator it;
	it = registeredRelationMap.find(id);
	//assert(it != registeredStreamMap.end());

	return it->second;
}

// Added by salman
void QueryManager::registerCSV(boost::shared_ptr<CSVInput> csvInput)
{
    // Maps csvInput instance to JSON schemaID, e.g., productDimension
	this->registeredCSVMap.insert(make_pair(csvInput->getId(), csvInput));
}

boost::shared_ptr<CSVInput> QueryManager::getRegisteredCSVById(std::string id)
{
	std::map<std::string, boost::shared_ptr<CSVInput> >::iterator it;
	it = registeredCSVMap.find(id);
	assert(it != registeredCSVMap.end());

	return it->second;
}

