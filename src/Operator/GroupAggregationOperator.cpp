//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Operator/GroupAggregationOperator.h"
#include "../Internal/Synopsis/LineageSynopsis.h"
#include "../Internal/Queue/QueueEntity.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Internal/Synopsis/RelationSynopsis.h"
#include "../Internal/Element/ElementIterator.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Scheduler/Scheduler.h"

GroupAggregationOperator::GroupAggregationOperator(void) {
	this->relationSynopsis.reset(new RelationSynopsis());
}

GroupAggregationOperator::~GroupAggregationOperator(void) {
}

void GroupAggregationOperator::setResultQueryProjection(boost::shared_ptr<
		QueryProjectionAbstract> resultQueryProjection) {
	this->resultQueryProjection = resultQueryProjection;
}

std::map<std::string, std::set<AggregateOperation> > GroupAggregationOperator::getAggregattionMap() {

	//for aggregationAttributeName and operation
	std::map<std::string, std::set<AggregateOperation> > aggregattionMap;

	boost::shared_ptr<ObjectQueryProjection> objectQueryProjection =
			boost::static_pointer_cast<ObjectQueryProjection>(
					this->resultQueryProjection);

	std::vector<boost::shared_ptr<QueryProjectionAbstract> > fields =
			objectQueryProjection->getProjectionFields();

	vector<boost::shared_ptr<QueryProjectionAbstract> >::iterator iter =
			fields.begin();
	while (iter != fields.end()) {

		boost::shared_ptr<QueryProjectionAbstract> queryProjectionAbstract =
				*iter;
		boost::shared_ptr<DirectQueryProjection> directQueryProjection =
				boost::static_pointer_cast<DirectQueryProjection>(
						queryProjectionAbstract);
		boost::shared_ptr<QueryExpressionAbstract> expression =
				directQueryProjection->getExpression();
		boost::shared_ptr<AggregationQueryExpression>
				aggregationQueryExpression = boost::static_pointer_cast<
						AggregationQueryExpression>(expression);

		AttrSource attrSource =
				aggregationQueryExpression->getAttributeSource();

		switch (attrSource) {

		case _GROUP_KEY_VAR: {
			//no need
			//this->groupByAttributeName = aggregationAttributeName;
			break;
		}
		case _GROUP_ARRAY: {

			std::string
					aggregationAttributeName =
							boost::any_cast<std::string>(
									aggregationQueryExpression->getAggregationAttributeName());
			AggregateOperation aggregateOperation =
					aggregationQueryExpression->getOperation();

			//keyValueMap
			std::map<std::string, std::set<AggregateOperation> >::iterator
					iter = aggregattionMap.find(aggregationAttributeName);

			if (iter != aggregattionMap.end()) {
				std::set<AggregateOperation> oldAggOpSet = iter->second;
				oldAggOpSet.insert(aggregateOperation);
				iter->second = oldAggOpSet;
			} else {
				std::set<AggregateOperation> aggOpSet;
				aggOpSet.insert(aggregateOperation);
				aggregattionMap.insert(make_pair(aggregationAttributeName,
						aggOpSet));
			}

			break;
		}

		}

		iter++;
	}

	return aggregattionMap;
}

