//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../../Common/stdafx.h"
#include "../../Common/Types.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <fstream>
#include "../../Internal/Buffer/BufferConstraint.h"

#define PLUS_MARK '+'
#define MINUS_MARK '-'

/*
  one element contains four parts
  (1) timestamp of the document
  (2) document identifier
  (3) mark which may be PLUS_MARK or MINUS_MARK
  (4) document itself
 */

static char global_buffer[CHUNK_SIZE];
struct Element
{
private:
	int elementSize;

public:

	Timestamp timestamp;
	DocumentId id;
	Mark mark;
	Document document;
	MasterTag masterTag;
	Element()
	{
		this->elementSize = -1;
		this->masterTag = false;
		this->timestamp = -1;
	}
	Element(Timestamp& timestamp, DocumentId& id,Mark& mark,MasterTag& masterTag, Document& document)
	{
 		assert(mark==PLUS_MARK||mark==MINUS_MARK);
		this->timestamp = timestamp;
		this->id = id;
		this->mark =mark;
		this->masterTag = masterTag;
		this->document = document;
		this->elementSize = TIMESTAMP_SIZE + DOCUMENT_IDENTIFIER_SIZE + MARK_SIZE + MASTER_TAG_SIZE + document.objsize();
	}
	int getSize(void)
	{
		if(elementSize==-1)
		{
			this->elementSize = TIMESTAMP_SIZE + DOCUMENT_IDENTIFIER_SIZE + MARK_SIZE + MASTER_TAG_SIZE + document.objsize();

		}
		return elementSize;
	}
	friend ostream& operator<<(ostream&s,const Element&e)
	{
		s<<"element start"<<endl;
		s<<"elememt timestamp : "<<e.timestamp<<endl;
		s<<"element id        : "<<e.id<<endl;
		s<<"element mark      : "<<e.mark<<endl;
		s<<"element master tag: "<<e.masterTag<<endl;
		s<<"element document  : "<<e.document<<endl;
		s<<"element end"<<endl;
		return s;
	}
	//serialization
	friend istream& operator>>(istream&s,  Element&e)
	{
		int p = 0;
		s.read(global_buffer + p,TIMESTAMP_SIZE);
		e.timestamp = *(Timestamp*)(global_buffer+p);
		p += TIMESTAMP_SIZE;

		s.read(global_buffer + p,DOCUMENT_IDENTIFIER_SIZE);
		e.id = *(DocumentId*)(global_buffer+p);
		p += DOCUMENT_IDENTIFIER_SIZE;

		s.read(global_buffer + p,MARK_SIZE);
		e.mark = *(Mark*)(global_buffer + p);
		p += MARK_SIZE;

		s.read(global_buffer + p, MASTER_TAG_SIZE);
		e.masterTag = *(MasterTag*)(global_buffer + p);
		p += MASTER_TAG_SIZE;

		s.read(global_buffer + p, sizeof(int)); //document size;
		int documentSize = *(int*)(global_buffer + p);
		char* documentPosition = global_buffer + p;
		p += sizeof(int);

		s.read(global_buffer + p, documentSize - sizeof(int));

		Document document (documentPosition);
		e.document = document;
		e.document.getOwned();


		return s;
	}
};

