//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include "../Query/QueryEntity.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include <boost/shared_ptr.hpp>
#include <list>
class GateOperator: public Operator
{
private:
	static int gateOperatorNumber;
	std::list<QueryEntity* > queryEntityList;
	boost::shared_ptr<RelationSynopsis> relationSynopsis;
public:
	static int deletedNumber;
	static int totalPlusNumber;
	GateOperator();
	~GateOperator();
	void execution();
	void addQueryEntity(QueryEntity* queryEntity);


};

