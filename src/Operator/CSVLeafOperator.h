//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * CSVInput.h
 *
 *  Created on: August 2, 2016
 *      Author: salman
 */

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Element/ElementIterator.h"
#include "../IO/RelationInput.h"
#include "../Operator/Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Query/QueryExpressionAbstract.h"
#include "../Query/QueryProjectionAbstract.h"
#include "../IO/CSVInput.h"

class CSVLeafOperator: public Operator
{

private:
    std::ifstream csvFileStream;
    std::string csvFileName;
    bool isCSVLeafExecuted;
    boost::shared_ptr<CSVInput> csvInput;

public:
	CSVLeafOperator();
	virtual ~CSVLeafOperator();

	void setCSVInput(boost::shared_ptr<CSVInput> csvInput);
	void execution();
	bool getIsCSVLeafExecuted();

};
