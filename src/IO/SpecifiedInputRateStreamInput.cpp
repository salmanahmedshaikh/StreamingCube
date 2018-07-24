//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/SpecifiedInputRateStreamInput.h"
#include "../Schema/SchemaInterpreter.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Utility/SleepUtility.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <time.h>
#include "../Configure/ConfigureManager.h"
//int SpecifiedInputRateStreamInput::ignoredInputNumber = 0;

SpecifiedInputRateStreamInput::SpecifiedInputRateStreamInput(boost::shared_ptr<JsonSchema> schema) : IStreamInput(schema)
{
	this->schema = schema;
	SchemaInterpreter::generateRandomDocumentBySchema(this->schema,	this->bufferedDocument);
	this->bufferedElementNumber = 0;
	this->totalElementNumber = 0;
}

SpecifiedInputRateStreamInput::~SpecifiedInputRateStreamInput(void) {
	//	std::cout<<this->getId()<<" total element number : "<< totalElementNumber<<std::endl;
}

void SpecifiedInputRateStreamInput::getNextElement(Element& element) {
	//	boost::recursive_mutex::scoped_lock lock(m_oMutex);
	//	this->bufferedElementNumber--;
	//	assert(this->bufferedElementNumber>=0);
	//	lock.unlock();
	//	assert(bufferedElementNumber>0);

	bufferedElementNumber--;
	element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	element.mark = PLUS_MARK;
	element.timestamp = TimestampGenerator::getCurrentTime();
	//element.document = this->bufferedDocument;

	if (this->getId() == "performanceTestStream1") {
		DocumentBuilder builder;
		builder.append("A", "hello");
		builder.append("B", 1);
		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "performanceTestStream2") {
		DocumentBuilder builder;
		//builder.append("A",rand()%10);
		builder.append("A", "hello");
		builder.append("C", 1);
		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "performanceTestStream3") {
		DocumentBuilder builder;
		builder.append("A", "hello");
		builder.append("D", 1);
		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "performanceTestStream4") {
		DocumentBuilder builder;
		builder.append("A", "hello");
		builder.append("E", 1);
		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "salesFact") {
		//SchemaInterpreter::generateRandomDocumentBySchema(this->schema,	this->bufferedDocument);
		//element.document = this->bufferedDocument;
		DocumentBuilder builder;
		builder.append("prodID", 125);
		builder.append("suppID", 130);
		builder.append("salesAmountFACT", 100);
		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "salesFact5Dim") {
		//SchemaInterpreter::generateRandomDocumentBySchema(this->schema,	this->bufferedDocument);
		//element.document = this->bufferedDocument;
		DocumentBuilder builder;

		/* initialize random seed: */

		//Formula: min + (rand() % (int)(max - min + 1))
		int randInt = 125 + (rand() % (int)(10));
		builder.append("prodID", randInt);

		randInt = 125 + (rand() % (int)(10));
		builder.append("suppID", randInt);

		randInt = 140 + (rand() % (int)(10));
		builder.append("promoID", randInt);

		builder.append("custID", 150);
		builder.append("storeID", 160);

		randInt = 100 + (rand() % (int)(900));
		builder.append("salesAmountFACT", randInt);

		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "salesFact5DimWNamesWrapper")
	{
		//sleep(1);
		DocumentBuilder builder;

		//Formula: min + (rand() % (int)(max - min + 1))
		int randInt = 1 + (rand() % (int)(250));
		builder.append("prodID", randInt);

		randInt = 1 + (rand() % (int)(4));
		builder.append("suppID", randInt);

		randInt = 1 + (rand() % (int)(5));
		builder.append("promoID", randInt);

		std::stringstream randDim;

        randInt = 1 + (rand() % (int)(10));
		builder.append("custID", randInt);
		randDim << "Customer" << randInt;
		builder.append("custName", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(2));
		builder.append("storeID", randInt);
		randDim << "Store" << randInt;
		builder.append("storeArea", randDim.str());

		/*
		builder.append("custID", 150);
		builder.append("custName", "Suzuki");

		builder.append("storeID", 160);
		builder.append("storeArea", "Tsukuba");
		*/

		randInt = 100 + (rand() % (int)(900));
		builder.append("salesAmountFACT", randInt);

		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "salesFact6DimWNamesWrapper")
	{
		//sleep(1);
		DocumentBuilder builder;

		//Formula: min + (rand() % (int)(max - min + 1))
		int randInt = 1 + (rand() % (int)(250));
		builder.append("prodID", randInt);

		randInt = 1 + (rand() % (int)(4));
		builder.append("suppID", randInt);

		randInt = 1 + (rand() % (int)(5));
		builder.append("promoID", randInt);

		std::stringstream randDim;

        randInt = 1 + (rand() % (int)(7));
		builder.append("custID", randInt);
		randDim << "Customer" << randInt;
		builder.append("custName", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(2));
		builder.append("storeID", randInt);
		randDim << "Store" << randInt;
		builder.append("storeArea", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(4));
		builder.append("salesPersonID", randInt); // SalesPerson
		randDim << "SP" << randInt;
		builder.append("salesPersonName", randDim.str());

		/*
		builder.append("custID", 150);
		builder.append("custName", "Suzuki");

		builder.append("storeID", 160);
		builder.append("storeArea", "Tsukuba");
		*/

		randInt = 100 + (rand() % (int)(900));
		builder.append("salesAmountFACT", randInt);

		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "salesFactPreJoined6DWrapper")
	{
		//sleep(1);
		DocumentBuilder builder;
		std::stringstream randDim;

		//Formula: min + (rand() % (int)(max - min + 1))
		int randInt = 1 + (rand() % (int)(250));
		builder.append("prodID", randInt);
		randDim << "Product" << randInt;
		builder.append("prodName", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(4));
		builder.append("suppID", randInt);
		randDim << "Supplier" << randInt;
		builder.append("suppName", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(5));
		builder.append("promoID", randInt);
		randDim << "Promotion" << randInt;
		builder.append("promoName", randDim.str());

		randDim.clear();
        randDim.str(std::string());

        randInt = 1 + (rand() % (int)(7));
		builder.append("custID", randInt);
		randDim << "Customer" << randInt;
		builder.append("custName", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(2));
		builder.append("storeID", randInt);
		randDim << "Store" << randInt;
		builder.append("storeArea", randDim.str());

		randDim.clear();
        randDim.str(std::string());

		randInt = 1 + (rand() % (int)(4));
		builder.append("salesPersonID", randInt); // SalesPerson
		randDim << "SP" << randInt;
		builder.append("salesPersonName", randDim.str());

		/*
		builder.append("custID", 150);
		builder.append("custName", "Suzuki");

		builder.append("storeID", 160);
		builder.append("storeArea", "Tsukuba");
		*/

		randInt = 100 + (rand() % (int)(900));
		builder.append("salesAmountFACT", randInt);

		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "salesFact4DimTPCH")
	{
		//sleep(1);
		DocumentBuilder builder;

		//Formula: min + (rand() % (int)(max - min + 1))
		int randInt = 1 + (rand() % (int)(2000));
		builder.append("partKey", randInt);

		randInt = 1 + (rand() % (int)(100));
		builder.append("suppKey", randInt);

		randInt = 1 + (rand() % (int)(1500));
		builder.append("custKey", randInt);

		builder.append("storeKey", 150);
		builder.append("storeArea", "Japan");

		randInt = 100 + (rand() % (int)(900));
		builder.append("salesQtyFACT", randInt);

		Document obj = builder.obj();
		element.document = obj;
	}
	if (this->getId() == "eventStream")
    {}
	if (this->getId() == "performanceTestStreamGroup")
	{
		SchemaInterpreter::generateRandomDocumentBySchema(this->schema,	this->bufferedDocument);
		element.document = this->bufferedDocument;
	}

	element.document.getOwned();
	//std::cout << "element from specifiedInputRate" << element << std::endl;
}

