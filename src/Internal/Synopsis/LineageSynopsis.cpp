//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../../Common/stdafx.h"
#include "../../Internal/Synopsis/LineageSynopsis.h"
#include "../../Internal/Buffer/LineageBuffer.h"

LineageSynopsis::LineageSynopsis( int lineageNumber):lineageNumber(lineageNumber)
{
	this->buffer.reset(new LineageBuffer(lineageNumber));
}


LineageSynopsis::~LineageSynopsis(void)
{
}

void LineageSynopsis::insertLineage(Lineage& lineage,Element& outputElement)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@lineage synopsis begin@@@@@@@@@@"<<std::endl;
	std::cout<<"insert lineage : "<<lineage<<std::endl;
	std::cout<<"insert element : "<<outputElement<<std::endl;
#endif
	this->buffer->insertLineage(lineage,outputElement);
#ifdef DEBUG
	std::cout<<"@@@@@@@@lineage synopsis end@@@@@@@@@@"<<std::endl;

#endif
}
bool LineageSynopsis::getAndDeleteElement(Lineage& lineage, Element& outputElement)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@lineage synopsis begin@@@@@@@@@@"<<std::endl;
	std::cout<<"get by lineage : "<<lineage<<std::endl;
#endif
	bool bl =   this->buffer->getAndDeleteElement(lineage, outputElement);
#ifdef DEBUG
	std::cout<<" return :" <<bl<<std::endl;
	std::cout<<"get element : "<<outputElement<<std::endl;
	std::cout<<"@@@@@@@@lineage synopsis end@@@@@@@@@@"<<std::endl;

#endif
	return bl;
}
bool LineageSynopsis::isFull(void)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@lineage synopsis begin@@@@@@@@@@"<<std::endl;
#endif

	bool bl =  this->buffer->isFull();
#ifdef DEBUG
	if(bl== true)
	{
		std::cout<<"the synopsis is full"<<std::endl;
	}
	else
	{
		std::cout<<"the synopsis is not full"<<std::endl;
	}
	std::cout<<"@@@@@@@@lineage synopsis end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}
