//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/RssStreamInput.h"
#include "../IO/IStreamInput.h"
#include "../Wrapper/TwitterWrapper/curl.h"
#include "../Utility/EncodingUtility.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
static const std::string rssLogFileName = "rsslog.txt";

RssStreamInput::RssStreamInput(std::vector<std::string> urlVector, std::vector<std::string> encodingVector , boost::shared_ptr<JsonSchema>jsonSchema):IStreamInput(jsonSchema)
{
	this->urlVector = urlVector;
	this->encodingVector = encodingVector;
}


RssStreamInput::~RssStreamInput(void)
{
}
size_t RssStreamInput::getRssCallBackFunction(void *ptr, size_t size, size_t nmemb, void *userp)
{
	ostringstream& urlContentStream = *(ostringstream*) userp;
	char* str = (char*) ptr;
	std::string jsonDocumentString(str, size*nmemb);
	//std::cout<<jsonDocumentString<<std::endl;
	urlContentStream<<jsonDocumentString;
	return size*nmemb;
}
void RssStreamInput::getNextElement(Element& element)
{
	assert(!isEmpty());
	element = this->rssElementList.front();
	this->rssElementList.pop_front();
}
bool RssStreamInput::isEmpty()
{
	if(this->rssElementList.size() == 0)
	{
	//	Sleep(5*60*1000);//wait for 5 minutes
		initial();       //then again fetch the RSS url content
	}
	return this->rssElementList.size()==0;
}
void RssStreamInput::initial()
{
	readLogFile();
	isUrlTitleMapValueChanged = false;

	processRSS();

	if(isUrlTitleMapValueChanged == true)
	{
		writeLogFile();
	}

}
void RssStreamInput::processRSS(void)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	assert(curl);


	for(int i = 0 ; i <urlVector.size(); i++)
	{
		std::string url = urlVector[i];
		std::string encoding = encodingVector[i];
		ostringstream rssContentStream;
		std::string rssContent;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getRssCallBackFunction);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rssContentStream);
		res = curl_easy_perform(curl);
		rssContent = rssContentStream.str();
		//get the current system encoding
		std::string systemStringEncoding = EncodingUtility::getCurrentSystemStringEncoding();

		if(systemStringEncoding != encoding)
		{
			//translate rss content into system readable encoding
			rssContent = EncodingUtility::convertEncoding(rssContent, systemStringEncoding, encoding);
		}
		//std::cout<<rssContent<<std::endl;
		generateElementByRss(rssContent);
		if (res!=CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() faild: %s\n", curl_easy_strerror(res));
			assert(false);
		}
	}

	curl_easy_cleanup(curl);
}
//parse the RSS feed which is xml, get each items, generate element for new item and insert into elementList
void RssStreamInput::generateElementByRss(std::string& rssXmlContent)
{
	boost::property_tree::ptree pt;
	stringstream ss;
	ss << rssXmlContent;
	read_xml(ss, pt);
	boost::property_tree::ptree channelPt;
	channelPt = pt.get_child("rss.channel");
	std::string linkurl = channelPt.get_child("link").data();

	std::vector<std::string> titleVector; // save the titles in the RSS feed page
	std::vector<std::string> linkVector;  // save the links in the RSS feed page

	//mark if the newest title we processed is still in this RSS feed page
	bool isProcessedNewestTitleStillInThisPage = false;

	std::string processedNewestTitle;
	std::map<std::string, std::string >::iterator it = this->urlTitleMap.find(linkurl);
	if(it != this->urlTitleMap.end())
	{
		processedNewestTitle = it->second;
	}
	else
	{
		processedNewestTitle = "";
	}

	BOOST_FOREACH(boost::property_tree::ptree::value_type &v1, channelPt)
	{
		if(v1.first=="item")
		{

			boost::property_tree::ptree itemPt = v1.second;
			//get the title and link for each item
			std::string title;
			std::string link;
			title = itemPt.get_child("title").data();
			link = itemPt.get_child("link").data();
			//save title and link
			titleVector.push_back(title);
			linkVector.push_back(link);
			//see if any exists
			if(processedNewestTitle == title)
			{
				isProcessedNewestTitleStillInThisPage = true;
			}
		}
	}
	//if new titles exist, generate corresponding elements
	if(isProcessedNewestTitleStillInThisPage == false)
	{//no title has been processed, generate element for each one
		int number = titleVector.size();
		for( int i = number-1; i >=0 ; i--)//reverse scan, we scan from the oldest to the newest
		{
			std::string title = titleVector[i];
			std::string link = linkVector[i];
			DocumentBuilder builder;
			builder.append("title",title);
			builder.append("link",link);
			Document document = builder.obj();
			Element element;
			element.document = document;
			element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
			element.timestamp = TimestampGenerator::getCurrentTime();
			this->rssElementList.push_back(element);
		}
	}
	else
	{//we have processed some titles in this page, generate element for the new titles
		int number = titleVector.size();
		bool isProccesdNewsetTitleMeet = false;
		for( int i = number-1; i >=0 ; i--)//reverse scan, we scan from the oldest to the newest
		{
			std::string title = titleVector[i];
			std::string link = linkVector[i];
			if(isProccesdNewsetTitleMeet == true)
			{// during the scan of the titles from the oldest to the newest, the one which we have processed before is passed
			 // then the titles after are all new titles, we should generate element for them each.
				DocumentBuilder builder;
				builder.append("title",title);
				builder.append("link",link);
				Document document = builder.obj();
				Element element;
				element.document = document;
				element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
				element.timestamp = TimestampGenerator::getCurrentTime();
				element.masterTag = false;
				this->rssElementList.push_back(element);
			}

			if(title == processedNewestTitle)
			{
				isProccesdNewsetTitleMeet = true;
			}
		}
	}
	//update processedNewestTitle
	if(this->urlTitleMap[linkurl] != titleVector.front())
	{
		this->urlTitleMap[linkurl] = titleVector.front();
		isUrlTitleMapValueChanged = true;
	}

}
//write rss log file, contain urlTitleMap information
void RssStreamInput::writeLogFile(void)
{
	ofstream rsslogFile(rssLogFileName.c_str());
	if(!rsslogFile)return;
	std::map<std::string, std::string >::iterator it = urlTitleMap.begin();
	for(;it != urlTitleMap.end() ; it++)
	{
		std::string linkurl = it->first;
		std::string title =  it->second;
		rsslogFile<<linkurl<<" = "<<title<<std::endl;
	}
	rsslogFile.close();
}
//read rss log file, construct urlTitleMap data structure
void RssStreamInput::readLogFile(void)
{
	ifstream rssLogFile(rssLogFileName.c_str());
	if(!rssLogFile)return;
	std::string linkurl;
	std::string title;
	std::string s;
	while(getline(rssLogFile,s))
	{
		std::string linkurl = s.substr(0,s.find('='));
		boost::trim(linkurl);
		std::string title = s.substr(s.find('=')+1,s.length());
		boost::trim(title);
		this->urlTitleMap[linkurl] = title;
	}

	rssLogFile.close();
}