bool SpecifiedInputRateStreamInput::isEmpty() {
	//	boost::recursive_mutex::scoped_lock lock(m_oMutex);
	//	bool bl = false;
	//	if(bufferedElementNumber==0)
	//	{
	//	bl = true;
	//	}
	//	lock.unlock();
	//	return bl;
	return bufferedElementNumber == 0;
}

void SpecifiedInputRateStreamInput::initial() {
	// A new thread is generated for each data stream source
	//std::cout << "SpecifiedInputRateStreamInput thread initialization" << std::endl;

	int sleepMiliSeconds = 20;
	//std::cout<<"creating one thread+++++++++++++++"<<std::endl;
	boost::function<void(int)> memberFunctionWrapper(boost::bind(
			&SpecifiedInputRateStreamInput::generateInput, this, _1));
	boost::thread downloadThread((boost::bind(memberFunctionWrapper,
			sleepMiliSeconds)));

}

void SpecifiedInputRateStreamInput::generateInput(int sleepMicroSeconds) {
	//std::string inputRateStr = ConfigureManager::getInstance()->getConfigureValue("stream1_input_rate");
	//double inputRate = 0.0;
	//inputRate = atof(inputRateStr.c_str());

	while (1) {

		if (this->getId() == "performanceTestStream1") {
			//boost::recursive_mutex::scoped_lock lock(m_oMutex);
			this->bufferedElementNumber++;
			//lock.unlock();
			this->totalElementNumber++;
			std::stringstream ss;
			//
			std::string inputRateStr = ConfigureManager::getInstance()->getConfigureValue("stream1_input_rate");
			double inputRate = 0.0;
			inputRate = atof(inputRateStr.c_str());
			//std::cout << this->getId() << " inputRate " << inputRate << std::endl;

			//ss << inputRateStr;
			//std::cout << this->getId() << " inputRate " << inputRateStr.c_str() << std::endl;
			//inputRate = boost::lexical_cast<int>(ss);
			//inputRate = atof(inputRateStr.c_str());
			//ss >> inputRate;

			// For the sake of distributed experiments only
			static int counter = 0;
            if(inputRate < 10000 & counter < 50)
            {
                    counter++;
                    inputRate = 10000;
                    //std::cout << "Modified for the sake of distributed proc. only " << counter << std::endl;
            }


			if (inputRate == 0.11) {
				SleepUtility::sleepMicroSecond(9000000);//1  tuple/9s
			} else if (inputRate == 0.14) {
				SleepUtility::sleepMicroSecond(7000000);//1  tuple/7s
			} else if (inputRate == 0.2) {
				SleepUtility::sleepMicroSecond(5000000);//0.2  tuple/s; 1 tuple/5s
			} else if (inputRate == 0.25) {
				SleepUtility::sleepMicroSecond(4000000);// 1 tuple/4s
			} else if (inputRate == 0.33) {
				SleepUtility::sleepMicroSecond(3000000);// 1 tuple/3s
			} else if (inputRate == 0.4) {
				SleepUtility::sleepMicroSecond(2600000);//0.4  tuple/s
			} else if (inputRate == 0.6) {
				SleepUtility::sleepMicroSecond(1676667);//0.6  tuple/s
			} else if (inputRate == 0.5) {
				SleepUtility::sleepMicroSecond(2000000);// 1 tuple/2s
			} else if (inputRate == 0.8) {
				SleepUtility::sleepMicroSecond(1260000);//0.8  tuple/s
			} else if (inputRate == 1) {
				SleepUtility::sleepMicroSecond(1000000);//1  tuple/s
			} else if (inputRate == 1.2) {
				SleepUtility::sleepMicroSecond(843333);//1.2  tuple/s
			} else if (inputRate == 10) {
				SleepUtility::sleepMicroSecond(100000);//10  tuple/s
			} else if (inputRate == 100) {
				SleepUtility::sleepMicroSecond(10000);//100  tuple/s
			} else if (inputRate == 200) {
				SleepUtility::sleepMicroSecond(5000);//200  tuple/s
			} else if (inputRate == 300) {
				SleepUtility::sleepMicroSecond(3333);//300  tuple/s
			} else if (inputRate == 400) {
				SleepUtility::sleepMicroSecond(2500);//400  tuple/s
			} else if (inputRate == 500) {
				SleepUtility::sleepMicroSecond(2000);//500  tuple/s
			} else if (inputRate == 600) {
				SleepUtility::sleepMicroSecond(1666);//600  tuple/s
			} else if (inputRate == 700) {
				SleepUtility::sleepMicroSecond(1428);//700  tuple/s
			} else if (inputRate == 800) {
				SleepUtility::sleepMicroSecond(1250);//800  tuple/s
			} else if (inputRate == 900) {
				SleepUtility::sleepMicroSecond(1111);//900  tuple/s
			} else if (inputRate == 1000) {
				SleepUtility::sleepMicroSecond(910);//1000  tuple/s
			} else if (inputRate == 10000) {
				SleepUtility::sleepMicroSecond(30);//10000  tuple/s
			} else {
				std::cout << "error input specification1" << std::endl;
				assert(false);
			}
			//SleepUtility::sleepMicroSecond(900000);//1  tuple/s
			//SleepUtility::sleepMicroSecond(20000);//50  tuple/s
			//SleepUtility::sleepMicroSecond(10000);//100 tuple/s
			//SleepUtility::sleepMicroSecond(6666);//150 tuple/s
			//SleepUtility::sleepMicroSecond(5000);//200 tuple/s
		} else if (this->getId() == "performanceTestStream2") {
			//boost::recursive_mutex::scoped_lock lock(m_oMutex);
			this->bufferedElementNumber++;
			//lock.unlock();
			this->totalElementNumber++;
			std::stringstream ss;
			// Q
			std::string inputRateStr =
					ConfigureManager::getInstance()->getConfigureValue(
							"stream2_input_rate");
			//int inputRate ;
			//ss << inputRateStr;
			//ss >> inputRate;

			double inputRate = 0.0;
			inputRate = atof(inputRateStr.c_str());
			//std::cout << this->getId() << " inputRate " << inputRate << std::endl;

			if (inputRate == 0.2) {
				SleepUtility::sleepMicroSecond(5100000);//0.2  tuple/s
			} else if (inputRate == 0.4) {
				SleepUtility::sleepMicroSecond(2600000);//0.4  tuple/s
			} else if (inputRate == 0.6) {
				SleepUtility::sleepMicroSecond(1676667);//0.6  tuple/s
			} else if (inputRate == 0.8) {
				SleepUtility::sleepMicroSecond(1260000);//0.8  tuple/s
			} else if (inputRate == 1) {
				SleepUtility::sleepMicroSecond(1000000);//1  tuple/s
			} else if (inputRate == 1.2) {
				SleepUtility::sleepMicroSecond(843333);//1.2  tuple/s
			} else if (inputRate == 10) {
				SleepUtility::sleepMicroSecond(100000);//10  tuple/s
			} else if (inputRate == 100) {
				SleepUtility::sleepMicroSecond(10000);//100  tuple/s
			} else if (inputRate == 1000) {
				SleepUtility::sleepMicroSecond(910);//1000  tuple/s
			} else if (inputRate == 10000) {
				SleepUtility::sleepMicroSecond(30);//10000  tuple/s
			} else if (inputRate == 20000) {
				static int k1 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k1) { // sleep only after generation of multiples of 100
					k1++;
					SleepUtility::sleepMicroSecond(4600); //
				}
			} else if (inputRate == 30000) {
				static int k2 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k2) { // sleep only after generation of multiples of 100
					k2++;
					SleepUtility::sleepMicroSecond(3000); //
				}
			} else if (inputRate == 40000) {
				static int k3 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k3) { // sleep only after generation of multiples of 100
					k3++;
					SleepUtility::sleepMicroSecond(2180); //
				}
			} else if (inputRate == 50000) {
				static int k4 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k4) { // sleep only after generation of multiples of 100
					k4++;
					SleepUtility::sleepMicroSecond(1675); //
				}
			} else if (inputRate == 60000) {
				static int k5 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k5) { // sleep only after generation of multiples of 100
					k5++;
					SleepUtility::sleepMicroSecond(1340); //
				}
			} else if (inputRate == 70000) {
				static int k6 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k6) { // sleep only after generation of multiples of 100
					k6++;
					SleepUtility::sleepMicroSecond(1115); //
				}
			} else if (inputRate == 80000) {
				static int k7 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k7) { // sleep only after generation of multiples of 100
					k7++;
					SleepUtility::sleepMicroSecond(935); //
				}
			} else if (inputRate == 90000) {
				static int k8 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k8) { // sleep only after generation of multiples of 100
					k8++;
					SleepUtility::sleepMicroSecond(805); //
				}
			} else if (inputRate == 100000) {
				static int k11 = 0; // will be initialized only once
				if (totalElementNumber > 100 * k11) { // sleep only after generation of multiples of 100
					k11++;
					SleepUtility::sleepMicroSecond(700); // 100,000 tuples/s
				}
			} else if (inputRate == 200000) {
				static int k12 = 0;
				if (totalElementNumber > 100 * k12) {
					k12++;
					SleepUtility::sleepMicroSecond(240); // 200,000 tuples/s
				}
			} else if (inputRate == 300000) {
				static int k13 = 0;
				if (totalElementNumber > 100 * k13) {
					k13++;
					SleepUtility::sleepMicroSecond(200); // 300,000 tuples/s
				}
			} else if (inputRate == 400000) {
				static int k14 = 0;
				if (totalElementNumber > 100 * k14) {
					k14++;
					SleepUtility::sleepMicroSecond(155); // 400,000 tuples/s
				}
			} else if (inputRate == 500000) {
				static int k15 = 0;
				if (totalElementNumber > 100 * k15) {
					k15++;
					SleepUtility::sleepMicroSecond(110); // 500,000 tuples/s
				}
			} else if (inputRate == 600000) {
				static int k16 = 0;
				if (totalElementNumber > 100 * k16) {
					k16++;
					SleepUtility::sleepMicroSecond(75); // 600,000 tuples/s
				}
			} else if (inputRate == 700000) {
				static int k17 = 0;
				if (totalElementNumber > 100 * k17) {
					k17++;
					SleepUtility::sleepMicroSecond(50); // 700,000 tuples/s
				}
			} else if (inputRate == 800000) {
				static int k18 = 0;
				if (totalElementNumber > 100 * k18) {
					k18++;
					SleepUtility::sleepMicroSecond(35); // 800,000 tuples/s
				}
			} else if (inputRate == 900000) {
				static int k19 = 0;
				if (totalElementNumber > 100 * k19) {
					k19++;
					SleepUtility::sleepMicroSecond(32); // 900,000 tuples/s
				}
			} else if (inputRate == 1000000) {
				static int k20 = 0;
				if (totalElementNumber > 100 * k20) {
					k20++;
					SleepUtility::sleepMicroSecond(30); // 1,000,000 tuples/s
				}
			}
		} else if (this->getId() == "performanceTestStream3") {
			this->bufferedElementNumber++;
			this->totalElementNumber++;
			std::stringstream ss;
			std::string inputRateStr =
					ConfigureManager::getInstance()->getConfigureValue(
							"stream3_input_rate");
			//int inputRate ;
			//ss << inputRateStr;
			//ss >> inputRate;
			double inputRate = 0.0;
			inputRate = atof(inputRateStr.c_str());
			//std::cout << this->getId() << " inputRate " << inputRate << std::endl;

			if (inputRate == 0.11) {
				SleepUtility::sleepMicroSecond(9000000);//1  tuple/9s
			} else if (inputRate == 0.14) {
				SleepUtility::sleepMicroSecond(7000000);//1  tuple/7s
			} else if (inputRate == 0.2) {
				SleepUtility::sleepMicroSecond(5000000);//0.2  tuple/s
			} else if (inputRate == 0.25) {
				SleepUtility::sleepMicroSecond(4000000);// 1 tuple/4s
			} else if (inputRate == 0.33) {
				SleepUtility::sleepMicroSecond(3000000);// 1 tuple/3s
			} else if (inputRate == 0.4) {
				SleepUtility::sleepMicroSecond(2600000);//0.4  tuple/s
			} else if (inputRate == 0.6) {
				SleepUtility::sleepMicroSecond(1676667);//0.6  tuple/s
			} else if (inputRate == 0.5) {
				SleepUtility::sleepMicroSecond(2000000);// 1 tuple/2s
			} else if (inputRate == 0.8) {
				SleepUtility::sleepMicroSecond(1260000);//0.8  tuple/s
			} else if (inputRate == 1) {
				SleepUtility::sleepMicroSecond(1000000);//1  tuple/s
			} else if (inputRate == 1.2) {
				SleepUtility::sleepMicroSecond(843333);//1.2  tuple/s
			} else if (inputRate == 10) {
				SleepUtility::sleepMicroSecond(100000);//10  tuple/s
			} else if (inputRate == 100) {
				SleepUtility::sleepMicroSecond(10000);//100  tuple/s
			} else if (inputRate == 1000) {
				SleepUtility::sleepMicroSecond(910);//1000  tuple/s
			} else if (inputRate == 10000) {
				SleepUtility::sleepMicroSecond(30);//10000  tuple/s
			} else if (inputRate == 100000) {
				static int k11 = 0;
				if (totalElementNumber > 100 * k11) {
					k11++;
					SleepUtility::sleepMicroSecond(700); // 100,000 tuples/s
				}
			} else if (inputRate == 200000) {
				static int k12 = 0;
				if (totalElementNumber > 100 * k12) {
					k12++;
					SleepUtility::sleepMicroSecond(240); // 200,000 tuples/s
				}
			} else if (inputRate == 300000) {
				static int k13 = 0;
				if (totalElementNumber > 100 * k13) {
					k13++;
					SleepUtility::sleepMicroSecond(200); // 300,000 tuples/s
				}
			} else if (inputRate == 400000) {
				static int k14 = 0;
				if (totalElementNumber > 100 * k14) {
					k14++;
					SleepUtility::sleepMicroSecond(155); // 400,000 tuples/s
				}
			} else if (inputRate == 500000) {
				static int k15 = 0;
				if (totalElementNumber > 100 * k15) {
					k15++;
					SleepUtility::sleepMicroSecond(110); // 500,000 tuples/s
				}
			} else if (inputRate == 600000) {
				static int k16 = 0;
				if (totalElementNumber > 100 * k16) {
					k16++;
					SleepUtility::sleepMicroSecond(75); // 600,000 tuples/s
				}
			} else if (inputRate == 700000) {
				static int k17 = 0;
				if (totalElementNumber > 100 * k17) {
					k17++;
					SleepUtility::sleepMicroSecond(50); // 700,000 tuples/s
				}
			} else if (inputRate == 800000) {
				static int k18 = 0;
				if (totalElementNumber > 100 * k18) {
					k18++;
					SleepUtility::sleepMicroSecond(35); // 800,000 tuples/s
				}
			} else if (inputRate == 900000) {
				static int k19 = 0;
				if (totalElementNumber > 100 * k19) {
					k19++;
					SleepUtility::sleepMicroSecond(32); // 900,000 tuples/s
				}
			} else if (inputRate == 1000000) {
				static int k10 = 0;
				if (totalElementNumber > 100 * k10) {
					k10++;
					SleepUtility::sleepMicroSecond(30); // 1,000,000 tuples/s
				}
			}else if (inputRate == 100000) {
				static int k11 = 0;
				if (totalElementNumber > 100 * k11) {
					k11++;
					SleepUtility::sleepMicroSecond(700); // 100,000 tuples/s
				}
			} else if (inputRate == 200000) {
				static int k12 = 0;
				if (totalElementNumber > 100 * k12) {
					k12++;
					SleepUtility::sleepMicroSecond(240); // 200,000 tuples/s
				}
			} else if (inputRate == 300000) {
				static int k13 = 0;
				if (totalElementNumber > 100 * k13) {
					k13++;
					SleepUtility::sleepMicroSecond(200); // 300,000 tuples/s
				}
			} else if (inputRate == 400000) {
				static int k14 = 0;
				if (totalElementNumber > 100 * k14) {
					k14++;
					SleepUtility::sleepMicroSecond(155); // 400,000 tuples/s
				}
			} else if (inputRate == 500000) {
				static int k15 = 0;
				if (totalElementNumber > 100 * k15) {
					k15++;
					SleepUtility::sleepMicroSecond(110); // 500,000 tuples/s
				}
			} else if (inputRate == 600000) {
				static int k16 = 0;
				if (totalElementNumber > 100 * k16) {
					k16++;
					SleepUtility::sleepMicroSecond(75); // 600,000 tuples/s
				}
			} else if (inputRate == 700000) {
				static int k17 = 0;
				if (totalElementNumber > 100 * k17) {
					k17++;
					SleepUtility::sleepMicroSecond(50); // 700,000 tuples/s
				}
			} else if (inputRate == 800000) {
				static int k18 = 0;
				if (totalElementNumber > 100 * k18) {
					k18++;
					SleepUtility::sleepMicroSecond(35); // 800,000 tuples/s
				}
			} else if (inputRate == 900000) {
				static int k19 = 0;
				if (totalElementNumber > 100 * k19) {
					k19++;
					SleepUtility::sleepMicroSecond(32); // 900,000 tuples/s
				}
			} else if (inputRate == 1000000) {
				static int k10 = 0;
				if (totalElementNumber > 100 * k10) {
					k10++;
					SleepUtility::sleepMicroSecond(30); // 1,000,000 tuples/s
				}
			}
			else {
				std::cout << "error input specification3" << std::endl;
				//sleep(1);
				assert(false);
			}
		} else if (this->getId() == "performanceTestStream4") {
			this->bufferedElementNumber++;
			this->totalElementNumber++;
			std::stringstream ss;
			std::string inputRateStr =
					ConfigureManager::getInstance()->getConfigureValue(
							"stream4_input_rate");
			//int inputRate ;
			//ss << inputRateStr;
			//ss >> inputRate;
			double inputRate = 0.0;
			inputRate = atof(inputRateStr.c_str());
			//std::cout << this->getId() << " inputRate " << inputRate << std::endl;

			if (inputRate == 0.2) {
				SleepUtility::sleepMicroSecond(5100000);//0.2  tuple/s
			} else if (inputRate == 0.4) {
				SleepUtility::sleepMicroSecond(2600000);//0.4  tuple/s
			} else if (inputRate == 0.6) {
				SleepUtility::sleepMicroSecond(1676667);//0.6  tuple/s
			} else if (inputRate == 0.8) {
				SleepUtility::sleepMicroSecond(1260000);//0.8  tuple/s
			} else if (inputRate == 1) {
				SleepUtility::sleepMicroSecond(1000000);//1  tuple/s
			} else if (inputRate == 1.2) {
				SleepUtility::sleepMicroSecond(843333);//1.2  tuple/s
			} else if (inputRate == 10) {
				SleepUtility::sleepMicroSecond(100000);//10  tuple/s
			} else if (inputRate == 100) {
				SleepUtility::sleepMicroSecond(10000);//100  tuple/s
			} else if (inputRate == 1000) {
				SleepUtility::sleepMicroSecond(910);//1000  tuple/s
			} else if (inputRate == 10000) {
				SleepUtility::sleepMicroSecond(30);//10000  tuple/s
			} else if (inputRate = 100000) {

				static int k2 = 0;
				if (totalElementNumber > 100 * k2) {
					k2++;
					SleepUtility::sleepMicroSecond(600); // 100,000 tuples/s
				}
			}
		} else if (this->getId() == "eventStream") {
			//boost::recursive_mutex::scoped_lock lock(m_oMutex);
			this->bufferedElementNumber++;
			//lock.unlock();
			this->totalElementNumber++;
			// P
			//SleepUtility::sleepMicroSecond(900000);//1  tuple/s
			//SleepUtility::sleepMicroSecond(20000);//50  tuple/s
			SleepUtility::sleepMicroSecond(10000);//100 tuple/s
			//SleepUtility::sleepMicroSecond(6666);//150 tuple/s
			//SleepUtility::sleepMicroSecond(5000);//200 tuple/s
		} else if (this->getId() == "performanceTestStreamGroup") {

			boost::recursive_mutex::scoped_lock lock(m_oMutex);
			this->bufferedElementNumber++;

			lock.unlock();
			this->totalElementNumber++;
			SleepUtility::sleepMicroSecond(1000); //1000tuple/s

		} else if (this->getId() == "salesFact" || this->getId() == "salesFact5Dim" || this->getId() == "salesFact5DimWNamesWrapper" || this->getId() == "salesFact6DimWNamesWrapper" || this->getId() == "salesFact4DimTPCH" || this->getId() == "salesFactPreJoined6DWrapper") {

			boost::recursive_mutex::scoped_lock lock(m_oMutex);
			this->bufferedElementNumber++;

			lock.unlock();
			this->totalElementNumber++;
			SleepUtility::sleepMicroSecond(1000); //1000tuple/s

		}
	}
}
/*
 //original function by wang
 //void SpecifiedInputRateStreamInput::generateInput(int sleepMicroSeconds)
 //{
 //	while(1)
 //	{
 //
 //		if(this->getId() == "performanceTestStream1")
 //		{
 //	 		//boost::recursive_mutex::scoped_lock lock(m_oMutex);
 //			this->bufferedElementNumber++;
 //			//lock.unlock();
 //			this->totalElementNumber++;
 //		//	SleepUtility::sleepMicroSecond(10000);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(5000);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(3333);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(2500);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(2000);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(1666);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(1428);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(1250);//tuple/s
 //		//	SleepUtility::sleepMicroSecond(1111);//tuple/s
 //			SleepUtility::sleepMicroSecond(1000);//tuple/s
 //		}
 //		else if(this->getId() == "performanceTestStream2")
 //		{
 //	 		//boost::recursive_mutex::scoped_lock lock(m_oMutex);
 //			this->bufferedElementNumber++;
 //			//lock.unlock();
 //			this->totalElementNumber++;
 //			static int k = 0;
 //			if(totalElementNumber>100*k)
 //			{
 //				k++;
 //		//		SleepUtility::sleepMicroSecond(900);   100,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(400);   200,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(240);   300,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(155);   400,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(110);   500,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(75);   600,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(50);   700,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(35);   800,000 tuples/s
 //		//		SleepUtility::sleepMicroSecond(32);   900,000 tuples/s
 //				SleepUtility::sleepMicroSecond(30);   1,000,000 tuples/s
 //			}
 //		}
 //	}
 //}
 */

