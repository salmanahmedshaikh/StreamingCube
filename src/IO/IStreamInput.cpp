//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/IStreamInput.h"
#include "../Internal/Element/Element.h"
#include "../IO/IOManager.h"
#include "../Schema/SchemaInterpreter.h"

IStreamInput::IStreamInput(boost::shared_ptr<JsonSchema>jsonSchema)
{
	this->jsonSchema = jsonSchema;
	//std::cout << "this->jsonSchema " << this->jsonSchema << std::endl;
	this->id = jsonSchema->getId();
	//std::cout << "this->schemaId " << this->id << std::endl;
	this->isActive = false;
}

IStreamInput::IStreamInput(void)
{
    this->isActive = false;
}

IStreamInput::~IStreamInput(void)
{
}
std::string IStreamInput::getId(void)
{
	return this->id;
}

bool IStreamInput::getAndCheckNextElement(Element& element)
{
	bool bl = true;
	//getNextElement is defined for input stream in its wrapper e.g., SocketStreamInput, PeopleFlowStreamInput
	// IStreamInput is the base class of all the wrapper classes
	getNextElement(element);
#ifdef CHECKINPUTSCHEMA
	bl = SchemaInterpreter::checkDocumentSatisfiedSchema(element.document,this->jsonSchema);
#endif

	return bl;
}

boost::shared_ptr<JsonSchema> IStreamInput::getSchema(void)
{
	return this->jsonSchema;
}

void IStreamInput::start()
{
	if(this->isActive == false)
	{
		initial();
		this->isActive = true;
	}
}
