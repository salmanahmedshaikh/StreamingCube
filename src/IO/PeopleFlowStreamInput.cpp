//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/PeopleFlowStreamInput.h"
#include "../Schema/JsonSchema.h"
#include "../Utility/DocumentIdentifierGenerator.h"
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
PeopleFlowStreamInput::PeopleFlowStreamInput(std::vector<std::string> dataFolderVector,boost::shared_ptr<JsonSchema>jsonSchema):IStreamInput(jsonSchema)
{
	this->dataFolderVector = dataFolderVector;
}


PeopleFlowStreamInput::~PeopleFlowStreamInput(void)
{
}

void PeopleFlowStreamInput::getNextElement(Element& element)
{


	if(this->currentFileStream.peek()==EOF)
	{
		this->currentFileStream.close();
		this->currentPeopleFlowFileListIterator++;
		assert(this->currentPeopleFlowFileListIterator!= this->peopleFlowFileList.end());
		std::string peopleFlowFile = *this->currentPeopleFlowFileListIterator;
		this->currentFileStream.open(peopleFlowFile.c_str(), std::ifstream::in);
	}

	string value;

	// read one whole line in the file
	getline ( currentFileStream, value, ',' ); // read a string until next comma:
	std::string personid = value.substr(0,value.length());


	getline ( currentFileStream, value, ',' );
	std::string version = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string tripNumber = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string time = value.substr(0,value.length());
	//change time from "2008/10/1  0:00:00" to timestamp
	std::string yearMonthDay = time.substr(0,time.find(' '));
	std::string hourMinuteSecond = time.substr(time.find(' ')+1);
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
	std::string gender = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string age = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string home = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string occupation = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string destination = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string expandingCoefficient = value.substr(0,value.length());

	getline ( currentFileStream, value, ',' );
	std::string expandingCoefficient2 = value.substr(0,value.length());

	getline ( currentFileStream, value, '\n' );
	std::string transportation = value.substr(0,value.length());

	//create document for this line
	DocumentBuilder documentBuilder;
	documentBuilder.append("personid",boost::lexical_cast<int>(personid) );
	documentBuilder.append("time",(long long)timestamp);
	documentBuilder.append("longitude",longitude);
	documentBuilder.append("latitude",latitude);
	documentBuilder.append("gender",gender);
	documentBuilder.append("age",age);
	documentBuilder.append("home",home);
	documentBuilder.append("occupation",occupation);
	Document document = documentBuilder.obj();

	element.timestamp = (Timestamp)timestamp;
	element.mark = PLUS_MARK;
	element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	element.document = document;
	element.masterTag = false;
	//std::cout<<element<<std::endl;

}
bool PeopleFlowStreamInput::isEmpty()
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

void PeopleFlowStreamInput::initial()
{
	//get one people flow folder path from the configure file

	for(int i = 0; i<dataFolderVector.size(); i++)
	{
		std::string peopleFlowFolder = dataFolderVector[i];
		boost::filesystem::path peopleFlowFolderFullPath( boost::filesystem::initial_path() );
		peopleFlowFolderFullPath = boost::filesystem::system_complete( boost::filesystem::path( peopleFlowFolder) );
		//peopleFlowFolderFullPath = boost::filesystem::system_complete( boost::filesystem::path( peopleFlowFolder, boost::filesystem::native ) );
		assert ( boost::filesystem::exists( peopleFlowFolderFullPath ) );
		assert ( boost::filesystem::is_directory( peopleFlowFolderFullPath ) );
		boost::filesystem::directory_iterator end_iter;
		//scan the folder and get each wrapper file
		for (  boost::filesystem::directory_iterator dir_itr( peopleFlowFolderFullPath );dir_itr != end_iter;	++dir_itr )
		{

			assert ( boost::filesystem::is_regular_file( *dir_itr ) );

			boost::filesystem::path peopleFlowFilePath= dir_itr->path();
			//wrapper file path
			std::string peopleFlowFileFullPath = peopleFlowFilePath.string();
			std::string extensionFilename = peopleFlowFileFullPath.substr(peopleFlowFileFullPath.find_last_of(".")+1);

			if(extensionFilename != "csv")
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
