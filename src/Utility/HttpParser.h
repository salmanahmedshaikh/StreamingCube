//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"

class HttpParser
{
    private:
        static HttpParser* httpParser;
        std::map< std::string, std::string > postRequestMap;

    public:
        HttpParser();
        ~HttpParser();
        static HttpParser* getInstance();
        void parsePostRequest(std::string, std::string);
        void parsePostRequestBody(std::string, std::string);
        bool clearPostRequestMap();
        std::string getPostRequestMapValue(std::string);
        bool insertInPostRequestMap(std::string, std::string);

};
