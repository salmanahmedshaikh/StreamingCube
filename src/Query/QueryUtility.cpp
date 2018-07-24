//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#include "../Common/stdafx.h"
#include "../Query/QueryUtility.h"
#include "../Query/QueryIntermediateRepresentationInterpreter.h"
#include "../Query/QueryAttribute.h"
#include <assert.h>
#include <boost/regex.hpp>
QueryUtility::QueryUtility(void)
{
}


QueryUtility::~QueryUtility(void)
{
}

boost::shared_ptr<QueryConditionAbstract> QueryUtility::resolveQueryCondition(Document& document)
{
	std::string op = document.getField(QUERY_CONDITION_TYPE).String();
	if(op.compare("bool") == 0)
	{
		//BoolQueryCondition
		boost::shared_ptr<BoolQueryCondition> retCondition (new BoolQueryCondition());
		Document doc = document.getField(QUERY_EXPRESSION).embeddedObject();
		boost::shared_ptr<BoolQueryExpression> expression(new BoolQueryExpression());
		bool bl = doc.getField(QUERY_BOOL_VALUE).Bool();
		expression->setValue(bl);
		retCondition->setExpression(expression);
		return retCondition;
	}
	else if(op.compare("neg") == 0)
	{
		boost::shared_ptr<NegativeQueryCondition> retCondition (new NegativeQueryCondition());
		Document doc = document.getField(QUERY_CONDITION_TYPE).embeddedObject();
		boost::shared_ptr<QueryConditionAbstract> condition (resolveQueryCondition(doc));
		retCondition->setCondition(condition);
		return retCondition;
		//NegativeQueryCondition
	}
	else if(op.compare("gt") == 0)
	{
		CompareLogicConditionOperation operation = CO_GT;
		boost::shared_ptr<CompareQueryCondition>retCondition(new CompareQueryCondition());
		retCondition->setCompareLogicConditionOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract>leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract>rightExpression(resolveQueryExpression(rightExpressionDocument));
		retCondition->setLeftExpression(leftExpression);
		retCondition->setRightExpression(rightExpression);
		return retCondition;
	}
	else if(op.compare("ge") == 0)
	{
		CompareLogicConditionOperation operation = CO_GE;
		boost::shared_ptr<CompareQueryCondition>retCondition(new CompareQueryCondition());
		retCondition->setCompareLogicConditionOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract>leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract>rightExpression(resolveQueryExpression(rightExpressionDocument));
		retCondition->setLeftExpression(leftExpression);
		retCondition->setRightExpression(rightExpression);
		return retCondition;
	}
	else if(op.compare("lt") == 0)
	{
		CompareLogicConditionOperation operation = CO_LT;
		boost::shared_ptr<CompareQueryCondition>retCondition(new CompareQueryCondition());
		retCondition->setCompareLogicConditionOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract>leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract>rightExpression(resolveQueryExpression(rightExpressionDocument));
		retCondition->setLeftExpression(leftExpression);
		retCondition->setRightExpression(rightExpression);
		return retCondition;
	}
	else if(op.compare("le") == 0)
	{
		CompareLogicConditionOperation operation = CO_LE;
		boost::shared_ptr<CompareQueryCondition>retCondition(new CompareQueryCondition());
		retCondition->setCompareLogicConditionOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract>leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract>rightExpression(resolveQueryExpression(rightExpressionDocument));
		retCondition->setLeftExpression(leftExpression);
		retCondition->setRightExpression(rightExpression);
		return retCondition;
	}
	else if(op.compare("eq") == 0)
	{
		CompareLogicConditionOperation operation = CO_EQ;
		boost::shared_ptr<CompareQueryCondition>retCondition(new CompareQueryCondition());
		retCondition->setCompareLogicConditionOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract>leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract>rightExpression(resolveQueryExpression(rightExpressionDocument));
		retCondition->setLeftExpression(leftExpression);
		retCondition->setRightExpression(rightExpression);
		return retCondition;
	}
	else if(op.compare("ne") == 0)
	{
		CompareLogicConditionOperation operation = CO_NE;
		boost::shared_ptr<CompareQueryCondition>retCondition(new CompareQueryCondition());
		retCondition->setCompareLogicConditionOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract>leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract>rightExpression(resolveQueryExpression(rightExpressionDocument));
		retCondition->setLeftExpression(leftExpression);
		retCondition->setRightExpression(rightExpression);
		return retCondition;
	}
	else if(op.compare("and") == 0)
	{
		BinaryLogicConditionOperation operation = CO_AND;
		boost::shared_ptr<BinaryLogicQueryCondition> retCondition(new BinaryLogicQueryCondition());

		retCondition->setBinaryLogicConditionOperation(operation);

		Document leftConditionDocument = document.getField(QUERY_LEFT_CONDITION).embeddedObject();
		Document rightConditionDocument = document.getField(QUERY_RIGHT_CONDITION).embeddedObject();
		boost::shared_ptr<QueryConditionAbstract>leftCondition (resolveQueryCondition(leftConditionDocument));
		boost::shared_ptr<QueryConditionAbstract>rightCondition (resolveQueryCondition(rightConditionDocument));
		retCondition->setLeftCondition(leftCondition);
		retCondition->setRightCondition(rightCondition);
		//std::cout << "retCondtion: " << retCondtion << std::endl;
		return retCondition;
	}
	else if(op.compare("or") == 0)
	{
		BinaryLogicConditionOperation operation = CO_OR;
		boost::shared_ptr<BinaryLogicQueryCondition> retCondition(new BinaryLogicQueryCondition());

		retCondition->setBinaryLogicConditionOperation(operation);

		Document leftConditionDocument = document.getField(QUERY_LEFT_CONDITION).embeddedObject();
		Document rightConditionDocument = document.getField(QUERY_RIGHT_CONDITION).embeddedObject();
		boost::shared_ptr<QueryConditionAbstract>leftCondition (resolveQueryCondition(leftConditionDocument));
		boost::shared_ptr<QueryConditionAbstract>rightCondition (resolveQueryCondition(rightConditionDocument));
		retCondition->setLeftCondition(leftCondition);
		retCondition->setRightCondition(rightCondition);
		return retCondition;
	}
	else assert(false);

}



