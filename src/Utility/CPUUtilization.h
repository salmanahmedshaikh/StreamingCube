//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////


class CPUUtilization
{
    public:
        CPUUtilization();
        ~CPUUtilization();

        void init();
        double getCurrentValue();

    protected:
    private:
        static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

};
