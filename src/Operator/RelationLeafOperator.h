//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * RelationOperator.h
 *
 *  Created on: May 21, 2015
 *      Author: root
 */

#ifndef SRC_OPERATOR_RELATIONLEAFOPERATOR_H_
#define SRC_OPERATOR_RELATIONLEAFOPERATOR_H_

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/thread.hpp>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Element/ElementIterator.h"
#include "../IO/RelationInput.h"
#include "Operator.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Query/QueryExpressionAbstract.h"
#include "../Query/QueryProjectionAbstract.h"

class RelationLeafOperator: public Operator {

private:

	boost::shared_ptr<RelationInput> relationInput;

	boost::shared_ptr<RelationSynopsis> relationSynopsis;
	boost::shared_ptr<LineageSynopsis> lineageSynopsis;

	boost::shared_ptr<ElementIterator> elementIterator;

	boost::thread m_Thread;

	void updateSynopsis(std::vector<std::string> vecSegTag);
	void insertSynopsis(std::vector<std::string> vecSegTag);
	void deleteSynopsis(std::vector<std::string> vecSegTag);

public:
	RelationLeafOperator();
	virtual ~RelationLeafOperator();
	void execution();
	void outputNewcomeElements(Element oldElement, Element newElement);
	void parseAndExecuteSQLStatement(std::string sql);
	void start() {
		m_Thread = boost::thread(&RelationLeafOperator::execution, this);
	}

	void join() {
		m_Thread.join();
	}

	const boost::shared_ptr<RelationInput>& getRelationInput() const {
		return relationInput;
	}

	void setRelationInput(
			const boost::shared_ptr<RelationInput>& relationInput) {
		this->relationInput = relationInput;
	}

	void testSynopsis() {

		this->elementIterator->initial();
		Element element;
		while (this->elementIterator->getNext(element)) {
			Document document = element.document;
			cout << document << endl;
		}

	}
};

#endif /* SRC_OPERATOR_RELATIONLEAFOPERATOR_H_ */
