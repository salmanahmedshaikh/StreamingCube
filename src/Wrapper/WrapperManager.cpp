//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Wrapper/WrapperManager.h"
#include "../Configure/ConfigureManager.h"
#include "../Common/Types.h"
#include "../BinaryJson/json.h"
#include "../Schema/JsonSchema.h"
#include "../Server/JsonStreamServer.h"
#include "../IO/RandomGeneratedStreamInput.h"
#include "../IO/SocketStreamInput.h"
#include "../IO/TwitterStreamInput.h"
#include "../Query/QueryManager.h"
#include "../IO/PeopleFlowStreamInput.h"
#include "../IO/TokyoPeopleFlowStreamInput.h"
#include "../IO/TokyoPeopleFlowStreamInputShort.h"
#include "../IO/RssStreamInput.h"
#include "../IO/SpecifiedInputRateStreamInput.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <strstream>

WrapperManager* WrapperManager::wrapperManager = NULL;
WrapperManager::WrapperManager(void)
{
	initial();
}


WrapperManager::~WrapperManager(void)
{
}
WrapperManager * WrapperManager::getInstance()
{
	if(wrapperManager==NULL)
	{
		wrapperManager = new WrapperManager();
	}
	return wrapperManager;
}

void WrapperManager::initial()
{
	//get the wrapper folder path from the configure file
	std::string wrapperFolder = ConfigureManager::getInstance()->getConfigureValue(WRAPPER_FOLDER);
	boost::filesystem::path wrapperFolderFullPath( boost::filesystem::initial_path() );
	wrapperFolderFullPath = boost::filesystem::system_complete( boost::filesystem::path( wrapperFolder ) );
	assert ( boost::filesystem::exists( wrapperFolderFullPath ) );
	assert ( boost::filesystem::is_directory( wrapperFolderFullPath ) );
	boost::filesystem::directory_iterator end_iter;
	//scan the folder and get each wrapper file
	for (  boost::filesystem::directory_iterator dir_itr( wrapperFolderFullPath );dir_itr != end_iter;	++dir_itr )
	{

			assert ( boost::filesystem::is_regular_file( *dir_itr ) );

			boost::filesystem::path wrapperFilePath= dir_itr->path();
			//wrapper file path
			std::string wrapperFileFullPath = wrapperFilePath.string();
			std::string extensionFilename = wrapperFileFullPath.substr(wrapperFileFullPath.find_last_of(".")+1);

			if(extensionFilename != "txt")
			{
				continue;
			}
			//open the file and explain it
			readWrapperFile(wrapperFileFullPath);
	}
}

void WrapperManager::readWrapperFile(std::string filePath)
{
	std::ifstream fin(filePath.c_str());
	std::stringstream  in;// << fin.rdbuf();
	in << fin.rdbuf() ;
	std::string wrapperDocumentString = in.str();
	//std::cout<< "new wrapper starts:: " << wrapperDocumentString<<std::endl;
	Document wrapperDocuemnt = fromjson(wrapperDocumentString);
	registerWrapper(wrapperDocuemnt);
}

