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
#include "../IO/IStreamOutput.h"
#include "../Query/QueryEntity.h"
#include <boost/shared_ptr.hpp>

class RootOperator:public Operator
{
private:
	boost::shared_ptr<IStreamOutput>streamOutput;
	QueryEntity* queryEntity;
public:
	RootOperator(void);
	~RootOperator(void);

	static int rootOutputSize;

	void execution();

	void setStreamOutput(boost::shared_ptr<IStreamOutput> streamOutput);
	void setQueryEntity(QueryEntity* queryEntity);
	QueryEntity* getQueryEntity();
};

