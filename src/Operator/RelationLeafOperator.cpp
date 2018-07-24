//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

/*
 * RelationOperator.cpp
 *
 *  Created on: May 21, 2015
 *      Author: root
 */

#include "RelationLeafOperator.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <string>

#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "../BinaryJson/json.h"

#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Query/QueryUtility.h"

using namespace std;

RelationLeafOperator::RelationLeafOperator()
{
	//int lineageNumber = 1;
	//this->lineageSynopsis.reset(new LineageSynopsis(lineageNumber));
	this->relationSynopsis.reset(new RelationSynopsis());
	this->elementIterator.reset(new ElementIterator(this->relationSynopsis));

}

RelationLeafOperator::~RelationLeafOperator()
{
	// TODO Auto-generated destructor stub
}

struct event_base *base;

void run(RelationLeafOperator * relationLeafOperator);
void read_callback_update(struct bufferevent *bev, void *ctx);
void errorcb(struct bufferevent *bev, short error, void *ctx);
void do_accept(evutil_socket_t listener, short event, void *arg);
Element documentToElement(Document doc, char mark);

void RelationLeafOperator::execution()
{
	//output all elements once
	boost::shared_ptr<QueueEntity> outputQueue =
			this->getOutputQueueList().front();
	list<Element> elements = relationInput->getAllElementsFromDatabase();
	list<Element>::iterator it;
	it = elements.begin();
	while (it != elements.end()) {
		Element element = *it;
		//dont't treat relation as stream.
		//elements in this queue's top joining with join's another side's synopsis is of no meaning? NO...
		relationSynopsis->insertElement(element);
		outputQueue->enqueue(element);
		it++;
	}

	cout << "old Synopsis: " << endl;

	//then listening to the changes in the rdbms
	setvbuf(stdout, NULL, _IONBF, 0);
	run(this);
}

void read_callback_update(struct bufferevent *bev, void *ctx)
{
	RelationLeafOperator * thisOp = (RelationLeafOperator *) ctx;
	char tmp[128];
	size_t n;
	int i;
	string data;
	while (1) {
		n = bufferevent_read(bev, tmp, sizeof(tmp));

		if (n <= 0)
			break; /* No more data. */
		for (i = 0; i < n; ++i) {
			data += tmp[i];
		}

		cout << "sql: " << data << endl;

	}
	thisOp->parseAndExecuteSQLStatement(data);

	/*cout << "new Synopsis: " << endl;

	 thisOp->testSynopsis();*/

}

Element documentToElement(Document doc, char mark) {

	Element newElement;
	newElement.id =
			DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	newElement.document = doc;
	newElement.mark = mark;
	newElement.timestamp = TimestampGenerator::getCurrentTime();
	newElement.document.getOwned();

	return newElement;
}

//todo parse SQL to document, maybe use antlr.

void RelationLeafOperator::parseAndExecuteSQLStatement(std::string sql) {

	DocumentBuilder documentBuilder;

	boost::char_separator<char> sep(", ()");
	typedef boost::tokenizer<boost::char_separator<char> > CustonTokenizer;
	CustonTokenizer tok(sql, sep);
	std::vector<std::string> vecSegTag;
	for (CustonTokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
		vecSegTag.push_back(*beg);
	}

	if (vecSegTag[0] == "update") {
		updateSynopsis(vecSegTag);
	} else if (vecSegTag[0] == "insert") {
		insertSynopsis(vecSegTag);
	} else if (vecSegTag[0] == "delete") {
		deleteSynopsis(vecSegTag);
	}

}

