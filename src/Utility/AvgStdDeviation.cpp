//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Utility/AvgStdDeviation.h"
#include <iostream>

AvgStdDeviation* AvgStdDeviation::avgStdDeviation = NULL;

AvgStdDeviation::AvgStdDeviation(void)
{
}

AvgStdDeviation::~AvgStdDeviation(void)
{
}

AvgStdDeviation* AvgStdDeviation::getInstance()
 {
	 if(avgStdDeviation==NULL)
	 {
		 avgStdDeviation = new AvgStdDeviation();
	 }
	 return avgStdDeviation;
 }

/*
template <typename T> double AvgStdDeviation::getAverage (std::vector<T> values)
{
    typename std::vector<T>::iterator valuesIt;
    T sum = 0;
    for(valuesIt = values.begin(); valuesIt != values.end(); valuesIt++)
    {
        sum += (*valuesIt);
    }

    double avg = (sum*1.0)/values.size();
    std::cout << "avg " << avg << " | sum " << sum << std::endl;

    return avg;
}
*/

