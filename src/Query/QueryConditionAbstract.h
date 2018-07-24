//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Query/QueryExpressionAbstract.h"
#include <boost/shared_ptr.hpp>
#include "../Internal/Element/Element.h"

class QueryConditionAbstract
{
private:

public:
	virtual bool getValue(Element& element)=0;
	QueryConditionAbstract(void){};
	virtual ~QueryConditionAbstract(void){};
};

class BoolQueryCondition: public QueryConditionAbstract
{
private:
	boost::shared_ptr<QueryExpressionAbstract> expression ;
public:

	BoolQueryCondition(void){};
	virtual ~BoolQueryCondition(void){};
	bool getValue(Element& element)
	{
		boost::any value = expression->getValue(element);
		assert(value.type() == typeid(bool));
		return boost::any_cast<bool>(value);
	}
	void setExpression(boost::shared_ptr<QueryExpressionAbstract> expression)
	{
		this->expression = expression;
	}
};

class NegativeQueryCondition: public QueryConditionAbstract
{
private:
	boost::shared_ptr<QueryConditionAbstract> condition ;
public:

	NegativeQueryCondition(void){};
	virtual ~NegativeQueryCondition(void){};
	bool getValue(Element& element)
	{
		return !this->condition->getValue(element);
	};
	void setCondition(boost::shared_ptr<QueryConditionAbstract> condition)
	{
		this->condition = condition;
	}
};
enum CompareLogicConditionOperation
{
	CO_GT, CO_GE, CO_LT, CO_LE, CO_EQ, CO_NE
};
class CompareQueryCondition: public QueryConditionAbstract
{
private:
	CompareLogicConditionOperation operation;


	boost::shared_ptr<QueryExpressionAbstract> leftExpression ;
	boost::shared_ptr<QueryExpressionAbstract> rightExpression ;
public:

	CompareQueryCondition(void){};
	virtual ~CompareQueryCondition(void){};

	void setCompareLogicConditionOperation(CompareLogicConditionOperation operation)
	{
		this->operation = operation;
	}
	void setLeftExpression(boost::shared_ptr<QueryExpressionAbstract> expression)
	{
		this->leftExpression = expression;
	}
	void setRightExpression(boost::shared_ptr<QueryExpressionAbstract> expression)
	{
		this->rightExpression = expression;
	}