/*
 //Used by salman for experiments
 #include "../Common/stdafx.h"
 #include "../IO/SpecifiedInputRateStreamInput.h"
 #include "../Schema/SchemaInterpreter.h"
 #include "../Utility/DocumentIdentifierGenerator.h"
 #include "../Utility/TimestampGenerator.h"
 #include "../Utility/SleepUtility.h"
 #include <boost/thread.hpp>
 #include <boost/bind.hpp>
 #include <boost/function.hpp>
 int SpecifiedInputRateStreamInput::ignoredInputNumber = 0;
 SpecifiedInputRateStreamInput::SpecifiedInputRateStreamInput(boost::shared_ptr<JsonSchema> schema):IStreamInput(schema)
 {
 this->schema = schema;
 SchemaInterpreter::generateRandomDocumentBySchema(this->schema,this->bufferedDocument);
 this->bufferedElementNumber = 0;
 this->totalElementNumber = 0;
 }

 SpecifiedInputRateStreamInput::~SpecifiedInputRateStreamInput(void)
 {
 std::cout<<this->getId()<<" total element number : "<< totalElementNumber<<std::endl;
 }

 void SpecifiedInputRateStreamInput::getNextElement(Element& element)
 {
 boost::recursive_mutex::scoped_lock lock(m_oMutex);
 this->bufferedElementNumber--;
 assert(this->bufferedElementNumber>=0);
 lock.unlock();
 element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
 element.mark = PLUS_MARK;
 element.timestamp = TimestampGenerator::getCurrentTime();
 element.document = this->bufferedDocument;
 element.document.getOwned();
 }

 bool SpecifiedInputRateStreamInput::isEmpty()
 {
 boost::recursive_mutex::scoped_lock lock(m_oMutex);
 bool bl = false;
 if(bufferedElementNumber==0)
 {
 bl = true;
 }
 lock.unlock();
 return bl;
 }

 void SpecifiedInputRateStreamInput::initial()
 {
 int sleepMiliSeconds = 20;
 //std::cout<<"creating one thread+++++++++++++++"<<std::endl;
 boost::function<void (int)> memberFunctionWrapper(boost::bind(&SpecifiedInputRateStreamInput::generateInput, this, _1));
 boost::thread downloadThread ((boost::bind(memberFunctionWrapper, sleepMiliSeconds)));
 }

 void SpecifiedInputRateStreamInput::generateInput(int sleepMiliSeconds)
 {
 while(1)
 {
 if(this->getId() == "performanceTestStream1")
 {
 boost::recursive_mutex::scoped_lock lock(m_oMutex);
 this->bufferedElementNumber++;
 lock.unlock();
 this->totalElementNumber++;
 //SleepUtility::sleepMicroSecond(1000);//tuple/s
 // If sleep 1 sec (usleep(1000000)), 1 tuple is generated per second
 // If sleep 0.1 sec (usleep(100000)), 10 tuples are generated per second
 // If sleep 0.01 sec (usleep(10000)), 100 tuples are generated per second
 //usleep(11000000); //0.1
 //usleep(5100000);  //0.2
 //usleep(2500000); //0.5
 //usleep(1550000); //0.7
 //usleep(1250000); //0.9
 //usleep(1100000); // 1
 //usleep(1000000); //1.1
 //usleep(850000); //1.3
 //usleep(100000); //10
 usleep(10000); //100


 }
 else if(this->getId() == "performanceTestStream2")
 {
 boost::recursive_mutex::scoped_lock lock(m_oMutex);
 this->bufferedElementNumber++;
 lock.unlock();
 totalElementNumber++;

 static int k = 0;
 // sleep after generating every 100 tuples
 if(totalElementNumber>100*k)
 {
 k++;
 //SleepUtility::sleepMicroSecond(100);
 usleep(100);
 }


 }
 }
 }
 */
