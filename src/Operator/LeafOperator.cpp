//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/LeafOperator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"
#include "../Utility/TimestampGenerator.h"
#include "../Query/QueryEntity.h"
#include "../Command/CommandManager.h"
#include "../Utility/MyStatus.h"
#include "../Utility/StreamDistribution.h"
#include "../Schema/SchemaInterpreter.h"
#include <boost/lexical_cast.hpp>

int LeafOperator::totalInputNumber = 0;
LeafOperator::LeafOperator(void)
{
	this->isMasterStream = false;
}

LeafOperator::~LeafOperator(void)
{
}

bool LeafOperator::isMaster(void)
{
	return this->isMasterStream;
}

void LeafOperator::setStreamInput(boost::shared_ptr<IStreamInput> streamInput)
{
	this->streamInput = streamInput;
	//std::cout << "streamInput in Leaf constructor " << streamInput << std::endl;
	this->streamInput->start();
}

void LeafOperator::execution()
{
#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	for(int i = 0 ; i < 25 ;i++)
	{
        //std::cout << "streamInput in Leaf " << streamInput << std::endl;
		if(streamInput->isEmpty())
		{
            //std::cout << "streamInput is Empty" << std::endl;
            //usleep(1);
			//return;
			continue;
		}
		totalInputNumber++;

		//std::cout << totalInputNumber << std::endl;
		Element element;

		bool bl = streamInput->getAndCheckNextElement(element);
		element.masterTag = false;

		//element.timestamp = TimestampGenerator::getCurrentTime();

		if(bl == true)
		{
			std::list<QueryEntity* >::iterator i;
			for(i = this->queryList.begin(); i!= this->queryList.end(); i++)
			{
				QueryEntity* queryEntity = *i;
				// If this leaf operator belongs to master stream, arrival of any tuple from this stream activates the query
				bool masterTag = queryEntity->getMasterTag(this);
				// Yes, this leaf operator belongs to master stream
				if(masterTag == true)
				{
					element.masterTag = true;
					queryEntity->setActive(element.timestamp);
				}
				// No, this leaf operator does not belong to master stream
				else
				{
					bool isActive = queryEntity->isActive(element.timestamp);
					if(isActive == true)
					{
						element.masterTag = true;
					}
					else
					{
						if(element.masterTag!= true)
						{
							element.masterTag = false;
						}
					}

				}
			}
			//std::cout << "Leaf Op:" << std::endl << element << std::endl;
			output(element);
		}
		else
		{
#ifdef DEBUG
			std::cout<<"the input element doesn't satisfied the schema"<<std::endl;
			std::cout<<"input element : "<<std::endl;
			std::cout<<element<<std::endl;
			std::cout<<"input schema : "<<std::endl;
			std::cout<<*(streamInput->getSchema())<<std::endl;
#endif
		}
	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}

boost::shared_ptr<IStreamInput> LeafOperator::getStreamInput(void)
{
	return this->streamInput;
}

void LeafOperator::addQuery( QueryEntity* queryEntity)
{
	this->queryList.push_back(queryEntity);

//	this->tempQueryEntity = queryEntity;
//	this->tempMasterTag = queryEntity->getMasterTag(this);
//	std::cout<<"3333333333333333"<<std::endl;
//	std::cout<<this<<std::endl;
//	std::cout<<tempMasterTag<<std::endl;
}

std::list<QueryEntity*> LeafOperator::getRelatedQueries()
{
	return this->queryList;
}
