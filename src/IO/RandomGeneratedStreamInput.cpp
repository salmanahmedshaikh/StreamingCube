//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../IO/RandomGeneratedStreamInput.h"
#include "../Schema/SchemaInterpreter.h"
#include "../Utility/DocumentIdentifierGenerator.h"
#include "../Utility/TimestampGenerator.h"
#include "../Utility/SleepUtility.h"
int RandomGeneratedStreamInput::throughout = 0;
RandomGeneratedStreamInput::RandomGeneratedStreamInput(boost::shared_ptr<JsonSchema> schema):IStreamInput(schema)
{
	this->schema = schema;
}


RandomGeneratedStreamInput::~RandomGeneratedStreamInput(void)
{
}
void RandomGeneratedStreamInput::getNextElement(Element& element)
{
	throughout++;

	//SchemaInterpreter::generateRandomDocumentBySchema(this->schema,outputDocument);

	element.document = this->bufferedDocument;
	element.document.getOwned();
	element.id = DocumentIdentifierGenerator::generateNewDocumentIdentifier();
	element.mark = PLUS_MARK;
	element.timestamp = TimestampGenerator::getCurrentTime();
	element.masterTag = false;


}
bool RandomGeneratedStreamInput::isEmpty()
{
	//wangyan return true;
	return false;
}
void RandomGeneratedStreamInput::initial()
{

	SchemaInterpreter::generateRandomDocumentBySchema(this->schema,this->bufferedDocument);
}
