//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/DispatcherStreamInput.h"
#include "../IO/IStreamInput.h"
#include "../Internal/Element/Element.h"
#include "../IO/IOManager.h"
#include "../Server/JsonStreamServer.h"
#include "../Schema/JsonSchema.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Utility/TimestampGenerator.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Configure/ConfigureManager.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "../Internal/Synopsis/WindowSynopsis.h"
#ifdef WINDOWS
#include <windows.h>
#endif

int DispatcherStreamInput::ignoredInputNumber = 0;
//boost::unordered_map<struct evbuffer*, BUFFER* > bufferMap;

bool DispatcherStreamInput::getIsDispatcherStreamInput(void)
{
    return this->isDispatcherStreamInput;
}

void DispatcherStreamInput::setIsDispatcherStreamInput(bool isDispatcherStreamInput)
{
    this->isDispatcherStreamInput = isDispatcherStreamInput;
}

DispatcherStreamInput::DispatcherStreamInput(void)
{
    isDispatcherStreamInput = false;
    isStarterElementArrived = false;
    this->windowSynopsis.reset(new WindowSynopsis());
    //std::cout << "windowSynopsis address: " << this->windowSynopsis << std::endl;
}

DispatcherStreamInput::DispatcherStreamInput(std::string streamSource)
{
    isDispatcherStreamInput = false;
    this->streamSource = streamSource;
    isStarterElementArrived = false;
    this->windowSynopsis.reset(new WindowSynopsis());
}

DispatcherStreamInput::~DispatcherStreamInput(void)
{
}

void DispatcherStreamInput::getNextElement(Element& element)
{
	//checkWindowSynopsisCorrect(1);
	boost::mutex::scoped_lock lock(m_oMutex);
	//std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
	this->windowSynopsis->getOldestElement(element);

	/*
	element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	element.mark = PLUS_MARK;
	element.timestamp = TimestampGenerator::getCurrentTime();
	*/

	element.document.getOwned();
	this->windowSynopsis->deleteOldestElement();
	lock.unlock();
	//checkWindowSynopsisCorrect(2);
}

bool DispatcherStreamInput::isEmpty()
{
	boost::mutex::scoped_lock lock(m_oMutex);
	bool bl = this->windowSynopsis->isEmpty();
	//std::cout << "isEmpty : # of elements in Window synopsis " << this->windowSynopsis << ", are: " << this->windowSynopsis->getElementNumber() << std::endl;
	lock.unlock();

	return bl;
}

