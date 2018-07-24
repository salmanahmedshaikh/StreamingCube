//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../../Common/stdafx.h"
#include "../../Internal/Synopsis/WindowSynopsis.h"
#include "../../Internal/Buffer/QueueBuffer.h"


WindowSynopsis::WindowSynopsis()
{
	this->buffer.reset(new QueueBuffer());

}


WindowSynopsis::~WindowSynopsis(void)
{
}
void WindowSynopsis::insertElement(Element& element)
{

#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis begin@@@@@@@@@@"<<std::endl;
	std::cout<<"insert element : "<<element<<std::endl;
#endif
	this->buffer->enqueue(element);
#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis end@@@@@@@@@@"<<std::endl;
#endif
}

void WindowSynopsis::deleteOldestElement(void)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis begin@@@@@@@@@@"<<std::endl;
	std::cout<<"delete oldest element"<<std::endl;
#endif
	this->buffer->dequeue();
#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis end@@@@@@@@@@"<<std::endl;
#endif
}
void WindowSynopsis::getOldestElement(Element& element)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis begin@@@@@@@@@@"<<std::endl;
#endif
	this->buffer->peek(element);
#ifdef DEBUG
	std::cout<<"pick element : "<<element<<std::endl;
	std::cout<<"@@@@@@@@window synopsis end@@@@@@@@@@"<<std::endl;
#endif
}
bool WindowSynopsis::isFull(void)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis begin@@@@@@@@@@"<<std::endl;
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
	std::cout<<"@@@@@@@@window synopsis end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}
bool WindowSynopsis::isEmpty(void)
{
#ifdef DEBUG
	std::cout<<"@@@@@@@@window synopsis begin@@@@@@@@@@"<<std::endl;
#endif
	bool bl =  this->buffer->isEmpty();
#ifdef DEBUG
	if(bl== true)
	{
		std::cout<<"the synopsis is empty"<<std::endl;
	}
	else
	{
		std::cout<<"the synopsis is not empty"<<std::endl;
	}
	std::cout<<"@@@@@@@@window synopsis end@@@@@@@@@@"<<std::endl;
#endif
	return bl;
}
void WindowSynopsis::peekNewestElementList(std::list<Element>& elementList, int number)
{
	this->buffer->peekNewestElementList(elementList , number);
}
void WindowSynopsis::peekAllElements(std::list<Element>&elementList)
{
	this->buffer->peekAllElements(elementList);
}
int WindowSynopsis::getElementNumber()
{
	return this->buffer->getElementNumber();
}
