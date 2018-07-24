//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Operator/Operator.h"
#include "../Internal/Element/ElementIterator.h"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "../Query/QueryProjectionAbstract.h"
#include <boost/any.hpp>

class GroupAggregationOperator: public Operator {
private:
	boost::shared_ptr<RelationSynopsis> relationSynopsis;
	std::map<Document, Document> kvMap;
	std::map<Document, std::set<DocumentId> > keyIdsetMap;

	boost::shared_ptr<QueryAttribute> gbAttribute;//group by A, B...
	boost::shared_ptr<QueryProjectionAbstract> resultQueryProjection;// avg(C), avg(D), max(D)...

	std::map<std::string, std::set<AggregateOperation> > getAggregattionMap();

public:
	GroupAggregationOperator(void);
	~GroupAggregationOperator(void);
	void execution();
	void setGroupByAttributeName(std::string groupByAttributeName);
	void setAggregationAttributeName(std::string aggregationAttributeName);

	void setResultQueryProjection(
			boost::shared_ptr<QueryProjectionAbstract> resultQueryProjection);
	void setGbAttribute(boost::shared_ptr<QueryAttribute> gbAttribute){
		this->gbAttribute = gbAttribute;
	}



};

