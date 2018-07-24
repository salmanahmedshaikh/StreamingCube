//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * CubifyOperator.h
 *
 *  Created on: August 24, 2016
 *      Author: salman
 */

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <string>

#include "../Operator/CubifyOperator.h"
#include "../OLAP/OLAPManager.h"
#include "../OLAP/MaterializedLattice.h"

using namespace std;

int CubifyOperator::insertedTupleCounter = 0;
CubifyOperator::CubifyOperator()
{
}

CubifyOperator::CubifyOperator(std::string configFile, int queryID)
{
    //this->dimensions = ConfigureManager::getInstance()->getCubifyDimensions();
    //this->mVertices = ConfigureManager::getInstance()->getCubifyMVertices();
    //OLAPManager::getInstance()->getLatticeVertices("./configure/cubify.conf", this->latticeVertices);
    this->queryId = queryID;
    this->latticeVertices = OLAPManager::getInstance()->getLatticeVertices(configFile, queryID);
    this->timeGrain = OLAPManager::getInstance()->getConfigValue(this->queryId, "TimeGrain");
    this->IoA = atoi(OLAPManager::getInstance()->getConfigValue(this->queryId, "IoA").c_str());

    //std::cout << "configFile " << configFile << " | " << "queryID " << queryID << " | " << "timeGrain " << timeGrain << std::endl;
    MaterializedLattice* materializedLattice = new MaterializedLattice(this->latticeVertices);
    this->mLattice = boost::shared_ptr<MaterializedLattice>(materializedLattice);

    // materializing selected nodes
    mLattice->materializeSelectedNodes();

    //For checking latticeVertices
    /*
    for(latticeVerticesIt = latticeVertices.begin(); latticeVerticesIt != latticeVertices.end(); latticeVerticesIt++)
    {
        std::vector<std::string> vertexDims = latticeVerticesIt->second;

        if(latticeVerticesIt->first.isMaterialized)
            std::cout << "Materialized Node" << std::endl;
        else
            std::cout << "Non-Materialized Node" << std::endl;

        std::cout << latticeVerticesIt->first.vertexID << ", " << latticeVerticesIt->first.level << ", " << latticeVerticesIt->first.ordinal << std::endl;
        for(int i = 0; i < vertexDims.size(); i++)
            std::cout << vertexDims[i] << std::endl;

        std::cout << std::endl;
    }
    */
}

CubifyOperator::~CubifyOperator()
{
	// TODO Auto-generated destructor stub
}

