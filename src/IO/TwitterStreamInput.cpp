//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/TwitterStreamInput.h"
#include "../IO/IStreamInput.h"
#include "../Internal/Element/Element.h"
#include "../Wrapper/TwitterWrapper/twitcurl.h"
#include "../Internal/Buffer/BufferConstraint.h"
#include "../BinaryJson/json.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#ifdef WINDOWS
#include <process.h>
#endif
#include <iostream>
#include <boost/thread.hpp>

std::string TwitterStreamInput::userName = "";
std::string TwitterStreamInput::userPassword = "";
TwitterStreamInput::TwitterStreamInput(std::string userNameInput, std::string userPasswordInput, boost::shared_ptr<JsonSchema>jsonSchema):IStreamInput(jsonSchema)
{
	 userName = userNameInput;
	 userPassword = userPasswordInput;
}


TwitterStreamInput::~TwitterStreamInput(void)
{
}
//the call back function whenever new data arrives from the twitter
size_t TwitterStreamInput::getTweetCallBackFunction(void *ptr, size_t size, size_t nmemb, void *callbackargument)
{
	static long start = -1;
	long end;
	if(start<0)
	{
		start=clock();

	}
	end=clock();
	long total_second=(end-start)/1000;
	static int total_number = 0;
	total_number++;
	if(total_second>0)
	{
		long temp = total_second/100;
		if(total_second == temp*100)
		{
			std::cout<<"tuple/per second: "<<total_number/total_second<<std::endl;
		}

	}
	TwitterStreamInput* twitterStreamInput = (TwitterStreamInput*)callbackargument;
	//std::cout<<(char*)(ptr)<<std::endl<<std::endl<<std::endl;
	char* str = (char*) ptr;

	//boost::mutex::scoped_lock lock(twitterStreamInput->m_oMutex);
	twitterStreamInput->tweetStringList.push_back(std::string(str, size*nmemb-2));
	//we just save the string in this thread for performance reason
	//we don't analyze the string, the analyzation is done in the main thread


	return size*nmemb;

}
//
//void TwitterStreamInput::pushTweetElement(Element tweetElement)
//{
//
//	this->tweetElementList.push_back(tweetElement);
//}

