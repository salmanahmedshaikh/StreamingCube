//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/noncopyable.hpp>
#include "../Query/QueryConditionAbstract.h"
#include "../Query/QueryExpressionAbstract.h"
#include "../Query/QueryProjectionAbstract.h"
#include "../Query/QueryAttribute.h"
#include "../Common/Types.h"
#include <boost/any.hpp>


// help generating query instances by query intermediate representation
class QueryUtility: public boost::noncopyable
{
public:
	QueryUtility(void);
	~QueryUtility(void);
	static boost::shared_ptr<QueryConditionAbstract> resolveQueryCondition(Document& document);
	static boost::shared_ptr<QueryExpressionAbstract> resolveQueryExpression(Document& document);
	static boost::shared_ptr<QueryProjectionAbstract> resolveQueryProjection(Document& document);
	static boost::shared_ptr<QueryAttribute> resolveQueryAttribute(Document& document);
	static bool compareEqual(boost::any leftValue, boost::any rightValue);
	static TimeDuration getDuration(std::string durationSpecificaiton);
};

