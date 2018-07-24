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
 * read configuration file of server
 */
static const std::string CONFIGURE_FILE_PATH = "./configure/JStreamSpinner.conf";

class ConfigureManager:private boost::noncopyable
{
private:
	boost::unordered_map<std::string, std::string> configuration;

	static ConfigureManager * configure;
	ConfigureManager(void);
	void initial(void);

public:
	static ConfigureManager* getInstance(void);
	~ConfigureManager(void);

	std::string getConfigureValue(std::string key);
	boost::unordered_map<std::string, std::string> getAllConfigureValue(void);
};

