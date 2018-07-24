//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/SocketStreamInput.h"
#include "../IO/IStreamInput.h"
#include "../Internal/Element/Element.h"
#include "../IO/IOManager.h"
#include "../Server/JsonStreamServer.h"
#include "../Schema/JsonSchema.h"
#include "../Utility/TimeCounterUtility.h"
#include "../Utility/TimestampGenerator.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Internal/Synopsis/WindowSynopsis.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm>

int SocketStreamInput::ignoredInputNumber = 0;
int SocketStreamInput::totalInputNumber = 0;

SocketStreamInput::SocketStreamInput(std::string ip,std::string port,boost::shared_ptr<JsonSchema> schema):IStreamInput(schema)
{
	this->ip = ip;
	this->port = port;
	this->windowSynopsis.reset(new WindowSynopsis());
	//std::cout << "SocketStreamInput init: IP " << ip << " ,Port: " << port << " for Schema: " << this->getSchema()->getId() << std::endl;
}

SocketStreamInput::~SocketStreamInput(void)
{
    delete [] remainingDataChr;
	delete [] remainingPlusNewData;
}

void SocketStreamInput::getNextElement(Element& element)
{
    //std::cout << "SocketStreamInput::getNextElement called " << std::endl;
	//checkWindowSynopsisCorrect(1);
	boost::mutex::scoped_lock lock(m_oMutex);
	//std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
	this->windowSynopsis->getOldestElement(element);
	//element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	element.mark = PLUS_MARK;
	element.timestamp = TimestampGenerator::getCurrentTime();
	element.document.getOwned();
	this->windowSynopsis->deleteOldestElement();
	lock.unlock();
	//checkWindowSynopsisCorrect(2);
}
bool SocketStreamInput::isEmpty()
{
    //std::cout << "SocketStreamInput::isEmpty called " << std::endl;
	boost::mutex::scoped_lock lock(m_oMutex);
	bool bl = this->windowSynopsis->isEmpty();
	lock.unlock();
	return bl;
}

const int STREAM_BUFFER_SIZE = 4096;
char stream_buffer[STREAM_BUFFER_SIZE];
int temp = 0;