bool WrapperManager::registerWrapper(Document& wrapperDocument)
{
	std::string implementClass = wrapperDocument.getField(IMPLEMENT_CLASS).valuestr();
	Document argumentDocument = wrapperDocument.getField(CLASS_ARGUMENT).embeddedObject();
	Document schemaDocument = wrapperDocument.getField(INFORMATION_SOURCE_SCHEMA).embeddedObject();

	boost::shared_ptr<JsonSchema>jsonSchema(new JsonSchema(schemaDocument));

	if(implementClass == "RandomGeneratedStreamInput")
	{
		boost::shared_ptr<IStreamInput> streamInput (new RandomGeneratedStreamInput(jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "SpecifiedInputRateStreamInput")
	{
		boost::shared_ptr<IStreamInput> streamInput (new SpecifiedInputRateStreamInput(jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "LogStreamInput")
	{
		std::string ip = argumentDocument.getField(IP).valuestr();
		std::string port = argumentDocument.getField(PORT).valuestr();
		boost::shared_ptr<IStreamInput> streamInput(new SocketStreamInput(ip,port,jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "ConnectionStreamInput")
	{
		std::string ip = argumentDocument.getField(IP).valuestr();
		std::string port = argumentDocument.getField(PORT).valuestr();
		boost::shared_ptr<IStreamInput> streamInput(new SocketStreamInput(ip,port,jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "SocketStreamInput")
	{
		std::string ip = argumentDocument.getField(IP).valuestr();
		std::string port = argumentDocument.getField(PORT).valuestr();
		boost::shared_ptr<IStreamInput> streamInput(new SocketStreamInput(ip,port,jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "TwitterStreamInput")
	{
		std::string userName = argumentDocument.getField(USER_NAME).valuestr();
		std::string userPassword = argumentDocument.getField(USER_PASSWORD).valuestr();
		boost::shared_ptr<IStreamInput> streamInput(new TwitterStreamInput(userName,userPassword,jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "RssStreamInput")
	{
		Document  urlArrayDocument = argumentDocument.getField(URL_ARRAY).embeddedObject();
		Document  encodingArrayDocument = argumentDocument.getField(CORRESPONDING_ENCODING_ARRAY).embeddedObject();

		std::vector<std::string> urlVector;
		std::vector<std::string> encodingVector;
		DocumentIterator it(urlArrayDocument);
		while(it.more())
		{
			DocumentElement documentElement =  it.next();
			std::string url = documentElement.valuestr();
			urlVector.push_back(url);
		}
		DocumentIterator it2(encodingArrayDocument);
		while(it2.more())
		{
			DocumentElement documentElement =  it2.next();
			std::string url = documentElement.valuestr();
			encodingVector.push_back(url);
		}
		boost::shared_ptr<IStreamInput> streamInput(new RssStreamInput(urlVector,encodingVector,jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "PeopleFlowStreamInput")
	{
		Document  dataFolderDocument = argumentDocument.getField(DATA_FOLDER).embeddedObject();

		std::vector<std::string> dataFolderVector;

		DocumentIterator it(dataFolderDocument);
		while(it.more())
		{
			DocumentElement documentElement =  it.next();

			std::string folder = documentElement.valuestr();
			dataFolderVector.push_back(folder);
		}

		boost::shared_ptr<IStreamInput> streamInput(new PeopleFlowStreamInput(dataFolderVector, jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "TokyoPeopleFlowStreamInput")
	{
		Document dataFolderDocument = argumentDocument.getField(DATA_FOLDER).embeddedObject();

		std::vector<std::string> dataFolderVector;

		DocumentIterator it(dataFolderDocument);
		while(it.more())
		{
			DocumentElement documentElement =  it.next();

			std::string folder = documentElement.valuestr();
			dataFolderVector.push_back(folder);
		}

        // creating an instance of TokyoPeopleFlowStreamInput
		boost::shared_ptr<IStreamInput> streamInput(new TokyoPeopleFlowStreamInput(dataFolderVector, jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if(implementClass == "TokyoPeopleFlowStreamInput_short")
	{
		Document dataFolderDocument = argumentDocument.getField(DATA_FOLDER).embeddedObject();

		std::vector<std::string> dataFolderVector;

		DocumentIterator it(dataFolderDocument);
		while(it.more())
		{
			DocumentElement documentElement =  it.next();

			std::string folder = documentElement.valuestr();
			dataFolderVector.push_back(folder);
		}

        // creating an instance of TokyoPeopleFlowStreamInput
		boost::shared_ptr<IStreamInput> streamInput(new TokyoPeopleFlowStreamInputShort(dataFolderVector, jsonSchema));
		QueryManager::getInstance()->registerStream(streamInput);
	}
	else if (implementClass == "RelationInput")
	{
		std::string ip = argumentDocument.getField(IP).valuestr();
		std::string port = argumentDocument.getField(PORT).valuestr();
		std::string userName = argumentDocument.getField(USER_NAME).valuestr();
		std::string userPassword = argumentDocument.getField(USER_PASSWORD).valuestr();
		std::string databaseName = argumentDocument.getField(DATABASE_NAME).valuestr();
		std::string tableName =	argumentDocument.getField(TABLE_NAME).valuestr();

		boost::shared_ptr<RelationInput> relationInput(new RelationInput(ip, port, userName, userPassword, databaseName, tableName, jsonSchema));
		QueryManager::getInstance()->registerRelation(relationInput);
	}
	else if (implementClass == "CSVInput")
	{
		std::string ip = argumentDocument.getField(IP).valuestr();
		std::string port = argumentDocument.getField(PORT).valuestr();
		std::string dataFile =	argumentDocument.getField(DATA_FILE).valuestr();

		//std::cout << "dataFile " << dataFile << std::endl;
		boost::shared_ptr<CSVInput> csvInput(new CSVInput(ip, port, dataFile, jsonSchema));
		QueryManager::getInstance()->registerCSV(csvInput);
	}

	else
	{
		std::cout<<implementClass<<" wrapper implement class  does not exist "<<std::endl;
		assert(false);
		return false;
	}
	return true;
}