//todo handle with different types. and give it a batch fashion.
void RelationLeafOperator::updateSynopsis(std::vector<std::string> vecSegTag) {
	//set
	std::string attr = vecSegTag[3];
	int newValue = boost::lexical_cast<int>(vecSegTag[5]);
	//where
	std::string condi = vecSegTag[7];
	int condiValue = boost::lexical_cast<int>(vecSegTag[9]);

	boost::shared_ptr<QueueEntity> outputQueue =
			this->getOutputQueueList().front();
	if (this->relationSynopsis->isFull()) {
		throw runtime_error("right synopsis is full");
	}

	this->elementIterator->initial();
	Element element;
	while ((!outputQueue->isFull()) && this->elementIterator->getNext(element)) {
		Document document = element.document;
		int value = document.getField(condi).Number();
		if (value == condiValue) {
			//update synopsis by remove and insert.
			//remove
			this->relationSynopsis->deleteElement(element);
			element.mark = MINUS_MARK;
			element.timestamp = TimestampGenerator::getCurrentTime();
			outputQueue->enqueue(element);
			//insert
			DocumentBuilder documentBuilder;
			DocumentIterator it(document);
			documentBuilder.append(attr, newValue);
			while (it.more()) {
				DocumentElement documentElement = it.next();
				std::string fieldName = documentElement.fieldName();
				//skip new attribute
				if (fieldName != attr) {
					documentBuilder.append(document.getField(fieldName));
				}
			}
			Element newElement;
			newElement.id =
					DocumentIdentifierGenerator::generateNewDocumentIdentifier();
			newElement.mark = PLUS_MARK;
			newElement.timestamp = element.timestamp;
			newElement.document = documentBuilder.obj();
			newElement.document.getOwned();

			this->relationSynopsis->insertElement(newElement);
			outputQueue->enqueue(newElement);
		}

	}

}
void RelationLeafOperator::insertSynopsis(std::vector<std::string> vecSegTag) {
	//table(a,b) values(1,2)......
	std::string attr1 = vecSegTag[3];
	std::string attr2 = vecSegTag[4];
	int newValue1 = boost::lexical_cast<int>(vecSegTag[6]);
	int newValue2 = boost::lexical_cast<int>(vecSegTag[7]);

	boost::shared_ptr<QueueEntity> outputQueue =
			this->getOutputQueueList().front();
	if (this->relationSynopsis->isFull()) {
		throw runtime_error("right synopsis is full");
	}

	this->elementIterator->initial();
	//insert
	DocumentBuilder documentBuilder;
	documentBuilder.append(attr1, newValue1);
	documentBuilder.append(attr2, newValue2);
	Element newElement;
	newElement.id =
			DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	newElement.mark = PLUS_MARK;
	newElement.timestamp = TimestampGenerator::getCurrentTime();
	newElement.document = documentBuilder.obj();
	newElement.document.getOwned();

	this->relationSynopsis->insertElement(newElement);
	outputQueue->enqueue(newElement);

}
void RelationLeafOperator::deleteSynopsis(std::vector<std::string> vecSegTag) {
//where
	std::string condi = vecSegTag[4];
	int condiValue = boost::lexical_cast<int>(vecSegTag[6]);

	boost::shared_ptr<QueueEntity> outputQueue =
			this->getOutputQueueList().front();
	if (this->relationSynopsis->isFull()) {
		throw runtime_error("right synopsis is full");
	}

	this->elementIterator->initial();
	Element element;
	while ((!outputQueue->isFull()) && this->elementIterator->getNext(element)) {
		Document document = element.document;
		int value = document.getField(condi).Number();
		if (value == condiValue) {
			//remove
			this->relationSynopsis->deleteElement(element);
			element.mark = MINUS_MARK;
			element.timestamp = TimestampGenerator::getCurrentTime();
			outputQueue->enqueue(element);
		}

	}
}

void errorcb(struct bufferevent *bev, short error, void *ctx) {
	if (error & BEV_EVENT_EOF) {
		/* connection has been closed, do any clean up here */
		printf("connection closed\n");
	} else if (error & BEV_EVENT_ERROR) {
		/* check errno to see what error occurred */
		printf("some other error\n");
	} else if (error & BEV_EVENT_TIMEOUT) {
		/* must be a timeout event handle, handle it */
		printf("Timed out\n");
	}
	bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener, short event, void *arg) {
	RelationLeafOperator * thisOp = (RelationLeafOperator *) arg;
	struct sockaddr_storage ss;
	socklen_t slen = sizeof(ss);
	int fd = accept(listener, (struct sockaddr*) &ss, &slen);
	if (fd < 0) {
		perror("accept");
	} else if (fd > FD_SETSIZE) {
		close(fd);
	} else {

		struct bufferevent *bev;
		evutil_make_socket_nonblocking(fd);

		bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

		bufferevent_setcb(bev, read_callback_update, NULL, errorcb, thisOp);
		bufferevent_enable(bev, EV_READ | EV_WRITE);
	}
}

void run(RelationLeafOperator * relationLeafOperator) {

	cout << "listening starts" << endl;

	evutil_socket_t listener;
	struct sockaddr_in sin;

	struct event *listener_event;

	base = event_base_new();
	if (!base)
		return; /*XXXerr*/

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(0x7f000001);
	sin.sin_port = htons(23456);

	listener = socket(AF_INET, SOCK_STREAM, 0);
	evutil_make_socket_nonblocking(listener);

	if (bind(listener, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
		perror("bind");
		return;
	}
	if (listen(listener, 16) < 0) {
		perror("listen");
		return;
	}

	listener_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept,
			(void*) relationLeafOperator);
	/*XXX check it */
	event_add(listener_event, NULL);

	event_base_dispatch(base);
}
