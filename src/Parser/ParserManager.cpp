//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "../Parser/ParserManager.h"
#include "../BinaryJson/json.h"
//#pragma comment(lib,"ws2_32.lib")
ParserManager * ParserManager::parserManager = NULL;
ParserManager::ParserManager(void)
{
	init();
}


ParserManager::~ParserManager(void)
{
}

ParserManager * ParserManager::getInstance()
{
	if(parserManager == NULL)
	{
		parserManager = new ParserManager();
	}
	return parserManager;
}
void ParserManager::init()
{

}



boost::shared_ptr<QueryIntermediateRepresentation>  ParserManager::processQuery(std::string jaqlQueryString)
{

	int sClient;
	struct sockaddr_in server;
	char szMessage[1024*10];
	int ret;

	strcpy(szMessage,jaqlQueryString.c_str());

	if ((sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
	{
		perror("socket creation error");
		exit(1);
	}
	memset(&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = PF_INET;
	server.sin_port = htons(ParserManager::parserServerPort);
	server.sin_addr.s_addr = inet_addr( "127.0.0.1");
	if(connect(sClient, (struct sockaddr *) &server, sizeof(struct sockaddr_in))<0)
	{
		perror("failed to connect to the parser server");
		exit(1);
	}
	if(send(sClient, szMessage, strlen(szMessage), 0)<0)
	{
		perror("send error");
		exit(1);
	}
	int retByte ;
	if((retByte= recv(sClient,szMessage,1024*10,0))<0)
	{
		perror("recv error");
		exit(1);
	}
	szMessage[retByte] = '\0';

	std::string queryIntermediateRepresentationString (szMessage,1,retByte-2);
	close(sClient);
	//std::cout <<"Query Intermediate: " << std::endl << queryIntermediateRepresentationString<<std::endl;
	Document queryDocumentArray = fromjson(queryIntermediateRepresentationString);
	boost::shared_ptr<QueryIntermediateRepresentation> retQueryIntermediateRepresentation(new QueryIntermediateRepresentation(queryDocumentArray));
	return retQueryIntermediateRepresentation;

}
