//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "CPUUtilization.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <iostream>

unsigned long long CPUUtilization::lastTotalUser = 0;
unsigned long long CPUUtilization::lastTotalUserLow = 0;
unsigned long long CPUUtilization::lastTotalSys = 0;
unsigned long long CPUUtilization::lastTotalIdle = 0;

CPUUtilization::CPUUtilization()
{}

CPUUtilization::~CPUUtilization()
{}

void CPUUtilization::init()
{
    FILE* file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %Ld %Ld %Ld %Ld", &CPUUtilization::lastTotalUser, &CPUUtilization::lastTotalUserLow, &CPUUtilization::lastTotalSys, &CPUUtilization::lastTotalIdle);
    fclose(file);
}


double CPUUtilization::getCurrentValue()
{
    double percent;
    FILE* file;
    unsigned long long totalUser = 0, totalUserLow = 0, totalSys = 0, totalIdle = 0, total = 0;


    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %Ld %Ld %Ld %Ld", &totalUser, &totalUserLow, &totalSys, &totalIdle);
    fclose(file);


    if (totalUser < CPUUtilization::lastTotalUser || totalUserLow < CPUUtilization::lastTotalUserLow || totalSys < CPUUtilization::lastTotalSys || totalIdle < CPUUtilization::lastTotalIdle)
    {
        //Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else{
        total = (totalUser - CPUUtilization::lastTotalUser) + (totalUserLow - CPUUtilization::lastTotalUserLow) + (totalSys - CPUUtilization::lastTotalSys);
        percent = total;
        total += (totalIdle - CPUUtilization::lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    std::cout << "totalSys " << totalSys << std::endl;
    std::cout << "totalIdle " << totalIdle << std::endl;

    CPUUtilization::lastTotalUser = totalUser;
    CPUUtilization::lastTotalUserLow = totalUserLow;
    CPUUtilization::lastTotalSys = totalSys;
    CPUUtilization::lastTotalIdle = totalIdle;

    return percent;
}
