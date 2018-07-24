//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../IO/IStreamInput.h"
#include "../IO/IStreamOutput.h"
#include "../IO/RelationInput.h"
#include "../IO/CSVInput.h"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <map>
#include "QueryIntermediateRepresentation.h"
#include "../Common/Types.h"
#include "QueryEntity.h"

/* manage the input and output for each query*/
class QueryManager
{
private:
	static QueryManager*  queryManager;
	QueryManager(void);
	std::map<std::string, boost::shared_ptr<IStreamInput> >registeredStreamMap;
	std::map<std::string, boost::shared_ptr<RelationInput> >registeredRelationMap;
	std::map<std::string, boost::shared_ptr<CSVInput> >registeredCSVMap;
	std::list<boost::shared_ptr<QueryEntity> >queryEntityList;
public:
	static QueryManager* getInstance();
	~QueryManager(void);
	void registerStream(boost::shared_ptr<IStreamInput> streamInput);
	void registerRelation(boost::shared_ptr<RelationInput> relationInput);
	void registerCSV(boost::shared_ptr<CSVInput> csvInput);


	boost::shared_ptr<IStreamInput>  getRegisteredStreamById(std::string id);//get the registered streaminput by id
	boost::shared_ptr<RelationInput>  getRegisteredRelationById(std::string id);//get the registered relationinput by id
	boost::shared_ptr<CSVInput> getRegisteredCSVById(std::string id);//get the registered CSVInput by id

	boost::shared_ptr<QueryEntity> addQuery(boost::shared_ptr<QueryIntermediateRepresentation>queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput, std::string queryDuration);
	boost::shared_ptr<QueryEntity> addQuery(boost::shared_ptr<QueryIntermediateRepresentation>queryIntermediateRepresentation,boost::shared_ptr<IStreamOutput> streamOutput, std::string queryDuration, bool isDispatcher);
	bool removeQuery(boost::shared_ptr<QueryEntity> queryEntity);
	std::list<boost::shared_ptr<QueryEntity> > getQueryEntityList(void);


};

