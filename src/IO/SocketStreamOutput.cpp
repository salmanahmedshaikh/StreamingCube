//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/SocketStreamOutput.h"
#include "../IO/IOManager.h"
#include "../Internal/Buffer/BufferConstraint.h"
#include "../Internal/Element/Element.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Utility/MyStatus.h"
#include <boost/lexical_cast.hpp>

static char socketbuffer[CHUNK_SIZE];
int SocketStreamOutput::totalOutputElementsCounter = 0;
bool SocketStreamOutput::isFirstElement = false;
bool SocketStreamOutput::isLastElement = false;

SocketStreamOutput::SocketStreamOutput(std::string ip, std::string port)
{
	this->ip = ip;
	this->port = port;
	isFirstElement =  false;
	isLastElement = false;
	this->isDispatcher = false;
	isDispatcherStarterElementSent = true;
}

SocketStreamOutput::SocketStreamOutput(std::string ip,std::string port, std::string streamSource)
{
    this->ip = ip;
	this->port = port;
	this->streamSource = streamSource;
	this->isDispatcher = false;
	isDispatcherStarterElementSent = false;
	isFirstElement =  false;
	isLastElement = false;
}

SocketStreamOutput::SocketStreamOutput(std::string ip,std::string port, std::string streamSource, bool isDispatcher)
{
    this->ip = ip;
	this->port = port;
	this->streamSource = streamSource;
	this->isDispatcher = isDispatcher;
	isDispatcherStarterElementSent = false;
	isFirstElement =  false;
	isLastElement = false;
}

SocketStreamOutput::~SocketStreamOutput(void)
{
    isFirstElement = true;
}

void SocketStreamOutput::createCommandElement(Element& outputCommandElement, std::string elementType)
{
        DocumentBuilder outputCommandDocumentBuilder;

        if(elementType == "dispatcher_starter_document")
        {
            //std::cout << "Dispatcher Starter Document Generated" << std::endl;
            //std::cout << "Dispatcher starter document Stream source from SocketStreamOutput " << this->streamSource << std::endl;

            outputCommandDocumentBuilder.append("document_type", elementType);
            outputCommandDocumentBuilder.append("stream_source", this->streamSource);
            //outputCommandDocumentBuilder.append("query_id", boost::lexical_cast<std::string>(MyStatus::getInstance()->getQueryID()));

            Document outputCommandDocument = outputCommandDocumentBuilder.obj();

            outputCommandElement.id = 999;
            outputCommandElement.document = outputCommandDocument;
            outputCommandElement.mark = PLUS_MARK;
            outputCommandElement.timestamp = TimestampGenerator::getCurrentTime();
            outputCommandElement.masterTag = false;
        }
        else if (elementType == "output_starter_document" || elementType == "output_ender_document")
        {
            outputCommandDocumentBuilder.append("document_type", elementType);
            outputCommandDocumentBuilder.append("worker_id", boost::lexical_cast<std::string>(MyStatus::getInstance()->getWorkerID()));
            outputCommandDocumentBuilder.append("query_id", boost::lexical_cast<std::string>(MyStatus::getInstance()->getQueryID()));
            outputCommandDocumentBuilder.append("active_worker_seq_num", boost::lexical_cast<std::string>(MyStatus::getInstance()->getActiveWorkerSeqNum()));
            outputCommandDocumentBuilder.append("total_active_workers", boost::lexical_cast<std::string>(MyStatus::getInstance()->getTotalActiveWorkers()));

            Document outputCommandDocument = outputCommandDocumentBuilder.obj();

            //outputCommandElement.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
            outputCommandElement.id = 999;
            outputCommandElement.document = outputCommandDocument;
            outputCommandElement.mark = PLUS_MARK;
            outputCommandElement.timestamp = TimestampGenerator::getCurrentTime();
            outputCommandElement.masterTag = false;
        }
}

void SocketStreamOutput::pushNext(Element& element)
{
    if(!isDispatcherStarterElementSent)
    {
        isDispatcherStarterElementSent = true;
        Element dispatcherStarterElement;
        this->createCommandElement(dispatcherStarterElement, "dispatcher_starter_document");
        this->pushNext(dispatcherStarterElement);
        //std::cout << dispatcherStarterElement << std::endl;
        //std::cout << "Dispatcher Starter Element Sent" << std::endl;
    }

    if(isFirstElement)
    {
        isFirstElement = false;
        Element outputStarterElement;
        this->createCommandElement(outputStarterElement, "output_starter_document");
        this->pushNext(outputStarterElement);
        totalOutputElementsCounter--;
	}
	else if(isLastElement)
	{
        isLastElement = false;
        Element outputEnderElement;
        this->createCommandElement(outputEnderElement, "output_ender_document");
        this->pushNext(outputEnderElement);
        totalOutputElementsCounter--;

        return;
	}


	totalOutputElementsCounter++;

    //	std::cout<<"________system output start__________"<<std::endl;
	int size = element.getSize();

	int p = 0;
	*(Timestamp*)(socketbuffer + p) = element.timestamp;
	//memcpy(socketbuffer + p , &element.timestamp,TIMESTAMP_SIZE);
	p += TIMESTAMP_SIZE;
	*(DocumentId*)(socketbuffer + p) = element.id;
	//memcpy(socketbuffer + p , &element.id, DOCUMENT_IDENTIFIER_SIZE);
	p += DOCUMENT_IDENTIFIER_SIZE;
	*(Mark*)(socketbuffer + p) = element.mark;
	//memcpy(socketbuffer + p , &element.mark, MARK_SIZE);
	p += MARK_SIZE;

	*(MasterTag*)(socketbuffer + p) = element.masterTag;
	p += MASTER_TAG_SIZE;

	memcpy(socketbuffer + p , element.document.objdata(),element.document.objsize());
	p += element.document.objsize(); // document size is stored at the end

	//IOManager::getInstance()->writeOutput(socketbuffer,size,this);
	// If an element can not be inserted due to the inavailability of the buffer, create it and resend the element
	//for(int i = 0; i < size-1; i++)
        //std::cout << socketbuffer[i];

	if(!IOManager::getInstance()->writeOutput(socketbuffer,size,this))
    {
        IOManager::getInstance()->writeOutput(socketbuffer,size,this);
    }
    //std::cout << "socketbuffer | size: " << socketbuffer << " | " << size << std::endl;
	//std::cout<<"SocketStreamOutput Element" << std::endl<<element<<std::endl;
	//sleep(1);
	//std::cout<<"________system output finish_________"<<std::endl;
}

bool SocketStreamOutput::isFull(void)
{
	return false;
}

void SocketStreamOutput::initial(void)
{
	IOManager::getInstance()->addStreamOutput(ip,port,this);
}

int SocketStreamOutput::getTotalOutputElementsCount(void)
{
    return totalOutputElementsCounter;
}

