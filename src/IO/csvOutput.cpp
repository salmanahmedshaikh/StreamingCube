//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/csvOutput.h"
#include <fstream>

int csvOutput::totalOutputElementsCounter = 0;
int csvOutput::outputFileName = 1;

csvOutput::csvOutput(std::string csvOutputFilePath)
{
    this->csvFilePath = csvOutputFilePath; // + "1234.csv";
}

csvOutput::~csvOutput(void)
{
}

void csvOutput::pushNext(Element& element)
{
	std::stringstream outputTupleSS;
	totalOutputElementsCounter++;

	int pid;
	double longitude;
	double latitude;
	int sex;
	int age;
	int padd;
	int work;
	int purpose;
	int datum; // transportation method

	std::set<std::string> documentFieldNames;
	element.document.getFieldNames(documentFieldNames);

	element.document.getField("pid").Val(pid);
	element.document.getField("longitude").Val(longitude);
	element.document.getField("latitude").Val(latitude);
	element.document.getField("sex").Val(sex);
	element.document.getField("age").Val(age);
	element.document.getField("padd").Val(padd);
	element.document.getField("work").Val(work);
	element.document.getField("purpose").Val(purpose);
	element.document.getField("datum").Val(datum);

	//std::cout << element << std::endl;

	if(pid > 0)
	{
        outputTupleSS << pid;
    }

    if(longitude > 0.1 && longitude < 400.000)
	{
        outputTupleSS << "," << longitude;
    }

    if(latitude > 0.1 && latitude < 200.000)
	{
        outputTupleSS << "," << latitude;
    }

    if(sex == 1 || sex == 2)
	{
        outputTupleSS << "," << sex;
    }

    if(age > 0 && age < 1000)
	{
        outputTupleSS << "," << age;
    }

    if(padd > 0 && padd < 99999999)
	{
        outputTupleSS << "," << padd;
    }

    if(work > 0 && work < 1000)
	{
        outputTupleSS << "," << work;
    }

    if(purpose > 0 && purpose < 1000)
	{
        outputTupleSS << "," << purpose;
    }

    if(datum > 0 && datum < 1000)
	{
        outputTupleSS << "," << datum << "\n";
    }
    else if(pid > 0)
    {
        outputTupleSS << "\n";
    }


    csvFile << outputTupleSS.str();

	/*
	std::cout << pid << std::endl;
	std::cout << longitude << std::endl;
	std::cout << latitude << std::endl;
	std::cout << sex << std::endl;
	std::cout << age << std::endl;
	std::cout << work << std::endl;
	*/

	//std::cout<<element.document.getField("longitude").Val(longitude)<<std::endl;
	//std::cout<<element.document.getField("latitude").Val(latitude)<<std::endl;
	//std::cout<<element.document.getField("sex").Val(sex)<<std::endl;
	//std::cout<<element.document.getField("age").Val(age)<<std::endl;
	//std::cout<<element.document.getField("work").Val(work)<<std::endl;
}

bool csvOutput::isFull(void)
{
	return false;
}

void csvOutput::initial()
{
}

void csvOutput::openFile(int csvFileName)
{
    std::stringstream csvFileNameSS;

    //std::cout << "csvOutput::openFile called." << std::endl;

    /*
    if(csvFileName == 0)
        csvFileNameSS << this->csvFilePath << "0000.csv";
    else if (csvFileName >= 1 && csvFileName < 10)
        csvFileNameSS << this->csvFilePath << "000" << csvFileName << ".csv";
    else if (csvFileName >= 10 && csvFileName < 100)
        csvFileNameSS << this->csvFilePath << "00" << csvFileName << ".csv";
    else if (csvFileName >= 100 && csvFileName < 1000)
        csvFileNameSS << this->csvFilePath << "0" << csvFileName << ".csv";
    else
        csvFileNameSS << this->csvFilePath << csvFileName << ".csv";
    */

    // Change the file extension from .tmp to .csv
    if(csvOutput::outputFileName > 1)
    {
        std::stringstream oldFileName;
        std::stringstream newFileName;

        oldFileName << this->csvFilePath << csvOutput::outputFileName - 1 << ".tmp";
        newFileName << this->csvFilePath << csvOutput::outputFileName - 1 << ".csv";

        if(rename((oldFileName.str()).c_str(), (newFileName.str()).c_str()) != 0)
            std::cout << "Could not rename output file" << std::endl;
    }

    csvFileNameSS << this->csvFilePath << csvOutput::outputFileName << ".tmp";
    csvOutput::outputFileName++;

    std::cout << "Output File Name " << (csvFileNameSS.str()).c_str() << std::endl;

    csvFile.open((csvFileNameSS.str()).c_str());

    csvFileNameSS.str( std::string() );
    csvFileNameSS.clear();
}

void csvOutput::closeFile(void)
{
    csvFile.close();
}

int csvOutput::getTotalOutputElementsCount(void)
{
    return totalOutputElementsCounter;
}