//the tweet string is saved in the tweetstringlist
//the tweet elemeng is saved in the tweetelementlist
//we pop string from tweetstringlist and try to generate element pushed to the tweetelementlist
bool TwitterStreamInput::isEmpty()
{
	//boost::mutex::scoped_lock lock(m_oMutex);
	if(tweetElementList.size()>0)
	{// if there is something in the tweetelementlist, it is not empty
		return false;
	}
	//if there is nothing in the tweetelementlist, we will look into the tweetstringlist



	while(1)
	{
		if(this->tweetStringList.size()==0)
		{//if nothing is in the tweetstringlist, it is empty
			return true;
		}
		//if something is in the tweetstringlist,we try to generate an element
		std::string& jsonDocumentString = this->tweetStringList.front();
		Document document = fromjson(jsonDocumentString);
		Element element;
		element.document = document;
		element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
		element.mark = PLUS_MARK;
		element.timestamp = TimestampGenerator::getCurrentTime();
		element.masterTag = false;
		if(element.getSize()<=CHUNK_SIZE-CHUNK_HEADER_MAX_LENGTH)
		{//if the element size is proper, it is an element we want
			tweetElementList.push_back(element);
			return false;
		}
		//if the element size is too long, we should look to the next string
	}



}
void TwitterStreamInput::initial(void)
{
	//boost::thread twitterThread(connectTwitterServer, 0, (void *)this);
	boost::thread twitterThread(boost::bind(&TwitterStreamInput::connectTwitterServer,this));
	twitterThread.join();
}
void TwitterStreamInput::getNextElement(Element& element)
{
	assert(!isEmpty());
	element = this->tweetElementList.front();
	this->tweetElementList.pop_front();
}
void TwitterStreamInput::connectTwitterServer(void* arg)
{
	TwitterStreamInput *twitterStreamInput  = (TwitterStreamInput *)arg;

	/* Get username and password from command line args */
	std::string userName = TwitterStreamInput::userName;
	std::string passWord = TwitterStreamInput::userPassword;


	twitCurl twitterObj;
	std::string tmpStr, tmpStr2;
	std::string replyMsg;
	char tmpBuf[1024];

	if(userName == ""|userPassword=="")
	{
		memset( tmpBuf, 0, 1024 );
		printf( "\nEnter twitter username: " );
		gets( tmpBuf );
		userName =  tmpBuf ;

		memset( tmpBuf, 0, 1024 );
		printf( "\nEnter twitter password: " );
		gets( tmpBuf );
		passWord = tmpBuf ;
	}


	/* Set twitter username and password */
	twitterObj.setTwitterUsername( userName );
	twitterObj.setTwitterPassword( passWord );

	/* Set proxy server usename, password, IP and port (if present) */
	memset( tmpBuf, 0, 1024 );
	printf( "\nDo you have a proxy server configured (0 for no; 1 for yes): " );
	gets( tmpBuf );
	tmpStr = tmpBuf;
	if( std::string::npos != tmpStr.find( "1" ) )
	{
		memset( tmpBuf, 0, 1024 );
		printf( "\nEnter proxy server IP: " );
		gets( tmpBuf );
		tmpStr = tmpBuf;
		twitterObj.setProxyServerIp( tmpStr );

		memset( tmpBuf, 0, 1024 );
		printf( "\nEnter proxy server port: " );
		gets( tmpBuf );
		tmpStr = tmpBuf;
		twitterObj.setProxyServerPort( tmpStr );

		memset( tmpBuf, 0, 1024 );
		printf( "\nEnter proxy server username: " );
		gets( tmpBuf );
		tmpStr = tmpBuf;
		twitterObj.setProxyUserName( tmpStr );

		memset( tmpBuf, 0, 1024 );
		printf( "\nEnter proxy server password: " );
		gets( tmpBuf );
		tmpStr = tmpBuf;
		twitterObj.setProxyPassword( tmpStr );
	}

	/* OAuth flow begins */
	/* Step 0: Set OAuth related params. These are got by registering your app at twitter.com */
	twitterObj.getOAuth().setConsumerKey( std::string( "vlC5S1NCMHHg8mD1ghPRkA" ) );
	twitterObj.getOAuth().setConsumerSecret( std::string( "3w4cIrHyI3IYUZW5O2ppcFXmsACDaENzFdLIKmEU84" ) );

	/* Step 1: Check if we alredy have OAuth access token from a previous run */
	std::string myOAuthAccessTokenKey("");
	std::string myOAuthAccessTokenSecret("");
	std::ifstream oAuthTokenKeyIn;
	std::ifstream oAuthTokenSecretIn;

	oAuthTokenKeyIn.open( "twitterClient_token_key.txt" );
	oAuthTokenSecretIn.open( "twitterClient_token_secret.txt" );

	memset( tmpBuf, 0, 1024 );
	oAuthTokenKeyIn >> tmpBuf;
	myOAuthAccessTokenKey = tmpBuf;

	memset( tmpBuf, 0, 1024 );
	oAuthTokenSecretIn >> tmpBuf;
	myOAuthAccessTokenSecret = tmpBuf;

	oAuthTokenKeyIn.close();
	oAuthTokenSecretIn.close();

	if( myOAuthAccessTokenKey.size() && myOAuthAccessTokenSecret.size() )
	{
		/* If we already have these keys, then no need to go through auth again */
		printf( "\nUsing:\nKey: %s\nSecret: %s\n\n", myOAuthAccessTokenKey.c_str(), myOAuthAccessTokenSecret.c_str() );

		twitterObj.getOAuth().setOAuthTokenKey( myOAuthAccessTokenKey );
		twitterObj.getOAuth().setOAuthTokenSecret( myOAuthAccessTokenSecret );
	}
	else
	{
		/* Step 2: Get request token key and secret */
		std::string authUrl;
		twitterObj.oAuthRequestToken( authUrl );

		/* Step 3: Get PIN  */
		memset( tmpBuf, 0, 1024 );
		printf( "\nDo you want to visit twitter.com for PIN (0 for no; 1 for yes): " );
		gets( tmpBuf );
		tmpStr = tmpBuf;
		if( std::string::npos != tmpStr.find( "1" ) )
		{
			/* Ask user to visit twitter.com auth page and get PIN */
			memset( tmpBuf, 0, 1024 );
			printf( "\nPlease visit this link in web browser and authorize this application:\n%s", authUrl.c_str() );
			printf( "\nEnter the PIN provided by twitter: " );
			gets( tmpBuf );
			tmpStr = tmpBuf;
			twitterObj.getOAuth().setOAuthPin( tmpStr );
		}
		else
		{
			/* Else, pass auth url to twitCurl and get it via twitCurl PIN handling */
			twitterObj.oAuthHandlePIN( authUrl );
		}

		/* Step 4: Exchange request token with access token */
		twitterObj.oAuthAccessToken();

		/* Step 5: Now, save this access token key and secret for future use without PIN */
		twitterObj.getOAuth().getOAuthTokenKey( myOAuthAccessTokenKey );
		twitterObj.getOAuth().getOAuthTokenSecret( myOAuthAccessTokenSecret );

		/* Step 6: Save these keys in a file or wherever */
		std::ofstream oAuthTokenKeyOut;
		std::ofstream oAuthTokenSecretOut;

		oAuthTokenKeyOut.open( "twitterClient_token_key.txt" );
		oAuthTokenSecretOut.open( "twitterClient_token_secret.txt" );

		oAuthTokenKeyOut.clear();
		oAuthTokenSecretOut.clear();

		oAuthTokenKeyOut << myOAuthAccessTokenKey.c_str();
		oAuthTokenSecretOut << myOAuthAccessTokenSecret.c_str();

		oAuthTokenKeyOut.close();
		oAuthTokenSecretOut.close();
	}
	/* OAuth flow ends */

	/* Account credentials verification */
	if( twitterObj.accountVerifyCredGet() )
	{
		twitterObj.getLastWebResponse( replyMsg );
		printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet web response:\n%s\n", replyMsg.c_str() );
	}
	else
	{
		twitterObj.getLastCurlError( replyMsg );
		printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet error:\n%s\n", replyMsg.c_str() );
	}

	replyMsg = "";
	printf( "\nGetting stream sample twitter articles\n" );


	while(1)
	{
		if(twitterObj.sampleStreamGet(getTweetCallBackFunction,(void*)twitterStreamInput)!=1)
		{
#ifdef WINDOWS
			Sleep(5000);
#endif
		}
	}
	assert(0);//not reached

}
