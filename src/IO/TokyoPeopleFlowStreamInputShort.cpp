//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/TokyoPeopleFlowStreamInputShort.h"
#include "../Schema/JsonSchema.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Common/Types.h"
#include "../Configure/ConfigureManager.h"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <strstream>
#include <time.h>
#include <memory>


//const int FILEBUFF = 65536;
//const int LINESIZE = 256;
//boost::shared_ptr<char> fileBuffer(new char[FILEBUFF]);

// Constructor
TokyoPeopleFlowStreamInputShort::TokyoPeopleFlowStreamInputShort(std::vector<std::string> dataFolderVector,boost::shared_ptr<JsonSchema>jsonSchema):IStreamInput(jsonSchema)
{
	this->dataFolderVector = dataFolderVector;
	this->streamArrivalRate = atoi(ConfigureManager::getInstance()->getConfigureValue("tokyoPeopleStream_input_rate").c_str());
	this->tokyoPeopleTuplesCounter = 0;
	this->canReadStreamTupels = true;

	//fileBuffer = new char[FILEBUFF];
	//buffer = boost::shared_ptr<char>(new char[FILEBUFF]);
	//buffer(new char[FILEBUFF]);
}

// Destructor
TokyoPeopleFlowStreamInputShort::~TokyoPeopleFlowStreamInputShort(void)
{
    //delete[] fileBuffer;
}

void TokyoPeopleFlowStreamInputShort::createCommandElement(Element& element, int peopleFlowFileNameInt, std::string elementType)
{
    if(elementType == "fileStarterElement")
    {
        //create document for this line
        DocumentBuilder documentBuilder;
        documentBuilder.append("pid", peopleFlowFileNameInt );
        //documentBuilder.append("pdate",(long long)timestamp);
        documentBuilder.append("longitude",0);
        documentBuilder.append("latitude",0);
        documentBuilder.append("sex",0);
        documentBuilder.append("age",0);
        documentBuilder.append("work",0);
        //documentBuilder.append("fileName",1234);
        //documentBuilder.append("purpose",purpose);
        Document document = documentBuilder.obj();

        // if element timestamp id contains -99999 then it contains filename
        element.timestamp = TimestampGenerator::getCurrentTime();
        element.mark = PLUS_MARK;
        element.id = peopleFlowFileNameInt;
        element.document = document;
        element.masterTag = false;
        //std::cout << "command element " << element << std::endl;
    }
}