	bool getValue(Element& element)
	{
		switch (this->operation)
		{
		case CO_GT:
			{
				boost::any leftValue = this->leftExpression->getValue(element);
				boost::any rightValue = this->rightExpression->getValue(element);
				if(leftValue.type()==typeid(double)||rightValue.type()==typeid(double))
				{
					return boost::any_cast<double>(leftValue) > boost::any_cast<double>(rightValue);
				}
				else if(leftValue.type()==typeid(float)||rightValue.type()==typeid(float))
				{
					return boost::any_cast<float>(leftValue) > boost::any_cast<float>(rightValue);
				}
				else if(leftValue.type()==typeid(int)||rightValue.type()==typeid(int))
				{
					return boost::any_cast<int>(leftValue) > boost::any_cast<int>(rightValue);
				}
				break;

			}
		case CO_GE:
			{
				boost::any leftValue = this->leftExpression->getValue(element);
				boost::any rightValue = this->rightExpression->getValue(element);
				if(leftValue.type()==typeid(double)||rightValue.type()==typeid(double))
				{
					return boost::any_cast<double>(leftValue) >= boost::any_cast<double>(rightValue);
				}
				else if(leftValue.type()==typeid(float)||rightValue.type()==typeid(float))
				{
					return boost::any_cast<float>(leftValue) >= boost::any_cast<float>(rightValue);
				}
				else if(leftValue.type()==typeid(int)||rightValue.type()==typeid(int))
				{
					return boost::any_cast<int>(leftValue) >= boost::any_cast<int>(rightValue);
				}
				break;

			}
		case CO_LT:
			{
				boost::any leftValue = this->leftExpression->getValue(element);
				boost::any rightValue = this->rightExpression->getValue(element);
				if(leftValue.type()==typeid(double)||rightValue.type()==typeid(double))
				{
					return boost::any_cast<double>(leftValue) < boost::any_cast<double>(rightValue);
				}
				else if(leftValue.type()==typeid(float)||rightValue.type()==typeid(float))
				{
					return boost::any_cast<float>(leftValue) < boost::any_cast<float>(rightValue);
				}
				else if(leftValue.type()==typeid(int)||rightValue.type()==typeid(int))
				{
					return boost::any_cast<int>(leftValue) < boost::any_cast<int>(rightValue);
				}
				break;

			}
		case CO_LE:
			{
				boost::any leftValue = this->leftExpression->getValue(element);
				boost::any rightValue = this->rightExpression->getValue(element);
				if(leftValue.type()==typeid(double)||rightValue.type()==typeid(double))
				{
					return boost::any_cast<double>(leftValue) <= boost::any_cast<double>(rightValue);
				}
				else if(leftValue.type()==typeid(float)||rightValue.type()==typeid(float))
				{
					return boost::any_cast<float>(leftValue) <= boost::any_cast<float>(rightValue);
				}
				else if(leftValue.type()==typeid(int)||rightValue.type()==typeid(int))
				{
					return boost::any_cast<int>(leftValue) <= boost::any_cast<int>(rightValue);
				}
				break;

			}
		case CO_EQ:
			{
				boost::any leftValue = this->leftExpression->getValue(element);
				boost::any rightValue = this->rightExpression->getValue(element);
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
				else if(leftValue.type()==typeid(std::string) && rightValue.type()==typeid(std::string))
				{
					return boost::any_cast<std::string>(leftValue) == boost::any_cast<std::string> (rightValue);
				}
				break;

			}
		case CO_NE:
			{
				boost::any leftValue = this->leftExpression->getValue(element);
				boost::any rightValue = this->rightExpression->getValue(element);
				if(leftValue.type()==typeid(double)||rightValue.type()==typeid(double))
				{
					return boost::any_cast<double>(leftValue) != boost::any_cast<double>(rightValue);
				}
				else if(leftValue.type()==typeid(float)||rightValue.type()==typeid(float))
				{
					return boost::any_cast<float>(leftValue) != boost::any_cast<float>(rightValue);
				}
				else if(leftValue.type()==typeid(int)||rightValue.type()==typeid(int))
				{
					return boost::any_cast<int>(leftValue) != boost::any_cast<int>(rightValue);
				}
				break;

			}
		default:
			assert(false);
			break;
		}
		assert(false);
		return false;
	}
};


enum BinaryLogicConditionOperation
{
	CO_AND, CO_OR
};

class BinaryLogicQueryCondition: public QueryConditionAbstract
{
private:
	BinaryLogicConditionOperation operation;

	boost::shared_ptr<QueryConditionAbstract> leftCondition ;
	boost::shared_ptr<QueryConditionAbstract> rightCondition ;
public:

	BinaryLogicQueryCondition(void){};
	virtual ~BinaryLogicQueryCondition(void){};

	void setBinaryLogicConditionOperation(BinaryLogicConditionOperation operation)
	{
		this->operation = operation;
	}
	void setLeftCondition(boost::shared_ptr<QueryConditionAbstract> condition)
	{
		this->leftCondition = condition;
	}
	void setRightCondition(boost::shared_ptr<QueryConditionAbstract> condition)
	{
		this->rightCondition = condition;
	}

	bool getValue(Element& element)
	{
		//std::cout << element << std::endl;
		switch (this->operation)
		{
		case CO_AND:
			{
				bool leftValue = this->leftCondition->getValue(element);
				bool rightValue = this->rightCondition->getValue(element);
				return leftValue && rightValue;
				break;

			}
		case CO_OR:
			{
				bool leftValue = this->leftCondition->getValue(element);
				bool rightValue = this->rightCondition->getValue(element);
				return leftValue || rightValue;
				break;

			}
		default:
			assert(false);
			break;
		}
	}
};
