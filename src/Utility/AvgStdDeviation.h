//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
class AvgStdDeviation
{
private:
    static AvgStdDeviation* avgStdDeviation;

public:
	AvgStdDeviation(void);
	~AvgStdDeviation(void);
	static AvgStdDeviation* getInstance();

	//template <typename T> double getAverage(std::vector<T> values);
	template <typename T>
	double getAverage (std::vector<T> values)
    {
        typename std::vector<T>::iterator valuesIt;
        T sum = 0.0;
        for(valuesIt = values.begin(); valuesIt != values.end(); valuesIt++)
        {
            sum += (*valuesIt);
        }

        double avg = (sum*1.0)/values.size();
        return avg;
    }

    template <typename T>
	double getStdDeviation (std::vector<T> values)
    {
        typename std::vector<T>::iterator valuesIt;
        T sum = 0.0;
        for(valuesIt = values.begin(); valuesIt != values.end(); valuesIt++)
        {
            sum += (*valuesIt);
        }

        double avg = (sum*1.0)/values.size();
        double standardDeviation = 0.0;

        for(valuesIt = values.begin(); valuesIt != values.end(); valuesIt++)
            standardDeviation += pow((*valuesIt) - avg, 2);

        return sqrt(standardDeviation / values.size());
    }
};

