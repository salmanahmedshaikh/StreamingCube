//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/ProjectionOperator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Scheduler/Scheduler.h"
#include "../Query/QueryProjectionAbstract.h"
ProjectionOperator::ProjectionOperator(boost::shared_ptr<QueryProjectionAbstract> queryProjection)
{
	int lineageNumber  = 1;
	this->lineageSynopsis.reset(new LineageSynopsis(lineageNumber));
	this->queryProjection = queryProjection;
}


ProjectionOperator::~ProjectionOperator(void)
{
}

void ProjectionOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();
	bool ret;

	//std::cout<<"===================Projection operator================="<<std::endl;

	while(1)
	{
		Element inputElement;
		if(outputQueue->isFull())
		{
			break;
		}
		if(inputQueue->isEmpty())
		{
			break;
		}
		TimeCounterUtility::getInstance()->start();
		inputQueue->dequeue(inputElement);

		if(inputElement.mark == PLUS_MARK)
		{
			// Produce new document
			Document newDocument;
			Document inputDocument = inputElement.document;
			//std::cout<<inputDocument<<std::endl;
			boost::any doc = this->queryProjection->performProjection(inputDocument);
			assert(doc.type() == typeid(Document));
			newDocument = boost::any_cast<Document>(doc);
//			newDocument.addFields(inputDocument,projectionAttributeSet);
			//std::cout<<newDocument<<std::endl;
			// Produce the output element
			Element outputElement;
			outputElement.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
			outputElement.mark = PLUS_MARK;
			outputElement.timestamp = inputElement.timestamp;
			outputElement.document = newDocument;
			outputElement.masterTag = inputElement.masterTag;
			//enqueue the output element
			outputQueue->enqueue(outputElement);

			//std::cout<<outputElement<<std::endl;
			//insert into the lineage synopsis
			if(lineageSynopsis->isFull())
			{
				throw std::runtime_error("synopsis is full");
			}

			Lineage lineage;
			lineage.lineageNumber = 1;
			lineage.lineageDocumentId[0] = inputElement.id;
			lineageSynopsis->insertLineage(lineage,outputElement);

		}
		else if(inputElement.mark  == MINUS_MARK)
		{
			Element elementInSynopsis;
			Lineage lineage;
			lineage.lineageNumber = 1;
			lineage.lineageDocumentId[0] = inputElement.id;
			ret = lineageSynopsis->getAndDeleteElement(lineage,elementInSynopsis);
			if(ret==false)
			{
				//lineage not found, the minus tuple has already been generated
				//we needn't generate the minus tuple again
				continue;
				//throw std::runtime_error("lineage not exist");
			}
			Element outputElement;
			outputElement.mark = MINUS_MARK;
			outputElement.document = elementInSynopsis.document;
			outputElement.timestamp = inputElement.timestamp;
			outputElement.id = elementInSynopsis.id;
			outputElement.masterTag = inputElement.masterTag;
			outputQueue->enqueue(outputElement);
		}
		TimeCounterUtility::getInstance()->pause();
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
