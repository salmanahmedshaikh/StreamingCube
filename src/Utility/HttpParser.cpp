//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/HttpParser.h"
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <map>

HttpParser* HttpParser::httpParser = NULL;

HttpParser::HttpParser()
{
    //ctor
}

HttpParser::~HttpParser()
{
    //dtor
}

HttpParser* HttpParser::getInstance()
 {
	 if(httpParser==NULL)
	 {
		 httpParser = new HttpParser();
	 }
	 return httpParser;
 }


void HttpParser::parsePostRequest(std::string httpRequestStr, std::string httpPostBoundary)
{
    /*
    --Sample HTTP POST request--

    POST /command HTTP/1.1
    Host: 192.168.0.217:8085
    Connection: keep-alive
    Content-Length: 253
    User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/49.0.2623.112 Safari/537.36
    Origin: null
    Content-Type: multipart/form-data; boundary=-----------------------------19588288329222
    Accept: *
    Accept-Encoding: gzip, deflate
    Accept-Language: en-US,en;q=0.8

    -------------------------------19588288329222
    Content-Disposition: form-data; name="query"
    stream1 = readFromWrapper ("tokyoPeopleFlowStream", true); tmp1 = stream1 -> window[rows 1000] ;tmp1->istream;
    -------------------------------19588288329222--
    */

    std::string httpRequestLine;
    size_t pos1, pos2;
    std::string mapKey;
    std::string mapValue;

    std::istringstream httpRequestSS(httpRequestStr);

    std::getline (httpRequestSS, httpRequestLine);

    pos1 = httpRequestLine.find("POST /");
    pos2 = httpRequestLine.find("HTTP/");

    postRequestMap["HttpCommand"] = httpRequestLine.substr(pos1 + 6, pos2 - pos1 - 7);

    while(std::getline (httpRequestSS, httpRequestLine))

    {
        pos1 = httpRequestLine.find(":");

        if(pos1 != std::string::npos)
        {
            postRequestMap[httpRequestLine.substr(0,pos1)] = httpRequestLine.substr(pos1 + 1);
        }
        else if (httpRequestLine.find(httpPostBoundary) != std::string::npos)
        {
            std::getline (httpRequestSS, httpRequestLine);
            pos1 = httpRequestLine.find("name");

            if(pos1 != std::string::npos)
            {
                mapKey = httpRequestLine.substr(pos1 + 6, httpRequestLine.find_last_of("\"") - (pos1 + 6));
                std::getline (httpRequestSS, httpRequestLine);
                postRequestMap[mapKey] = httpRequestLine;
            }
        }
    }


    //std::map< std::string, std::string >::iterator mapIt;
    //for(mapIt = postRequestMap.begin(); mapIt != postRequestMap.end(); mapIt++)
        //std::cout << mapIt->first << " => " << mapIt->second << std::endl;
}

void HttpParser::parsePostRequestBody(std::string httpRequestStr, std::string httpPostBoundary)
{
    std::string httpRequestLine;
    size_t pos1, pos2;
    std::string mapKey;
    std::string mapValue;

    std::istringstream httpRequestSS(httpRequestStr);

    while(std::getline (httpRequestSS, httpRequestLine))
    {
        if (httpRequestLine.find(httpPostBoundary) != std::string::npos)
        {
            std::getline (httpRequestSS, httpRequestLine);

            pos1 = httpRequestLine.find("name");

            if(pos1 != std::string::npos)
            {
                mapKey = httpRequestLine.substr(pos1 + 6, httpRequestLine.find_last_of("\"") - (pos1 + 6));
                std::getline (httpRequestSS, httpRequestLine);
                postRequestMap[mapKey] = httpRequestLine;
            }
        }
    }
}

/*
--Sample HTTP GET request--

GET /command HTTP/1.1
Host: localhost:8085
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:28.0) Gecko/20100101 Firefox/28.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8
Accept-Language: en-us,ja;q=0.7,en;q=0.3
Accept-Encoding: gzip, deflate
Connection: keep-alive
*/

bool HttpParser::clearPostRequestMap()
{
    postRequestMap.clear();
    return true;
}

bool HttpParser::insertInPostRequestMap(std::string mapKey, std::string mapValue)
{
    postRequestMap[mapKey] = mapValue;
    return true;
}

std::string HttpParser::getPostRequestMapValue(std::string mapKey)
{
    std::string mapValue;
    std::map< std::string, std::string >::iterator mapIt;

    mapIt = postRequestMap.find(mapKey);

    if(mapIt != postRequestMap.end())
    {
        mapValue = postRequestMap.find(mapKey)->second;
        return mapValue;
    }
    else
    {
        return "KeyNotFound";
    }
}