void GroupAggregationOperator::execution() {

#ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
#endif
	assert(this->getInputQueueList().size() == 1);
	assert(this->getOutputQueueList().size() == 1);
	boost::shared_ptr<QueueEntity> inputQueue =
			this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity> outputQueue =
			this->getOutputQueueList().front();

	std::map<std::string, std::set<AggregateOperation> > aggregattionMap =
			this->getAggregattionMap();

	while (1) {

		Element inputElement;
		if (outputQueue->isFull()) {
			break;
		}
		if (inputQueue->isEmpty()) {
			break;
		}
		inputQueue->dequeue(inputElement);

		//group
		Document inputDocument = inputElement.document;
		boost::any gbValueAny = gbAttribute->getValueDoc(inputDocument);
		Document gbValue = boost::any_cast<Document>(gbValueAny);

		//idsetMap
		std::set<DocumentId> currentGroupSet;
		std::map<Document, std::set<DocumentId> >::iterator it =
				keyIdsetMap.find(gbValue);

		if (inputElement.mark == PLUS_MARK) {
			this->relationSynopsis->insertElement(inputElement);
			if (it != keyIdsetMap.end()) { // group already exist
				std::set<DocumentId>& groupSet = it->second;
				groupSet.insert(inputElement.id);
				currentGroupSet = groupSet;
			} else { // group not exist
				std::set<DocumentId> groupSet;
				groupSet.insert(inputElement.id);
				keyIdsetMap.insert(make_pair(gbValue, groupSet));
				currentGroupSet = groupSet;
			}
		} else {
			this->relationSynopsis->deleteElement(inputElement);
			std::set<DocumentId>& groupSet = it->second;
			groupSet.erase(inputElement.id);

			if(groupSet.size() == 0){
				kvMap.erase(gbValue);
				keyIdsetMap.erase(it);
				break;
			}


			currentGroupSet = groupSet;
		}

		//keyValueMap
		std::map<std::string, std::set<AggregateOperation> >::iterator iter =
				aggregattionMap.begin();

		while (iter != aggregattionMap.end()) {
			std::string aggAttrName = iter->first;
			std::set<AggregateOperation> aggSet = iter->second;

			DocumentBuilder keyDocumentBuilder;
			keyDocumentBuilder.append("gbAttr", gbValue);
			keyDocumentBuilder.append("aggAttr", aggAttrName);
			Document keyDoc = keyDocumentBuilder.obj();
			std::map<Document, Document>::iterator it = kvMap.find(keyDoc);

			DocumentBuilder newValueDocumentBuilder;
			Document newValueDocument;

			if (it != kvMap.end()) { //exists

				Document valueDoc = it->second;
				double newValue = inputDocument.getField(aggAttrName).Number();
				DocumentId newId = inputElement.id;

				DocumentId oldMaxId = valueDoc.getField("maxId").Int();
				DocumentId oldMinId = valueDoc.getField("minId").Int();

				if (inputElement.mark == PLUS_MARK) {

					if (aggSet.find(SUM) != aggSet.end() || aggSet.find(AVG)
							!= aggSet.end() || aggSet.find(COUNT)
							!= aggSet.end()) {
						double newSum = valueDoc.getField("sum").Number()
								+ newValue;
						newValueDocumentBuilder.append("sum", newSum);
						int newCount = valueDoc.getField("count").Int() + 1;
						newValueDocumentBuilder.append("count", newCount);
					}
					if (aggSet.find(MAX) != aggSet.end() || aggSet.find(MIN)
							!= aggSet.end()) {

						Element oldMaxElement;
						this->relationSynopsis->findElementByElementId(
								oldMaxElement, oldMaxId);
						double oldMaxValue = oldMaxElement.document.getField(
								aggAttrName).Number();
						if (newValue > oldMaxValue) {
							newValueDocumentBuilder.append("maxId", newId);
						} else {
							newValueDocumentBuilder.append("maxId", oldMaxId);
						}

						Element oldMinElement;
						this->relationSynopsis->findElementByElementId(
								oldMinElement, oldMinId);
						double oldMinValue = oldMinElement.document.getField(
								aggAttrName).Number();
						if (newValue < oldMinValue) {
							newValueDocumentBuilder.append("minId", newId);
						} else {
							newValueDocumentBuilder.append("minId", oldMinId);
						}

					}

				} else {

					if (aggSet.find(SUM) != aggSet.end() || aggSet.find(AVG)
							!= aggSet.end() || aggSet.find(COUNT)
							!= aggSet.end()) {
						double newSum = valueDoc.getField("sum").Number()
								- newValue;
						newValueDocumentBuilder.append("sum", newSum);
						int newCount = valueDoc.getField("count").Int() - 1;
						newValueDocumentBuilder.append("count", newCount);

					}

					if (aggSet.find(MAX) != aggSet.end() || aggSet.find(MIN)
							!= aggSet.end()) {

						//if max or min element is removed
						if (oldMaxId == newId || oldMinId == newId) {

							boost::shared_ptr<SetElementIterator>
									elementIterator;
							elementIterator.reset(new SetElementIterator(
									this->relationSynopsis, currentGroupSet));
							elementIterator->initial();
							Element groupElement;
							elementIterator->getThis(groupElement);

							newValueDocumentBuilder.append("maxId",
									groupElement.id);
							newValueDocumentBuilder.append("minId",
									groupElement.id);

							double max = groupElement.document.getField(
									aggAttrName).Number();
							double min = max;
							while ((!outputQueue->isFull())
									&& elementIterator->getNext(groupElement)) {

								//assert( groupElement.document.getField(aggregationAttributeName).isNumber());

								double dl = groupElement.document.getField(
										aggAttrName).Number();

								if (max < dl) {
									newValueDocumentBuilder.append("maxId",
											groupElement.id);
								}
								if (min > dl) {
									newValueDocumentBuilder.append("minId",
											groupElement.id);
								}

							}

						} else {

							Element oldMaxElement;
							this->relationSynopsis->findElementByElementId(
									oldMaxElement, oldMaxId);
							double
									oldMaxValue =
											oldMaxElement.document.getField(
													aggAttrName).Number();
							if (newValue > oldMaxValue) {
								newValueDocumentBuilder.append("maxId", newId);
							} else {
								newValueDocumentBuilder.append("maxId",
										oldMaxId);
							}

							Element oldMinElement;
							this->relationSynopsis->findElementByElementId(
									oldMinElement, oldMinId);
							double
									oldMinValue =
											oldMinElement.document.getField(
													aggAttrName).Number();
							if (newValue < oldMinValue) {
								newValueDocumentBuilder.append("minId", newId);
							} else {
								newValueDocumentBuilder.append("minId",
										oldMinId);
							}

						}

					}

				}

				newValueDocument = newValueDocumentBuilder.obj();
				it->second = newValueDocument;

			} else { //not exists
				Document valueDoc;
				DocumentBuilder valueDocumentBuilder;
				if (aggSet.find(SUM) != aggSet.end() || aggSet.find(AVG)
						!= aggSet.end() || aggSet.find(COUNT) != aggSet.end()) {
					valueDocumentBuilder.append("sum", inputDocument.getField(
							aggAttrName).Number());
					valueDocumentBuilder.append("count", 1);
				}
				if (aggSet.find(MAX) != aggSet.end() || aggSet.find(MIN)
						!= aggSet.end()) {
					valueDocumentBuilder.append("maxId", inputElement.id);
					valueDocumentBuilder.append("minId", inputElement.id);
				}
				valueDoc = valueDocumentBuilder.obj();
				kvMap.insert(make_pair(keyDoc, valueDoc));

			}

			iter++;

		}

		Document newDocument;
		DocumentBuilder newDocumentBuilder;

		newDocumentBuilder.appendElements(gbValue.copy());

		iter = aggregattionMap.begin();
		while (iter != aggregattionMap.end()) {
			std::string aggAttrName = iter->first;
			std::set<AggregateOperation> aggOpSet = iter->second;

			DocumentBuilder keyDocumentBuilder;
			keyDocumentBuilder.append("gbAttr", gbValue);
			keyDocumentBuilder.append("aggAttr", aggAttrName);
			Document keyDoc = keyDocumentBuilder.obj();

			std::map<Document, Document>::iterator itt = kvMap.find(keyDoc);

			Document valDoc = itt->second;

			double sum, average, max, min;
			int count;
			if (valDoc.hasField("sum")) {
				sum = valDoc.getField("sum").Number();
				count = valDoc.getField("count").Int();
				average = sum / count;
			}
			if (valDoc.hasField("maxId")) {
				DocumentId maxId = valDoc.getField("maxId").Int();
				Element element;
				this->relationSynopsis->findElementByElementId(element, maxId);
				max = element.document.getField(aggAttrName).Number();
			}
			if (valDoc.hasField("minId")) {
				DocumentId minId = valDoc.getField("minId").Int();
				Element element;
				this->relationSynopsis->findElementByElementId(element, minId);
				min = element.document.getField(aggAttrName).Number();
			}

			std::set<AggregateOperation>::iterator it = aggOpSet.begin();
			while (it != aggOpSet.end()) {
				AggregateOperation aggregateOperation = *it;
				if (aggregateOperation == AVG) {
					newDocumentBuilder.appendNumber("avg(" + aggAttrName + ")",
							average);
				} else if (aggregateOperation == SUM) {
					newDocumentBuilder.appendNumber("sum(" + aggAttrName + ")",
							sum);
				} else if (aggregateOperation == COUNT) {
					newDocumentBuilder.appendNumber("count(" + aggAttrName
							+ ")", count);
				} else if (aggregateOperation == MIN) {
					newDocumentBuilder.appendNumber("min(" + aggAttrName + ")",
							min);
				} else if (aggregateOperation == MAX) {
					newDocumentBuilder.appendNumber("max(" + aggAttrName + ")",
							max);
				}
				it++;
			}
			iter++;

		}

		newDocument = newDocumentBuilder.obj();


		//2ND DO PROJECTION
		boost::any retDocument =
				this->resultQueryProjection->performProjection(newDocument);
		newDocument = boost::any_cast<Document>(retDocument);

		Element newElement;
		newElement.id
				= DocumentIdentifierGenerator::generateNewDocumentIdentifier();
		newElement.document = newDocument;
		newElement.mark = PLUS_MARK;
		newElement.timestamp = inputElement.timestamp;
		newElement.masterTag = inputElement.masterTag;

		//generate new output to the output queue
		if (outputQueue->isFull()) {
			throw std::runtime_error("output queue is full");
		}

		outputQueue->enqueue(newElement);
		cout << "newDocument: " << newDocument << endl;


	}
#ifdef DEBUG
	std::cout<<"===================operator over================="<<std::endl;
#endif
}
