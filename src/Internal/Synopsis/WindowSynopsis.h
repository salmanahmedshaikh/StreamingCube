//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Internal/Element/Element.h"
#include "../../Internal/Buffer/QueueBuffer.h"
#include "../../Internal/Synopsis/Synopsis.h"
class WindowSynopsis:public Synopsis
{
private:
	boost::shared_ptr<QueueBuffer>buffer;

public:
	WindowSynopsis(void);
	virtual ~WindowSynopsis(void);
	void insertElement(Element& element);
	void getOldestElement(Element& element);
	void peekNewestElementList(std::list<Element>& elementList, int number);
	void peekAllElements(std::list<Element>&elementList);
	void deleteOldestElement(void);
	bool isFull(void);
	bool isEmpty(void);
	int getElementNumber();
};