boost::shared_ptr<QueryExpressionAbstract> QueryUtility::resolveQueryExpression(Document& document)
{

	std::string op = document.getField(QUERY_EXPRESSION_TYPE).String();

	if(op.compare("add") == 0)
	{
		CalculationExpressionOperation operation = EO_ADD;
		boost::shared_ptr<CalculationQueryExpression>retExpression(new CalculationQueryExpression());
		retExpression->setOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract> leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract> rightExpression(resolveQueryExpression(rightExpressionDocument));
		retExpression->setLeftExpression(leftExpression);
		retExpression->setRightExpression(rightExpression);
		return retExpression;
	}
	else if(op.compare("sub") == 0)
	{
		CalculationExpressionOperation operation = EO_SUB;
		boost::shared_ptr<CalculationQueryExpression>retExpression(new CalculationQueryExpression());
		retExpression->setOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract> leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract> rightExpression(resolveQueryExpression(rightExpressionDocument));
		retExpression->setLeftExpression(leftExpression);
		retExpression->setRightExpression(rightExpression);
		return retExpression;
	}
	else if(op.compare("mul") == 0)
	{
		CalculationExpressionOperation operation = EO_MUL;
		boost::shared_ptr<CalculationQueryExpression>retExpression(new CalculationQueryExpression());
		retExpression->setOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract> leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract> rightExpression(resolveQueryExpression(rightExpressionDocument));
		retExpression->setLeftExpression(leftExpression);
		retExpression->setRightExpression(rightExpression);
		return retExpression;
	}
	else if(op.compare("div") == 0)
	{
		CalculationExpressionOperation operation = EO_DIV;
		boost::shared_ptr<CalculationQueryExpression>retExpression(new CalculationQueryExpression());
		retExpression->setOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract> leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract> rightExpression(resolveQueryExpression(rightExpressionDocument));
		retExpression->setLeftExpression(leftExpression);
		retExpression->setRightExpression(rightExpression);
		return retExpression;
	}
	else if(op.compare("mod") == 0)
	{
		CalculationExpressionOperation operation = EO_MOD;
		boost::shared_ptr<CalculationQueryExpression>retExpression(new CalculationQueryExpression());
		retExpression->setOperation(operation);
		Document leftExpressionDocument = document.getField(QUERY_LEFT_EXPRESSION).embeddedObject();
		Document rightExpressionDocument = document.getField(QUERY_RIGHT_EXPRESSION).embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract> leftExpression(resolveQueryExpression(leftExpressionDocument));
		boost::shared_ptr<QueryExpressionAbstract> rightExpression(resolveQueryExpression(rightExpressionDocument));
		retExpression->setLeftExpression(leftExpression);
		retExpression->setRightExpression(rightExpression);
		return retExpression;
	}
	else if(op.compare("aggregation") == 0)
	{
		// todo wangyan
		//AGGREGATE_OPERATION


				AggregateOperation operation;
				std::string aggregateOperation = document.getField(
						QUERY_AGGREGATE_OPERATION).valuestr();
				boost::shared_ptr<AggregationQueryExpression> retExpression(
						new AggregationQueryExpression());


				if (aggregateOperation == "average")
					operation = AVG;
				else if (aggregateOperation == "sum")
					operation = SUM;
				else if (aggregateOperation == "count")
					operation = COUNT;
				else if (aggregateOperation == "min")
					operation = MIN;
				else if (aggregateOperation == "max")
					operation = MAX;

				retExpression->setOperation(operation);

				//AGGREGATE_PROJECTION
				//after fixing the document?
				Document idArray = document.getField(QUERY_ID_NAME).embeddedObject();

				boost::shared_ptr<QueryAttribute> queryAttribute =
						resolveQueryAttribute(idArray);

			boost::any aggregationAttributeName =
						queryAttribute->getLastAttributeAny();

				retExpression->setAggregationAttributeName(boost::any_cast<string>(aggregationAttributeName));

				//ATTRIBUTE_SOURCE
				AttrSource attrSource;

				std::string attributeSource =
						document.getField(QUERY_ATTRIBUTE_SOURCE).valuestr();

				if (attributeSource == "group_array")
					attrSource = _GROUP_ARRAY;

				else if (aggregateOperation == "group_key_var")
					attrSource = _GROUP_KEY_VAR;

				retExpression->setAttributeSource(attrSource);

				return retExpression;

	}
	else if(op.compare("id") == 0)
	{
		std::vector<boost::any> idNameVector;
		AttrSource attrSource;
		boost::shared_ptr<IdentifierQueryExpression> retExpression(new IdentifierQueryExpression());
		Document idArray = document.getField(QUERY_ID_NAME).embeddedObject();
		boost::shared_ptr<QueryAttribute> queryAttribute = resolveQueryAttribute(idArray);
		retExpression->setAttibute(queryAttribute);
		if (!document.hasField(QUERY_ATTRIBUTE_SOURCE.c_str()))
		{
			attrSource = _Any;
		}
		else
		{
			std::string source = document.getField(QUERY_ATTRIBUTE_SOURCE).valuestr();
			if ( source == QUERY_LEFT)
			{
				attrSource =  _LEFT;
			}
			else if ( source == QUERY_RIGHT)
			{
				attrSource = _RIGHT;
			}
			else if ( source == QUERY_GROUP_ARRAY)
			{
				attrSource = _GROUP_ARRAY;
			}
			else if ( source == QUERY_KEY_VAR)
			{
				attrSource  = _GROUP_KEY_VAR;
			}
		}
		retExpression->setAttrSource(attrSource);
		return retExpression;
	}
	else if(op.compare("int") == 0)
	{
		boost::shared_ptr<IntegerQueryExpression> retExpression(new IntegerQueryExpression());
		int value = document.getField(QUERY_INT_VALUE).Int();
		retExpression->setValue(value);
		return retExpression;
	}
	else if(op.compare("double") == 0)
	{
		boost::shared_ptr<DoubleQueryExpression> retExpression(new DoubleQueryExpression());
		double value = document.getField(QUERY_DOUBLE_VALUE).Int();
		retExpression->setValue(value);
		return retExpression;
	}
	else if(op.compare("bool") == 0)
	{

		boost::shared_ptr<BoolQueryExpression> retExpression(new BoolQueryExpression());
		bool value = document.getField(QUERY_BOOL_VALUE).Bool();
		retExpression->setValue(value);
		return retExpression;
	}
	else if(op.compare("null") == 0)
	{
		boost::shared_ptr<NullQueryExpression> retExpression(new NullQueryExpression());
		return retExpression;
	}
	else if(op.compare("string") == 0)
	{
		boost::shared_ptr<StringQueryExpression> retExpression(new StringQueryExpression());
		std::string value = document.getField(QUERY_STRING_VALUE).valuestr();
		retExpression->setValue(value);
		return retExpression;
	}
	else assert(false);


}
boost::shared_ptr<QueryAttribute> QueryUtility::resolveQueryAttribute(Document& document)
{
	boost::shared_ptr<QueryAttribute> queryAttribute(new QueryAttribute());
	std::vector<boost::any >idNameVector;
	DocumentIterator it(document);
	while(it.more())
	{
		DocumentElement documentElement =  it.next();
		//std::cout<<documentElement<<std::endl;
		if (documentElement.isNumber())
		{
			idNameVector.push_back(documentElement.numberInt());
		}
		else if(documentElement.isStringType())
		{
			idNameVector.push_back(std::string(documentElement.valuestr()));
		}
		else if(documentElement.isArrayType())
		{
			Document  rangeArrayDocument = documentElement.embeddedObject();
			DocumentIterator itt(rangeArrayDocument);
			std::vector<int>rangeVector;

			while(itt.more())
			{
				DocumentElement arrangeElement =  itt.next();
				assert(arrangeElement.isNumber());
				rangeVector.push_back(arrangeElement.numberInt());
			}
			idNameVector.push_back(rangeVector);
		}
		else
		{
			assert(false);
		}
	}
	queryAttribute->setIdNameVector(idNameVector);
	return queryAttribute;
}
boost::shared_ptr<QueryProjectionAbstract> QueryUtility::resolveQueryProjection(Document& document)
{
	std::string type = document.getField(QUERY_PROJECTION_TYPE).valuestr();
	if(type == "object")
	{
		boost::shared_ptr<ObjectQueryProjection> retProjectionObject (new ObjectQueryProjection());
		std::vector<boost::shared_ptr<QueryProjectionAbstract> >  projectionVector;
		Document fieldDocument = document.getField(QUERY_FIELDS).embeddedObject();
		DocumentIterator it(fieldDocument);
		while(it.more())
		{
			DocumentElement documentElement =  it.next();
			Document projectionObject = documentElement.embeddedObject();
			//std::cout<<projectionObject<<std::endl;

			std::string type = projectionObject.getField("type").valuestr();
			assert(type == "projection_obj");
			boost::shared_ptr<QueryProjectionAbstract> projectionAbstract = resolveQueryProjection(projectionObject);
			projectionVector.push_back(projectionAbstract);
		}
		retProjectionObject->setProjectionFields(projectionVector);

		if(document.hasField("need_rename"))
		{
			bool bl = document.getField("need_rename").Bool();
			if(bl == true)
			{
				std::string rename = document.getField("rename").valuestr();
				retProjectionObject->setNeedRename(true);
				retProjectionObject->setRename(rename);
			}
			else
			{
				retProjectionObject->setNeedRename(false);
			}
		}
		else
		{
			retProjectionObject->setNeedRename(false);
		}
		return retProjectionObject;
	}
	else if(type == "array")
	{
		/*boost::shared_ptr<ArrayQueryProjection> retProjectionObject (new ArrayQueryProjection());
		std::vector<boost::shared_ptr<QueryProjectionAbstract> >  projectionVector;
		Document& itemDocument = document.getField(QUERY_ARRAY_ITEMS).embeddedObject();
		DocumentIterator it(itemDocument);
		while(it.more())
		{
			DocumentElement& documentElement =  it.next();
			Document& projectionObject = documentElement.embeddedObject();
			assert(projectionObject.getField("type").valuestr() == "projection_obj");
			std::string projectionTypeStr = projectionObject.getField("projection_type");
			boost::shared_ptr<QueryProjectionAbstract> projectionAbstract = resolveQueryProjection(projectionObject);
			projectionVector.push_back(projectionAbstract);
		}
		retProjectionObject->setProjectionFields(projectionVector);
		return retProjectionObject;*/
	}
	else if(type == "direct")
	{
		boost::shared_ptr<DirectQueryProjection> retProjectionObject (new DirectQueryProjection());
		if(document.hasField("need_rename"))
		{
			bool bl = document.getField("need_rename").Bool();
			if(bl == true)
			{
				std::string rename = document.getField("rename").valuestr();
				retProjectionObject->setNeedRename(true);
				retProjectionObject->setRename(rename);
			}
			else
			{
				retProjectionObject->setNeedRename(false);
			}
		}
		else
		{
			retProjectionObject->setNeedRename(false);
		}
		Document expressionDocument = document.getField("expression").embeddedObject();
		boost::shared_ptr<QueryExpressionAbstract> expression = resolveQueryExpression(expressionDocument);
		retProjectionObject->setExpression(expression);
		return retProjectionObject;


	}
}
bool QueryUtility::compareEqual(boost::any leftValue, boost::any rightValue)
{
	if(leftValue.type()==typeid(double)||rightValue.type()==typeid(double))
	{
		return boost::any_cast<double>(leftValue) == boost::any_cast<double>(rightValue);
	}
	else if(leftValue.type()==typeid(float)||rightValue.type()==typeid(float))
	{
		return boost::any_cast<float>(leftValue) == boost::any_cast<float>(rightValue);
	}
	else if(leftValue.type()==typeid(int)||rightValue.type()==typeid(int))
	{
		return boost::any_cast<int>(leftValue) == boost::any_cast<int>(rightValue);
	}
	else if(leftValue.type()==typeid(const char*) || leftValue.type()==typeid(const char*))
	{
		std::string leftStr = boost::any_cast<const char*> (leftValue);
		std::string rightStr = boost::any_cast<const char*> (rightValue);
		return leftStr == rightStr;
	}
	else if(leftValue.type()==typeid(std::string) || leftValue.type()==typeid(std::string ))
	{
		std::string leftStr = boost::any_cast<std::string> (leftValue);
		std::string rightStr = boost::any_cast<std::string> (rightValue);
		return leftStr == rightStr;
	}
}
//the precision of  timestamp is milisecond
//the duration should also be translated to milisecond
//
TimeDuration QueryUtility::getDuration(std::string durationSpecificaiton)
{
	if (durationSpecificaiton == ""	)
	{
		return 0;
	}
	boost::regex re("(\\d+)(\\w+)");
	boost::cmatch matches;
	if (boost::regex_match(durationSpecificaiton.c_str(), matches, re))
	{
		std::string numberString = matches[1].first;
		std::string unitString = matches[2].first;

		//std::cout << "unitString " << unitString << std::endl;

		int number = atoi (numberString.c_str());
	        if(unitString == "s" || unitString == "second")
	        {
		 	return number *1000*1000;
	        }
 		else if (unitString == "millisecond" )
		{
			return number * 1000;
		}
 		else if (unitString == "microsecond" )
		{
			return number;
		}
	        else
	        {
	       		assert(false);
	       		return -1;//never reached
		}
        }
	else
	{
		std::cout<<"the duration specification is not correct"<<std::endl;
		assert(false);
		return -1;//never reached
	}
}
