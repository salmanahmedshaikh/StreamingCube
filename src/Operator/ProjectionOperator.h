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
#include "../Operator/Operator.h"
#include "../Query/QueryProjectionAbstract.h"
#include <boost/shared_ptr.hpp>
class ProjectionOperator:public Operator
{
private:
	boost::shared_ptr<QueryProjectionAbstract> queryProjection;
//	std::set<std::string> projectionAttributeSet;
	boost::shared_ptr<LineageSynopsis>lineageSynopsis;
public:

	ProjectionOperator(boost::shared_ptr<QueryProjectionAbstract> queryProjection);
	~ProjectionOperator(void);

	void execution();

};

