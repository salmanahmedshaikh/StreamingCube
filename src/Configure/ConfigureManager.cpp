//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Configure/ConfigureManager.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include<boost/unordered_map.hpp>
ConfigureManager * ConfigureManager::configure = NULL;

ConfigureManager::ConfigureManager(void)
{
	initial();
}

ConfigureManager::~ConfigureManager(void)
{
}

ConfigureManager* ConfigureManager::getInstance(void)
{
	if(configure==NULL)
	{
		configure = new ConfigureManager();
	}
	return configure;
}

void ConfigureManager::initial()
{
	std::string configureFilePath = CONFIGURE_FILE_PATH;
	std::ifstream fin(configureFilePath.c_str());
	std::string s;

	while(getline(fin,s))
	{
		// ignoring commented lines
		if(s.find('#') != std::string::npos)
            continue;

		std::string key = s.substr(0,s.find('='));
		boost::trim(key);
		std::string value = s.substr(s.find('=')+1,s.length());
		boost::trim(value);

		this->configuration.insert(make_pair(key,value));
	}
}



std::string ConfigureManager::getConfigureValue(std::string key)
{
	boost::associative_property_map< boost::unordered_map<std::string, std::string> > configuration(this->configuration);
	return configuration[key];

}

boost::unordered_map<std::string, std::string> ConfigureManager::getAllConfigureValue(void)
{
	return this->configuration;
}
