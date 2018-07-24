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
#include "../Query/QueryExpressionAbstract.h"
#include "../Query/QueryProjectionAbstract.h"

class JoinOperator: public Operator
{
private:

	boost::shared_ptr<RelationSynopsis>leftRelationSynopsis;
	boost::shared_ptr<RelationSynopsis>rightRelationSynopsis;
	boost::shared_ptr<LineageSynopsis>lineageSynopsis;
	//Timestamp lastLeftTimestamp;
	//Timestamp lastRightTimestamp;
	boost::shared_ptr<ElementIterator> leftElementIterator;
	boost::shared_ptr<ElementIterator> rightElementIterator;

	boost::shared_ptr<QueryAttribute> leftJoinAttribute;
	boost::shared_ptr<QueryAttribute> rightJoinAttribute;
	bool leftOuter;
	bool rightOuter;
	boost::shared_ptr<QueryProjectionAbstract> resultQueryProjection;

	int outputTuplesCount;

public:
	JoinOperator(void);
	~JoinOperator(void);

	void execution();
	void processLeftPlus (Element& leftElement);
	void processLeftMinus (Element& leftElement);
	void processRightPlus (Element& rightElement);
	void processRightMinus (Element& rightElement);
	void setLeftJoinAttribute(boost::shared_ptr<QueryAttribute>  leftJoinAttribute);
	void setRightJoinAttribute(boost::shared_ptr<QueryAttribute>  rightJoinAttribute);
	void setLeftOuter(bool bl);
	void setRightOuter(bool bl);
	void setResultQueryProjection(boost::shared_ptr<QueryProjectionAbstract> resultQueryProjection);
};

