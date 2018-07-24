//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/TokyoPeopleFlowStreamInput.h"
#include "../Schema/JsonSchema.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Common/Types.h"
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <strstream>
#include <time.h>

// Constructor
TokyoPeopleFlowStreamInput::TokyoPeopleFlowStreamInput(std::vector<std::string> dataFolderVector,boost::shared_ptr<JsonSchema>jsonSchema):IStreamInput(jsonSchema)
{
	this->dataFolderVector = dataFolderVector;
}

// Destructor
TokyoPeopleFlowStreamInput::~TokyoPeopleFlowStreamInput(void)
{
}

void TokyoPeopleFlowStreamInput::createCommandElement(Element& element, int peopleFlowFileNameInt, std::string elementType)
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
        documentBuilder.append("padd",0);
        documentBuilder.append("work",0);
        //documentBuilder.append("fileName",1234);
        documentBuilder.append("purpose",0);
        documentBuilder.append("datum",0);
        Document document = documentBuilder.obj();

        // if element timestamp id contains -99999 then it contains filename
        element.timestamp = TimestampGenerator::getCurrentTime();
        element.mark = PLUS_MARK;
        element.id = peopleFlowFileNameInt;
        element.document = document;
        element.masterTag = false;
        //std::cout << "command element " << element << std::endl;
        //sleep(1);

    }
}


// Get Next Element
void TokyoPeopleFlowStreamInput::getNextElement(Element& element)
{
	/*
	if(this->currentFileStream.peek()==EOF)
	{
		this->currentFileStream.close();
		this->currentPeopleFlowFileListIterator++;
		assert(this->currentPeopleFlowFileListIterator!= this->peopleFlowFileList.end());
		std::string peopleFlowFile = *this->currentPeopleFlowFileListIterator;

		this->currentFileStream.open(peopleFlowFile.c_str(), std::ifstream::in);
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

    this->tokyoPeopleTuplesCounter++;



	string value;
	// read one whole line in the file
	getline ( currentFileStream, value, ',' ); // read a string until next comma:
	std::string pid = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string tno = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string sno = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string pdate = value.substr(0,value.length());

	//change time from "2008/10/1  0:00:00" to timestamp
	/*
	std::string yearMonthDay = pdate.substr(0,pdate.find(' '));
	std::string hourMinuteSecond = pdate.substr(pdate.find(' ')+1);
	std::string year = yearMonthDay.substr(0,yearMonthDay.find('/'));
	yearMonthDay = yearMonthDay.substr(yearMonthDay.find('/')+1);
	std::string month = yearMonthDay.substr(0, yearMonthDay.find('/'));
	yearMonthDay = yearMonthDay.substr(yearMonthDay.find('/')+1);
	std::string day = yearMonthDay;
	std::string hour = hourMinuteSecond.substr(0,hourMinuteSecond.find(':'));
	hourMinuteSecond = hourMinuteSecond.substr(hourMinuteSecond.find(':')+1);
	std::string minute = hourMinuteSecond.substr(0,hourMinuteSecond.find(':'));
	hourMinuteSecond = hourMinuteSecond.substr(hourMinuteSecond.find(':')+1);
	std::string second = hourMinuteSecond;

	struct tm t;
	time_t timestamp; //timestamp
	t.tm_year = atoi(year.c_str())-1900;
	t.tm_mon = atoi(month.c_str());
	t.tm_mday = atoi(day.c_str());
	t.tm_hour =  atoi(hour.c_str());;
	t.tm_min =  atoi(minute.c_str());;
	t.tm_sec =  atoi(second.c_str());;
	t.tm_isdst = 0;
	timestamp = mktime(&t);
	*/

	getline ( currentFileStream, value, ',' );
	std::string longitudestr = value.substr(0,value.length());
	std::stringstream s(longitudestr);
	float longitude;
	s >> longitude;

	getline ( currentFileStream, value, ',' );
	std::string latitudestr = value.substr(0,value.length());
	std::stringstream s2(latitudestr);
	float latitude;
	s2 >> latitude;

	getline ( currentFileStream, value, ',' );
	std::string sexStr = value.substr(0,value.length());

	std::stringstream s3(sexStr);
    int sex;
    s3 >> sex;

	getline ( currentFileStream, value, ',' );
	std::string ageStr = value.substr(0,value.length());

	std::stringstream s4(ageStr);
    int age;
    s4 >> age;

	getline ( currentFileStream, value, ',' );
	std::string paddStr = value.substr(0,value.length());

	std::stringstream s5(paddStr);
    int padd;
    s5 >> padd;

	getline ( currentFileStream, value, ',' );
	std::string workstr = value.substr(0,value.length());

	std::stringstream s6(workstr);
    int work;
    s6 >> work;

	getline ( currentFileStream, value, ',' );
	std::string purposeStr = value.substr(0,value.length());

	std::stringstream s7(purposeStr);
    int purpose;
    s7 >> purpose;

	getline ( currentFileStream, value, ',' );
	std::string magfacStr = value.substr(0,value.length());

	std::stringstream s8(magfacStr);
    int magfac;
    s8 >> magfac;

	getline ( currentFileStream, value, ',' );
	std::string magfac2Str = value.substr(0,value.length());

	std::stringstream s9(magfac2Str);
    int magfac2;
    s9 >> magfac2;

	getline ( currentFileStream, value, '\n' );
	std::string datumStr = value.substr(0,value.length());

	std::stringstream s10(datumStr);
    int datum;
    s10 >> datum;

	//create document for this line
	DocumentBuilder documentBuilder;
	documentBuilder.append("pid",boost::lexical_cast<int>(pid) );
	//documentBuilder.append("pdate",(long long)timestamp);
	documentBuilder.append("longitude",longitude);
	documentBuilder.append("latitude",latitude);
	documentBuilder.append("sex",sex);
	documentBuilder.append("age",age);
	documentBuilder.append("padd",padd);
	documentBuilder.append("work",work);
	documentBuilder.append("purpose",purpose);
	documentBuilder.append("datum",datum);
	Document document = documentBuilder.obj();

	//element.timestamp = (Timestamp)timestamp;
	element.timestamp = TimestampGenerator::getCurrentTime();
	element.mark = PLUS_MARK;
	element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	element.document = document;
	element.masterTag = false;

	//std::cout<< element <<std::endl;
	//sleep(1);
}

bool TokyoPeopleFlowStreamInput::isEmpty()
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

/*
// The initial method obtain the tokyo people flow folder path from the configure file and push all the .csv files containing
// tokyo people flow stream into the peopleFlowFileList and opens the first file for the processing / query execution.
void TokyoPeopleFlowStreamInput::initial()
{
	//get tokyo people flow folder path from the configure file
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
		}
	}

	this->currentPeopleFlowFileListIterator = this->peopleFlowFileList.begin();
	this->currentFileStream.open((*peopleFlowFileList.begin()).c_str());
}
*/

void TokyoPeopleFlowStreamInput::initial()
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

int TokyoPeopleFlowStreamInput::getIntFileName(std::string peopleFlowFile)
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

    //std::cout << "peopleFlowFileNameInt " << peopleFlowFileNameInt << std::endl;
    return peopleFlowFileNameInt;
}
