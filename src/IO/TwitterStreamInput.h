//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include "../Schema/JsonSchema.h"
#include "../IO/IStreamInput.h"
#include "../Common/Types.h"
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
/* this use the twistcurl library to deal with the Twitter API */
class TwitterStreamInput: public IStreamInput
{
private:

	void getNextElement(Element& element) ;
	static std::string userName;
	static std::string userPassword;
	std::list<Element> tweetElementList;
	std::list<std::string> tweetStringList;
	static void connectTwitterServer(void*arg) ;
	boost::mutex m_oMutex;
	//the call back function whenever new data arrives from the twitter
	static size_t getTweetCallBackFunction(void *ptr, size_t size, size_t nmemb, void *callbackargument);
public:

	TwitterStreamInput(std::string userNameInput, std::string userPasswordInput, boost::shared_ptr<JsonSchema>jsonSchema);
	~TwitterStreamInput(void);
	bool isEmpty() ;
	void initial() ;
	//void pushTweetElement(Element tweetElement);

};