// Get Next Element
void TokyoPeopleFlowStreamInputShort::getNextElement(Element& element)
{
    //char line[LINESIZE];
    //boost::shared_ptr<char> buffer(new char[FILEBUFF]);




    /*
    if(this->currentFileStream)
    {

        if(!(this->currentFileStream.getline(line, LINESIZE)))
        {
            this->currentFileStream.close();
            this->currentPeopleFlowFileListIterator++;
            assert(this->currentPeopleFlowFileListIterator!= this->peopleFlowFileList.end());
            std::string peopleFlowFile = *this->currentPeopleFlowFileListIterator;

            //this->currentFileStream.rdbuf()->pubsetbuf(fileBuffer, FILEBUFF);

            this->currentFileStream.open(peopleFlowFile.c_str());
        }
    */
        if(!isFirstFileElementSent)
        {
            //std::cout << "isFirstFileElementSent " << std::endl;
            isFirstFileElementSent = true;
            std::string peopleFlowFile = *this->currentPeopleFlowFileListIterator;
            int peopleFlowFileName = getIntFileName(peopleFlowFile);
            //std::cout << "peopleFlowFileName " << peopleFlowFileName << std::endl;
            createCommandElement(element, peopleFlowFileName, "fileStarterElement");
            //gettimeofday(&stoppingTime, NULL);
            return;
        }

        if(this->currentFileStream.peek()==EOF)
        {
            //std::cout << "it came EOF" << std::endl;
            this->currentFileStream.close();
            this->currentPeopleFlowFileListIterator++;
            assert(this->currentPeopleFlowFileListIterator!= this->peopleFlowFileList.end());
            std::string peopleFlowFile = *this->currentPeopleFlowFileListIterator;
            this->currentFileStream.open(peopleFlowFile.c_str(), std::ifstream::in);
            //std::cout << "peopleFlowFile " << peopleFlowFile << std::endl;
            /*
            std::string peopleFlowFileName = peopleFlowFile.substr(peopleFlowFile.find_last_of("/") + 1, peopleFlowFileName.find(".") - peopleFlowFile.find_last_of("/"));
            peopleFlowFileName = peopleFlowFileName.substr(0, peopleFlowFileName.find("."));

            std::stringstream fileNameSS;//(peopleFlowFileName);
            fileNameSS << "-";
            fileNameSS << peopleFlowFileName;
            fileNameSS << "99999";

            int peopleFlowFileNameInt;
            fileNameSS >> peopleFlowFileNameInt;
            //std::cout << "peopleFlowFileName " << peopleFlowFileName << std::endl;
            */

            int peopleFlowFileName = getIntFileName(peopleFlowFile);

            if(this->currentFileStream.is_open())
            {
                createCommandElement(element, peopleFlowFileName, "fileStarterElement");
                return;
            }
        }


        //if(this->tokyoPeopleTuplesCounter < this->streamArrivalRate)
        //{
            this->tokyoPeopleTuplesCounter++;

            //string value;
            std::string strLine;

            getline(currentFileStream, strLine);

            std::size_t pos1 = 0;
            std::size_t pos2 = strLine.find(',');
            std::string pid = strLine.substr(pos1, pos2 - pos1);

            pos1 = pos2;

            pos2 = strLine.find(',', pos1 + 1);
            std::string longitudestr = strLine.substr(pos1 + 1, pos2 - pos1 - 1);
            std::stringstream s(longitudestr);
            float longitude;
            s >> longitude;
            pos1 = pos2;

            pos2 = strLine.find(',', pos1 + 1);
            std::string latitudestr = strLine.substr(pos1 + 1, pos2 - pos1 - 1);
            std::stringstream s2(latitudestr);
            float latitude;
            s2 >> latitude;
            pos1 = pos2;

            pos2 = strLine.find(',', pos1 + 1);
            std::string sexstr = strLine.substr(pos1 + 1, pos2 - pos1 - 1);
            std::stringstream s3(sexstr);
            int sex;
            s3 >> sex;
            pos1 = pos2;

            pos2 = strLine.find(',', pos1 + 1);
            std::string agestr = strLine.substr(pos1 + 1, pos2 - pos1 - 1);
            std::stringstream s4(agestr);
            int age;
            s4 >> age;
            pos1 = pos2;

            pos2 = strLine.find('\n', pos1 + 1);
            std::string workstr = strLine.substr(pos1 + 1, pos2 - pos1 - 1);
            std::stringstream s5(workstr);
            int work;
            s5 >> work;
            //std::cout << "work " << work << std::endl;

            //create document for this line
            DocumentBuilder documentBuilder;
            documentBuilder.append("pid",boost::lexical_cast<int>(pid) );
            //documentBuilder.append("pdate",(long long)timestamp);
            documentBuilder.append("longitude",longitude);
            documentBuilder.append("latitude",latitude);
            documentBuilder.append("sex",sex);
            documentBuilder.append("age",age);
            documentBuilder.append("work",work);
            //documentBuilder.append("fileName",1234);
            Document document = documentBuilder.obj();

            //element.timestamp = (Timestamp)timestamp;
            element.timestamp = TimestampGenerator::getCurrentTime();
            element.mark = PLUS_MARK;
            element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
            element.document = document;
            element.masterTag = false;

            //usleep(100);
            //std::cout << element << std::endl;
            //std::cout << "generating data " << std::endl;
        /*
        }
        else
        {
            usleep(100);

            // empty element
            element.timestamp = TimestampGenerator::getCurrentTime();
            element.mark = PLUS_MARK;
            element.id = 0;
            element.masterTag = false;

            std::cout << "sleeping after generating data " << std::endl;
        }

        gettimeofday(&currentTime, NULL);
        double diff = ((currentTime.tv_sec * 1000000 + currentTime.tv_usec) - (stoppingTime.tv_sec * 1000000 + stoppingTime.tv_usec))/1000000.0;
        cout << "diff " << diff << endl;
        if(diff >= 1)
        {
            this->tokyoPeopleTuplesCounter = 0;
            gettimeofday(&stoppingTime, NULL);
        }

        //std::cout<< element <<std::endl;
        */
}