//void DispatcherStreamInput::readData(char* data, int length)
void DispatcherStreamInput::readData(BUFFER* buffer, struct evbuffer* input, int buffer_len)
{

    boost::mutex::scoped_lock lock(m_oMutex);

    /*
    BUFFER * buffer = NULL;
    boost::unordered_map<struct evbuffer*, BUFFER* >::iterator it;
    //boost::shared_ptr<QueueEntity> workerOutputQueue (new QueueEntity());
    //std::list<boost::shared_ptr<QueueEntity> >& getInputQueueList(void);

    // if buffer found for the specific input stream OR following if is true when we receive input from a specific input for the first time
    if ((it = bufferMap.find(input)) == bufferMap.end())
    {
        buffer = new BUFFER();// need to be released some time
        buffer->inputBuffer = (char*) malloc(PAGE_SIZE * 2);
        buffer->inputOffset = 0;
        bufferMap.insert(make_pair(input, buffer));

        // Check here which stream this buffer belongs, master, non-master and accordingly form the input queue...

        //std::cout << "Buffer initialized." << std::endl;
        // workerBufferTimeout. If a tuple takes more than 1 second to reach the AP Handler, it will be discarded automatically
        //elementTimeout = TimestampGenerator::getCurrentTime() - elementTimeoutDuration;
    }
    else
    {
        buffer = it->second;
        //elementTimeout = TimestampGenerator::getCurrentTime() - elementTimeoutDuration;
    }
    */

    //std::cout << "buffer->inputOffset " << buffer->inputOffset << std::endl;
    //std::cout << "buffer_len " << buffer_len << std::endl;

    if(buffer->inputOffset+buffer_len>8192)
    {
        int a  = 1;
        a++;
    }

    assert(buffer->inputOffset + buffer_len <= 8192);

    evbuffer_remove(input, buffer->inputBuffer + buffer->inputOffset, buffer_len);
    buffer->inputOffset += buffer_len;

    Element element;
    int currentOffset = 0;// how many bytes we have read from the buffer
    while(1)
    {
        if(currentOffset == buffer->inputOffset)
        {   // all the input data have been dealed with
            buffer->inputOffset = 0;
            break;
        }
        assert(currentOffset < buffer->inputOffset);
        //analyze the remaining data, read next element
        //analyze if we can achieve reading the size of the next document
        if(currentOffset+ sizeof(Timestamp)+sizeof(Mark)+sizeof(DocumentId)+ sizeof(MasterTag)+sizeof(int) > buffer->inputOffset)
        {  // we can not achieve, we copy the remaining data to the front of the buffer and quit
            memcpy(buffer->inputBuffer, buffer->inputBuffer+currentOffset, buffer->inputOffset - currentOffset);
            buffer->inputOffset = buffer->inputOffset - currentOffset;
            break;
        }
        //we can achieve the size of the next document
        //analyze if the remaining data is a whole element or not
        int documentSize = *(int*) (buffer->inputBuffer + currentOffset + sizeof(Timestamp)+sizeof(Mark)+sizeof(MasterTag)+sizeof(DocumentId));
        int elementSize = sizeof(Timestamp) + sizeof(Mark) + sizeof(DocumentId) +sizeof(MasterTag)+ documentSize;

        if(currentOffset + elementSize <= buffer->inputOffset)
        { // yes , it is a whole element in the buffer
            element.timestamp = *(Timestamp*)(buffer->inputBuffer+currentOffset);
            currentOffset += TIMESTAMP_SIZE;


            element.id = *(DocumentId*)(buffer->inputBuffer+currentOffset);
            currentOffset += DOCUMENT_IDENTIFIER_SIZE;


            element.mark = *(Mark*)(buffer->inputBuffer + currentOffset);
            currentOffset += MARK_SIZE;

            element.masterTag = *(MasterTag*)(buffer->inputBuffer + currentOffset);
            currentOffset += MASTER_TAG_SIZE;

            int documentSize = *(int*)(buffer->inputBuffer + currentOffset);
            char* documentPosition = buffer->inputBuffer + currentOffset;
            currentOffset += documentSize;
            assert(currentOffset <= buffer->inputOffset);

            Document document (documentPosition);
            element.document = document;
            element.document.getOwned();

            // If starter element arrived, process it. Else insert the normal elements in the windowSynopsis
            if(isStarterElementArrived == false)
            {
                std::string documentType = document.getField("document_type").valuestr();
                //std::cout<<"Dispatcher address: "<< this <<std::endl;

                if(documentType.compare("dispatcher_starter_document") == 0)
                {
                    isStarterElementArrived = true;
                    std::string streamSource = document.getField("stream_source").valuestr();

                    IOManager::getInstance()->insertIntoDispatcherStreamSourceMap(streamSource, input);

                    //std::cout << "Starter Element from DispatcherStreamInput" << std::endl;
                    //std::cout << element << std::endl;
                    //sleep(5);
                    std::cout << "windowSynopsis in dispatcher Stream input " <<  this->windowSynopsis << std::endl;

                }
            }
            else
            {
                // Commented to avoid assert failure, as we are currently not executing query, so the the # of elements exceed 100 and causes break unncecessarily


                /*
                //load shedding
                if(this->windowSynopsis->getElementNumber()>100)
                {
                    ignoredInputNumber ++ ;
                    //std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
                    std::cout << "break occurs in DispatcherStreamInput: Load Shedding" << std::endl;
                    break;
                }
                */

                if(this->windowSynopsis->isFull()==true)
                {
                    std::cout<<"synopsis out of memory error"<<std::endl;
                    exit(1);
                }
                else // either ignore element or insert into windowsynopsis
                {
                    int loadSheddingThreshold = 3500;
                    if(this->windowSynopsis->getElementNumber() <= loadSheddingThreshold)
                    {
                        // If less than ladSheddingThreshold elements in the windowSynopsis then insert else do load shedding
                        //window synopsis not full
                        this->windowSynopsis->insertElement(element);
                    }
                    else
                    {
                        ignoredInputNumber++ ;
                        //std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
                    }

                    //window synopsis not full without load shedding
                    //this->windowSynopsis->insertElement(element);
                    //std::cout << "Element inserted in windowSynopsis " << this->windowSynopsis << std::endl;
                    //std::cout << element << std::endl;
                    //std::cout << "readData : # of elements in Window synopsis " << this->windowSynopsis << ", are: " << this->windowSynopsis->getElementNumber() << std::endl;

                    // Just for debugging, to check if the element gets deleted continuously
                    //Element newElement;
                    //this->getAndCheckNextElement(newElement);
                }
            }
        }
        else
        { // no , it is not a whole element in the buffer
            memcpy(buffer->inputBuffer, buffer->inputBuffer+currentOffset, buffer->inputOffset - currentOffset);
            buffer->inputOffset = buffer->inputOffset - currentOffset;
            //std::cout << "break occurs in DispatcherStreamInput" << std::endl;
            //std::cout << "readData : # of elements in Window synopsis " << this->windowSynopsis << ", are: " << this->windowSynopsis->getElementNumber() << std::endl;
            break;
        }
    }

    lock.unlock();
    //std::cout << this->windowSynopsis->getElementNumber() << std::endl;
    //std::cout << this->windowSynopsis << ", " << ignoredInputNumber + 101 << std::endl;
}


void DispatcherStreamInput::initial()
{
	//use IOManager's function getDispatcherStream to listen on a port for data streams
    //IOManager::getInstance()->getDispatcherStream(ConfigureManager::getInstance()->getConfigureValue("worker_data_port"), this);
}
