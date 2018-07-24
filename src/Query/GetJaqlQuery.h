//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include <boost/property_map/property_map.hpp>
#include <boost/noncopyable.hpp>
#include<boost/unordered_map.hpp>
/*
 * read jaql query file
 */
static const std::string JAQL_QUERY_FILE_PATH = "./configure/jaqlQuery.conf";
//static const std::string JAQL_QUERY_FILE_PATH = "/home/jsspinner/Dropbox/public/peopleMoveByTime/jaqlQuery.txt";

class GetJaqlQuery:	private boost::noncopyable
{
private:
	boost::unordered_map<std::string, std::string> jaqlQueryMap;
	std::string jaqlQueryFilePath;

	static GetJaqlQuery * jaqlQuery;
	GetJaqlQuery(void);
	void initial(void);

public:
	static GetJaqlQuery* getInstance(void);
	~GetJaqlQuery(void);

	std::string getJaqlQuery(std::string key);
	//boost::unordered_map<std::string, std::string> getAllQueries(void);
};