void CubifyOperator::execution()
{
    #ifdef DEBUG
	std::cout<<"===================operator begin================="<<std::endl;
	std::cout<<"operatorid : "<<this->getId()<<std::endl;
    #endif

	assert(this->getInputQueueList().size()==1);
	assert(this->getOutputQueueList().size()==1);
	boost::shared_ptr<QueueEntity>inputQueue = this->getInputQueueList().front();
	boost::shared_ptr<QueueEntity>outputQueue = this->getOutputQueueList().front();

	//std::map<int, std::vector<unsigned int> > verticesToMaterialize;
	std::map<int, vertexKeyValue> materializedVertices;

	while(1)
	{
		if(outputQueue->isFull())
		{
			break;
		}
		if(inputQueue->isEmpty())
		{
			break;
		}

		TimeCounterUtility::getInstance()->start();
		Element inputElement;

		// Retrieve element from the queue of previous operator
		inputQueue->dequeue(inputElement);
		//std::cout << "Cubify Op: inputElement " << std::endl << inputElement << std::endl;
		Document& elementDocument = inputElement.document;

		DocumentIterator it(elementDocument);

		std::vector<DimensionKeyName> dimKeyNames;
		std::vector<DimensionKey> dimKeys;
		std::vector<DimensionKeyValue> dimKeyValues;
		std::vector<DimensionKeyValueName> dimKeyValueNames;

		double factValue;

        while(it.more())
        {
            std::string fieldName = it.next().fieldName();
            BinaryJsonElement elementValue = elementDocument.getField(fieldName.c_str());

            // If the keyword "Fact" found, extract the fact value
            if(fieldName.find("FACT") != std::string::npos)
            {
                if(elementValue.isInteger())
                {
                        int elementVal;
                        elementValue.Val(elementVal);
                        factValue = elementVal;

                        //groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
                        //std::cout << elementVal << std::endl;
                }
                else if(elementValue.isDouble())
                {
                        double elementVal;
                        elementValue.Val(elementVal);
                        factValue = elementVal;

                        //dimKeys.push_back(elementVal);
                        //groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
                        //std::cout << elementVal << std::endl;
                }
                else if(elementValue.isLong())
                {
                        long long elementVal;
                        elementValue.Val(elementVal);
                        factValue = elementVal;

                        //groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
                        //std::cout << elementVal << std::endl;
                }
                else if(elementValue.isNumber())
                {
                        long long elementVal;
                        elementValue.Val(elementVal);
                        factValue = elementVal;

                        //groupByAttributeStr = boost::lexical_cast<std::string>(elementVal);
                        //std::cout << elementVal << std::endl;
                }
                else if(elementValue.isNull())
                {
                        std::cout << "Fact value not found." << std::endl;
                        assert(false);
                        exit(0);

                }
                else
                {
                        std::cout << "Not a valid fact value." << std::endl;
                        assert(false);
                        exit(0);
                }

                //std::cout << fieldName << ": " << factValue << std::endl;
            }

            if(elementValue.isInteger())
            {
                    int elementVal;
                    elementValue.Val(elementVal);

                    dimKeyNames.push_back(fieldName);
                    dimKeys.push_back(elementVal);
                    //std::cout << fieldName << " : " << elementVal << std::endl;
            }
            else if(elementValue.isDouble())
            {
                    double elementVal;
                    elementValue.Val(elementVal);
            }
            else if(elementValue.isLong())
            {
                    long long elementVal;
                    elementValue.Val(elementVal);
            }
            else if(elementValue.isNumber())
            {
                    long long elementVal;
                    elementValue.Val(elementVal);
            }
            else if(elementValue.isStringType())
            {
                    std::string elementVal;
                    elementValue.Val(elementVal);

                    // Actual values corresponding to Dim keys. e.g., prodID = 125 = Dell Monitor (dimKeyNames = dimKeys = dimKeyValues)
                    dimKeyValueNames.push_back(fieldName);
                    dimKeyValues.push_back(elementVal);
                    //std::cout << fieldName << " : " << elementVal << std::endl;
            }
            else if(elementValue.isNull())
            {
                    std::cout << "Element value not found." << std::endl;
                    assert(false);

            }
            else
            {
                    std::cout << "Not a valid element value." << std::endl;
                    assert(false);
            }
        }

        Timestamp factTupleTimestamp = inputElement.timestamp;

        if(this->timeGrain == "Second" || this->timeGrain == "second")
            factTupleTimestamp = (factTupleTimestamp/1000000);
        else if(this->timeGrain == "Minute" || this->timeGrain == "minute")
            factTupleTimestamp = (factTupleTimestamp/(1000000*60));
        else if(this->timeGrain == "Hour" || this->timeGrain == "hour")
            factTupleTimestamp = (factTupleTimestamp/(1000000*60*60));

        latestElementTimestamp = factTupleTimestamp;

        //std::cout << "dimKeyNames.size(): " << dimKeyNames.size() << std::endl;
        //std::cout << "dimKeys.size(): " << dimKeys.size() << std::endl;
        //std::cout << "inputElement.timestamp: " << factTupleTimestamp << std::endl;
        //std::cout << "factValue: " << factValue << std::endl;
        //std::cout << "inputElement.mark: " << inputElement.mark << std::endl;

        // Clearing integratedPlusMinusElements
        integratedPlusMinusElements.clear();
        // integratedPlusMinusElements contain the generated plus minus elements
        mLattice->insertIntoLatticeNodes(dimKeyNames, dimKeys, dimKeyValueNames, dimKeyValues, factTupleTimestamp, factValue, inputElement.id, inputElement.mark, IoA, integratedPlusMinusElements);

        //if(integratedPlusMinusElements.size() > 0)
            //std::cout << "integratedPlusMinusElements " << integratedPlusMinusElements.size() << std::endl;

        insertedTupleCounter++;
        //std::cout << "Output from cubify op: " << elementsDocument << std::endl;
		TimeCounterUtility::getInstance()->pause();
	}

    #ifdef DEBUG
        std::cout<<"===================operator over================="<<std::endl;
    #endif
}

boost::shared_ptr<MaterializedLattice> CubifyOperator::getAssociatedMaterializedLattice()
{
    return this->mLattice;
}

void CubifyOperator::setQueryEntity( QueryEntity* queryEntity)
{
	this->queryEntity = queryEntity;
}

QueryEntity* CubifyOperator::getQueryEntity()
{
	return this->queryEntity;
}

Timestamp CubifyOperator::getLatestElementTimestamp()
{
    return latestElementTimestamp;
}
