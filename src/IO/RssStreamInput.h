//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../IO/IStreamInput.h"
#include "../Internal/Element/Element.h"

// a sample of what we get from the RSS source
//<rss version="2.0">
//	<channel>
//	<title>sample</title>
//	<link>http://www.bilibili.tv/video/bangumi.html</link>   -> we tell this "linkurl" later
//	<item>
//		<title><![CDATA[a sample title]]></title>
//		<link>http://www.bilibili.tv/video/av644092/</link>
//	</item>
//	<item>
//	    ......
//	</item>
//</channel>
//</rss>
class RssStreamInput: public IStreamInput
{
private:

	//a set of all the urls of the RSS resource
	std::vector<std::string> urlVector;

	//a set of all the urls' corresponding encoding
	std::vector<std::string> encodingVector;


	//when we get the content of one RSS url, we may able to get (1)several items with a schema of {"title":"","link":""} and (2) a linkurl of these items
	//how to ensure whether we have processed one item or not?
	//The answer is we maintain the title of the latest tile belonging to each linkurl.
	//We save this information in the map <linkurl, title>
	std::map<std::string, std::string > urlTitleMap;
	// record whether the urlTitleMap value is changed since we read it from the file,
	// if changed, we should write it in the file.
	bool isUrlTitleMapValueChanged;
	//the element list of rss document
	std::list<Element> rssElementList;

	//scan each url of the RSS source, get the content of the url
	//check if any update, if yes, create element and add to the elementlist
	void checkUpdataOfRssResource(void);

	//this function would be called when gets the content of the RSS url
	static size_t getRssCallBackFunction(void *buffer, size_t size, size_t nmemb, void *userp);

	void getNextElement(Element& element) ;

	//parse the RSS feed which is xml, get each items, generate element for new item and insert into elementList
	void generateElementByRss(std::string& rssXmlContent);

	//write rss log file, contain urlTitleMap information
	void writeLogFile(void);
	//read rss log file, construct urlTitleMap data structure
	void readLogFile(void);

	//connect all RSS urls and see if new titles, if yes, generate new element
	void processRSS(void);
public:
	RssStreamInput(std::vector<std::string> urlVector, std::vector<std::string> encodingVector ,boost::shared_ptr<JsonSchema>jsonSchema);
	~RssStreamInput(void);
	bool isEmpty() ;
	void initial() ;
};