void SocketStreamInput::readData(char* data, int length)
{
    std::string schemaID = this->getSchema()->getId();
    if(schemaID=="tsukubaMobilityGeolocation" || schemaID=="optimistiCQ" || schemaID=="socket_stream1" )
    {

        int remainingDataLength = 0;
        //std::cout << "schemaID " << schemaID << std::endl;

        remainingDataLength = remainingDataFromPrevArrivalStr.length();

        //char *remainingDataChr = (char*)malloc(remainingDataLength + 1);
        memset(remainingDataChr, 0, REMAINING_INPUT_BUFFER_SIZE);
        //std::copy(remainingDataFromPrevArrivalStr.begin(), remainingDataFromPrevArrivalStr.end(), remainingDataChr);
        //remainingDataChr[remainingDataFromPrevArrivalStr.size()] = '\0';

        //char *remainingPlusNewData = (char*)malloc(length + remainingDataLength);
        memset(remainingPlusNewData, 0, SOCKET_INPUT_BUFFER_SIZE);

        //std::cout << "length " << length << std::endl << data << std::endl;
        if(remainingDataLength != 0)
        {
            std::copy(remainingDataFromPrevArrivalStr.begin(), remainingDataFromPrevArrivalStr.end(), remainingDataChr);
            remainingDataChr[remainingDataFromPrevArrivalStr.size()] = '\0';

            std::cout << "remainingDataFromPrevArrival " << remainingDataFromPrevArrivalStr << std::endl;
            std::cout << "remainingDataLength " << remainingDataLength << std::endl;

            //remainingPlusNewData = malloc(length + remainingDataLength);
            strcpy(remainingPlusNewData, remainingDataChr);
            strcat(remainingPlusNewData, data);

            remainingDataFromPrevArrivalStr = "";
        }
        else
        {
            strcpy(remainingPlusNewData, data);
        }

        //std::cout << "remainingPlusNewData " << remainingPlusNewData << std::endl;
        //boost::mutex::scoped_lock lock(m_oMutex);
        boost::asio::streambuf& streambuffer = this->streambuffer;
        std::ostream os(&streambuffer);

        // For the sake of experiments ignoring remaining data from previous chunk
        //os.write(data,length);
        os.write(remainingPlusNewData, length + remainingDataLength); //  Inserts the length + remainingDataLength characters of the array pointed by remainingPlusNewData into the stream.
        std::istream is (&streambuffer);
        //lock.unlock();

        //std::cout << "this->streambuffer: " << (&streambuffer) << std::endl;
        std::cout << "this->streambuffer.size(): " << this->streambuffer.size() << std::endl;
        //std::cout << "schemaDoc " << this->getSchema()->getSchemaDocument() << std::endl;

        while(this->streambuffer.size()>0)
        {
            Element element;

            if(schemaID=="optimistiCQ")
            {
                //Sample data
                /*
                {
                  "query_id":"0",
                  "tuples": [
                     {"tuple_id":"0","timestamp":"1462878173698","attribute_1":"value1","attribute_2":"value2" },
                     {"tuple_id":"1","timestamp":"1462878173698","attribute_1":"value1","attribute_2":"value2" },
                     {"tuple_id":"2","timestamp":"1462878173698","attribute_1":"value1","attribute_2":"value2" },
                     {"tuple_id":"3","timestamp":"1462878173698","attribute_1":"value1","attribute_2":"value2" },
                     {"tuple_id":"4","timestamp":"1462878173698","attribute_1":"value1","attribute_2":"value2" }
                   ]
                }
                */

                std::string fetchedTupleStr;
                getline(is,fetchedTupleStr); // as we fetch line, buffer gets smaller

                //std::string dataStr = data.c_str();
                std::size_t pos1 = fetchedTupleStr.find('{');
                std::size_t pos2 = fetchedTupleStr.find('}');

                //If a complete tuple then create an element
                if(pos1!=std::string::npos && pos2!=std::string::npos && pos2 > pos1)
                {
                    pos1 = fetchedTupleStr.find("tuple_id");
                    pos2 = fetchedTupleStr.find(',', pos1 + 1);

                    std::string tuple_id = fetchedTupleStr.substr(pos1 + 11, pos2 - 1 - (pos1 + 11));
                    std::stringstream tuple_id_SS(tuple_id);
                    long long _tuple_id;
                    tuple_id_SS >> _tuple_id;

                    pos1 = fetchedTupleStr.find("timestamp");
                    pos2 = fetchedTupleStr.find(',', pos1 + 1);

                    std::string timestamp = fetchedTupleStr.substr(pos1 + 12, pos2 - 1 - (pos1 + 12));
                    std::stringstream timestamp_SS(timestamp);
                    long long _timestamp;
                    timestamp_SS >> _timestamp;

                    pos1 = fetchedTupleStr.find("attribute_1");
                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string _attribute_1 = fetchedTupleStr.substr(pos1 + 14, pos2 - 1 - (pos1 + 14));

                    pos1 = fetchedTupleStr.find("attribute_2");
                    pos2 = fetchedTupleStr.find_last_of('"');
                    std::string _attribute_2 = fetchedTupleStr.substr(pos1 + 14, pos2 - (pos1 + 14));

                    //create document for this tuple
                    DocumentBuilder documentBuilder;
                    documentBuilder.append("tuple_id",boost::lexical_cast<int>(_tuple_id) );
                    documentBuilder.append("timestamp",boost::lexical_cast<long long>(_timestamp) );
                    documentBuilder.append("attribute_1",_attribute_1);
                    documentBuilder.append("attribute_2",_attribute_2);

                    Document document = documentBuilder.obj();

                    element.timestamp = TimestampGenerator::getCurrentTime();
                    element.mark = PLUS_MARK;
                    element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
                    element.document = document;
                    element.masterTag = false;

                    // load shedding
                    if(this->windowSynopsis->getElementNumber()>10000)
                    {
                        ignoredInputNumber++ ;
                        continue;
                    }

                    if(this->windowSynopsis->isFull()==true)
                    {
                        std::cout<<"synopsis out of memory error"<<std::endl;
                        delete [] remainingDataChr;
                        delete [] remainingPlusNewData;
                        exit(1);
                    }
                    else
                    {//synopsis not full
                        totalInputNumber++;
                        boost::mutex::scoped_lock lock(m_oMutex);
                        //std::cout<<element<<std::endl;
                        this->windowSynopsis->insertElement(element);

                        lock.unlock();
                    }
                }
            }
            else if(schemaID=="socket_stream1" || schemaID=="tsukubaMobilityGeolocation")
            {
                //string tuple as line;
                std::string fetchedTupleStr;
                getline(is,fetchedTupleStr);

                /*
                // For the sake of experiments ignoring incomplete tuples at the start of the chunk
                size_t commaCount = std::count(fetchedTupleStr.begin(), fetchedTupleStr.end(), ',');
                if (commaCount < 5 || fetchedTupleStr.length() < 32)
                    continue;
                */
                // read only complete lines/tuples
                if(is.eof())
                {
                    remainingDataFromPrevArrivalStr = fetchedTupleStr;
                    break;
                }

                if(schemaID=="socket_stream1")
                {
                    std::size_t pos1 = 0;
                    std::size_t pos2 = fetchedTupleStr.find(',');
                    std::string pid = fetchedTupleStr.substr(pos1, pos2 - pos1);

                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string longitudestr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s(longitudestr);
                    float longitude;
                    s >> longitude;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string latitudestr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s2(latitudestr);
                    float latitude;
                    s2 >> latitude;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string sexstr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s3(sexstr);
                    int sex;
                    s3 >> sex;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string agestr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s4(agestr);
                    int age;
                    s4 >> age;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find('\n', pos1 + 1);
                    std::string workstr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s5(workstr);
                    int work;
                    s5 >> work;

                    //std::cout << "pid: " << pid << std::endl;
                    //std::cout << "work: " << work << std::endl;

                    //create document for this line
                    DocumentBuilder documentBuilder;
                    documentBuilder.append("pid",boost::lexical_cast<int>(pid) );
                    documentBuilder.append("longitude",longitude);
                    documentBuilder.append("latitude",latitude);
                    documentBuilder.append("sex",sex);
                    documentBuilder.append("age",age);
                    documentBuilder.append("work",work);


                    Document document = documentBuilder.obj();

                    element.timestamp = TimestampGenerator::getCurrentTime();
                    element.mark = PLUS_MARK;
                    element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
                    element.document = document;
                    element.masterTag = false;



                    // load shedding
                    if(this->windowSynopsis->getElementNumber()>10000)
                    {
                        ignoredInputNumber++ ;
                        continue;
                    }

                    if(this->windowSynopsis->isFull()==true)
                    {
                        std::cout<<"synopsis out of memory error"<<std::endl;
                        delete [] remainingDataChr;
                        delete [] remainingPlusNewData;
                        exit(1);
                    }
                    else
                    {//not full
                        totalInputNumber++;
                        boost::mutex::scoped_lock lock(m_oMutex);
                        //std::cout<<element<<std::endl;
                        this->windowSynopsis->insertElement(element);
                        this->windowSynopsis->insertElement(element);
                        this->windowSynopsis->insertElement(element);
                        lock.unlock();
                        //std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
                    }
                }
                else if(schemaID=="tsukubaMobilityGeolocation")
                {
                    std::size_t pos1 = 0;
                    std::size_t pos2 = fetchedTupleStr.find(',');
                    std::string obvID = fetchedTupleStr.substr(pos1, pos2 - pos1); //obvID
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //surveyCode
                    pos1 = pos2;
                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //surveyName
                    pos1 = pos2;
                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //userID
                    pos1 = pos2;
                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //userName
                    pos1 = pos2;
                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //loginID
                    pos1 = pos2;
                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //recordTimestamp
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string latitudestr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s2(latitudestr);
                    float latitude;
                    s2 >> latitude;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string longitudestr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s(longitudestr);
                    float longitude;
                    s >> longitude;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //altitude
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //measurementError
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //measurementMode
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1);
                    std::string ignitionStatusStr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s3(ignitionStatusStr);
                    int ignitionStatus;
                    s3 >> ignitionStatus;
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find(',', pos1 + 1); //generatedTimestamp
                    pos1 = pos2;

                    pos2 = fetchedTupleStr.find('\n', pos1 + 1);
                    std::string validityStr = fetchedTupleStr.substr(pos1 + 1, pos2 - pos1 - 1);
                    std::stringstream s5(validityStr);
                    int validity;
                    s5 >> validity;

                    //create document for this line
                    DocumentBuilder documentBuilder;
                    documentBuilder.append("obvID",boost::lexical_cast<int>(obvID) );
                    documentBuilder.append("longitude",longitude);
                    documentBuilder.append("latitude",latitude);
                    documentBuilder.append("ignitionStatus",ignitionStatus);
                    documentBuilder.append("validity",validity);

                    Document document = documentBuilder.obj();

                    element.timestamp = TimestampGenerator::getCurrentTime();
                    element.mark = PLUS_MARK;
                    element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
                    element.document = document;
                    element.masterTag = false;

                    // load shedding
                    if(this->windowSynopsis->getElementNumber()>10000)
                    {
                        ignoredInputNumber++ ;
                        continue;
                    }

                    if(this->windowSynopsis->isFull()==true)
                    {
                        std::cout<<"synopsis out of memory error"<<std::endl;
                        delete [] remainingDataChr;
                        delete [] remainingPlusNewData;
                        exit(1);
                    }
                    else
                    {//not full
                        totalInputNumber++;
                        boost::mutex::scoped_lock lock(m_oMutex);
                        //std::cout<<element<<std::endl;
                        this->windowSynopsis->insertElement(element);
                        this->windowSynopsis->insertElement(element);
                        this->windowSynopsis->insertElement(element);
                        lock.unlock();
                    }
                }
            }
        }
    }
    else
    {

        boost::mutex::scoped_lock lock(m_oMutex);
        boost::asio::streambuf& streambuffer = this->streambuffer;
        std::ostream os(&streambuffer);
        os.write(data,length);
        std::istream is (&streambuffer);

        while(this->streambuffer.size()>0)
        {
            if(this->streambuffer.size()<TIMESTAMP_SIZE + DOCUMENT_IDENTIFIER_SIZE + MARK_SIZE + MASTER_TAG_SIZE + sizeof(int))
            {
                break;
            }

            Element element;

            int p = 0;
            is.read(stream_buffer + p,TIMESTAMP_SIZE);
            element.timestamp = *(Timestamp*)(stream_buffer+p);
            p += TIMESTAMP_SIZE;

            is.read(stream_buffer + p,DOCUMENT_IDENTIFIER_SIZE);
            element.id = *(DocumentId*)(stream_buffer+p);
            p += DOCUMENT_IDENTIFIER_SIZE;

            is.read(stream_buffer + p,MARK_SIZE);
            element.mark = *(Mark*)(stream_buffer + p);
            p += MARK_SIZE;

            is.read(stream_buffer + p, MASTER_TAG_SIZE);
            element.masterTag = *(MasterTag*)(stream_buffer + p);
            p += MASTER_TAG_SIZE;

            is.read(stream_buffer + p, sizeof(int)); //document size;
            int documentSize = *(int*)(stream_buffer + p);
            p += sizeof(int);

            //std::cout << "this->streambuffer.size() | documentSize : " << this->streambuffer.size() << " | " << documentSize << std::endl;

            if(this->streambuffer.size() < documentSize)
            {
                int remainBytes = streambuffer.size();
                is.read(stream_buffer + p ,remainBytes );
                p += remainBytes;
                os.write(stream_buffer, p);
                break;
            }
            else
            {
                char* documentPosition = stream_buffer + p - sizeof(int);
                is.read(stream_buffer + p, documentSize - sizeof(int));

                Document document (documentPosition);
                element.document = document;
                element.document.getOwned();

                // load shedding
                if(this->windowSynopsis->getElementNumber()>100)
                {
                    ignoredInputNumber ++ ;
                    //std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
                    break;
                }

                if(this->windowSynopsis->isFull()==true)
                {
                    std::cout<<"synopsis out of memory error"<<std::endl;
                    exit(1);
                }
                else
                {//not full
                    this->windowSynopsis->insertElement(element);
                    //std::cout<<this->windowSynopsis->getElementNumber()<<std::endl;
                }
            }
            //std::cout << element << std::endl;
            //sleep(1);
        }
        lock.unlock();
    }
}

void SocketStreamInput::initial()
{
	IOManager::getInstance()->addStreamInput(ip,port,this);
}