bool TokyoPeopleFlowStreamInputShort::isEmpty()
{
	if(this->currentPeopleFlowFileListIterator == peopleFlowFileList.end())
	{
		return true;
	}

	if(++this->currentPeopleFlowFileListIterator == peopleFlowFileList.end())
	{
		if(this->currentFileStream.peek()==EOF)
		{
			this->currentFileStream.close();
			return true;
		}
		else
		{
			this->currentPeopleFlowFileListIterator--;

		}
	}
	else
	{
		this->currentPeopleFlowFileListIterator--;
	}

	return false;
}

// The initial method obtain the tokyo people flow folder path from the configure file and push all the .csv files containing
// tokyo people flow stream into the peopleFlowFileList and opens the first file for the processing / query execution.
void TokyoPeopleFlowStreamInputShort::initial()
{
	//get tokyo people flow folder path from the respective wrapper.txt file
	for(int i = 0; i < dataFolderVector.size(); i++)
	{
		std::string peopleFlowFolder = dataFolderVector[i];
		boost::filesystem::path peopleFlowFolderFullPath( boost::filesystem::initial_path() );
		peopleFlowFolderFullPath = boost::filesystem::system_complete( boost::filesystem::path( peopleFlowFolder) );
		//peopleFlowFolderFullPath = boost::filesystem::system_complete( boost::filesystem::path( peopleFlowFolder, boost::filesystem::native ) );
		assert ( boost::filesystem::exists( peopleFlowFolderFullPath ) );
		assert ( boost::filesystem::is_directory( peopleFlowFolderFullPath ) );

        // iterator to iterate through all the files in the TokyoPeopleFlowFolder
		boost::filesystem::directory_iterator end_iter;

		//scan the folder and get each wrapper file
		for (  boost::filesystem::directory_iterator dir_itr( peopleFlowFolderFullPath );dir_itr != end_iter;	++dir_itr )
		{
			assert ( boost::filesystem::is_regular_file( *dir_itr ) );

			boost::filesystem::path peopleFlowFilePath= dir_itr->path();
			//wrapper file path
			std::string peopleFlowFileFullPath = peopleFlowFilePath.string();
			std::string fileEextension = peopleFlowFileFullPath.substr(peopleFlowFileFullPath.find_last_of(".")+1);

            // Only files with extension csv are pushed into peopleFlowFileList
			if(fileEextension != "csv")
			{
				continue;
			}
			//add the file to the global list
			this->peopleFlowFileList.push_back(peopleFlowFileFullPath);
			this->peopleFlowFileList.sort();
			//std::cout << "peopleFlowFileFullPath " << peopleFlowFileFullPath << std::endl;
		}
	}

	/*
	for(std::list<std::string>::iterator i = this->peopleFlowFileList.begin(); i != this->peopleFlowFileList.end(); i++)
	{
        std::cout << *i << std::endl;
	}
	*/

    //fileBuffer = new char[FILEBUFF];
    //fileBuffer(new char[FILEBUFF]);
    //boost::shared_ptr<char> fileBuffer(new char[FILEBUFF]);

	this->currentPeopleFlowFileListIterator = this->peopleFlowFileList.begin();
	//this->currentFileStream.rdbuf()->pubsetbuf(fileBuffer, FILEBUFF);
	this->currentFileStream.open((*peopleFlowFileList.begin()).c_str());
	isFirstFileElementSent = false;
}

int TokyoPeopleFlowStreamInputShort::getIntFileName(std::string peopleFlowFile)
{
    std::cout << "Input File Name " << peopleFlowFile << std::endl;

    std::string peopleFlowFileName = peopleFlowFile.substr(peopleFlowFile.find_last_of("/") + 1);
    peopleFlowFileName = peopleFlowFileName.substr(0, peopleFlowFileName.find("."));

    std::stringstream fileNameSS;//(peopleFlowFileName);
    fileNameSS << "-";
    fileNameSS << peopleFlowFileName;
    fileNameSS << "99999";

    int peopleFlowFileNameInt;
    fileNameSS >> peopleFlowFileNameInt;

    fileNameSS.str( std::string() );
    fileNameSS.clear();

    std::cout << "peopleFlowFileNameInt Short " << peopleFlowFileNameInt << std::endl;
    return peopleFlowFileNameInt;
}

