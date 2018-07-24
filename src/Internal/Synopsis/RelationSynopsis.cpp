//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../../Common/stdafx.h"
#include "../../Internal/Synopsis/RelationSynopsis.h"
#include "../../Internal/Buffer/RelationBuffer.h"

RelationSynopsis::RelationSynopsis()
{
	this->buffer.reset(new RelationBuffer());
}

RelationSynopsis::~RelationSynopsis(void)
{
}
void RelationSynopsis::insertElement(Element& element)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@relation synopsis begin@@@@@@@@@@"<<std::endl;
	std::cout<<"insert element : "<<element<<std::endl;
#endif
	 this->buffer->insertElement(element);
#ifdef DEBUG
	std::cout<<"@@@@@@@@relation synopsis end@@@@@@@@@@"<<std::endl;
#endif
}
bool RelationSynopsis::deleteElement(Element& element)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@relation synopsis begin@@@@@@@@@@"<<std::endl;
	std::cout<<"delete element : "<<element<<std::endl;
#endif
	bool bl = this->buffer->deleteElement(element);
#ifdef DEBUG
	std::cout<<"@@@@@@@@relation synopsis end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}

bool RelationSynopsis::isFull(void)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@relation synopsis begin@@@@@@@@@@"<<std::endl;
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
	std::cout<<"@@@@@@@@relation synopsis end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}
void RelationSynopsis::clear(void)
{
	this->buffer->clear();
}

void RelationSynopsis::findElementByElementId(Element& element, DocumentId id){

	this->buffer->findElementByElementId(element, id);
}

