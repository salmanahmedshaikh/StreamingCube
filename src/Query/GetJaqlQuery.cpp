//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Query/GetJaqlQuery.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include<boost/unordered_map.hpp>

GetJaqlQuery * GetJaqlQuery::jaqlQuery = NULL;
GetJaqlQuery::GetJaqlQuery(void)
{
	initial();
}

GetJaqlQuery::~GetJaqlQuery(void)
{
}

GetJaqlQuery* GetJaqlQuery::getInstance(void)
{
	if(jaqlQuery==NULL)
	{
		jaqlQuery = new GetJaqlQuery();
	}
	return jaqlQuery;
}

void GetJaqlQuery::initial()
{
	//std::cout<<"reading configure files"<<std::endl;
	jaqlQueryFilePath = JAQL_QUERY_FILE_PATH;
}


std::string GetJaqlQuery::getJaqlQuery(std::string key)
{
    std::ifstream fin(jaqlQueryFilePath.c_str());
	std::string s;
	jaqlQueryMap.clear();

	while(getline(fin,s))
	{
		//std::cout<<s<<std::endl;
		std::string key = s.substr(0,s.find('='));
		boost::trim(key);
		std::string value = s.substr(s.find('=')+1,s.length());
		boost::trim(value);
		//std::cout<<key<<std::endl;
		//std::cout<<value<<std::endl;
		this->jaqlQueryMap.insert(make_pair(key,value));
	}

	boost::associative_property_map< boost::unordered_map<std::string, std::string> > jaqlQueryMap(this->jaqlQueryMap);

	return jaqlQueryMap[key];

}
/*
boost::unordered_map<std::string, std::string> GetJaqlQuery::getAllQueries(void)
{
	return this->configuration;
}
*/
