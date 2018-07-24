//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Internal/Element/Element.h"
#include <boost/any.hpp>
#include <assert.h>
#include "../Query/QueryAttribute.h"
class QueryExpressionAbstract {
public:
	QueryExpressionAbstract(void) {
	}
	;
	virtual ~QueryExpressionAbstract(void) {
	}
	;
	boost::any getValue(Element& element) {
		return getValue(element.document);
	}
	virtual boost::any getValue(Document& document) = 0;
};

enum CalculationExpressionOperation {
	EO_ADD, EO_SUB, EO_MUL, EO_DIV, EO_MOD
};

enum AggregateOperation {
	SUM, AVG, COUNT, MAX, MIN
};
class CalculationQueryExpression: public QueryExpressionAbstract {
private:
	CalculationExpressionOperation operation;

	boost::shared_ptr<QueryExpressionAbstract> leftExpression;
	boost::shared_ptr<QueryExpressionAbstract> rightExpression;
public:
	CalculationQueryExpression(void) {
	}
	;
	virtual ~CalculationQueryExpression(void) {
	}
	;
	void setOperation(CalculationExpressionOperation operation) {
		this->operation = operation;
	}
	void setLeftExpression(
			boost::shared_ptr<QueryExpressionAbstract> leftExpression) {
		this->leftExpression = leftExpression;
	}
	void setRightExpression(
			boost::shared_ptr<QueryExpressionAbstract> rightExpression) {
		this->rightExpression = rightExpression;
	}
	virtual boost::any getValue(Document& document) {
		boost::any leftValue = this->leftExpression->getValue(document);
		boost::any rightValue = this->rightExpression->getValue(document);
		switch (this->operation) {
		case EO_ADD: {
			boost::any leftValue = this->leftExpression->getValue(document);
			boost::any rightValue = this->rightExpression->getValue(document);
			if (leftValue.type() == typeid(double) || rightValue.type()
					== typeid(double)) {
				return boost::any_cast<double>(leftValue) + boost::any_cast<
						double>(rightValue);
			} else if (leftValue.type() == typeid(float) || rightValue.type()
					== typeid(float)) {
				return boost::any_cast<float>(leftValue) + boost::any_cast<
						float>(rightValue);
			} else if (leftValue.type() == typeid(int) || rightValue.type()
					== typeid(int)) {
				return boost::any_cast<int>(leftValue) + boost::any_cast<int>(
						rightValue);
			}
			break;
		}
		case EO_SUB: {
			boost::any leftValue = this->leftExpression->getValue(document);
			boost::any rightValue = this->rightExpression->getValue(document);
			if (leftValue.type() == typeid(double) || rightValue.type()
					== typeid(double)) {
				return boost::any_cast<double>(leftValue) - boost::any_cast<
						double>(rightValue);
			} else if (leftValue.type() == typeid(float) || rightValue.type()
					== typeid(float)) {
				return boost::any_cast<float>(leftValue) - boost::any_cast<
						float>(rightValue);
			} else if (leftValue.type() == typeid(int) || rightValue.type()
					== typeid(int)) {
				return boost::any_cast<int>(leftValue) - boost::any_cast<int>(
						rightValue);
			}
			break;
		}
		case EO_MUL: {
			boost::any leftValue = this->leftExpression->getValue(document);
			boost::any rightValue = this->rightExpression->getValue(document);
			if (leftValue.type() == typeid(double) || rightValue.type()
					== typeid(double)) {
				return boost::any_cast<double>(leftValue) * boost::any_cast<
						double>(rightValue);
			} else if (leftValue.type() == typeid(float) || rightValue.type()
					== typeid(float)) {
				return boost::any_cast<float>(leftValue) * boost::any_cast<
						float>(rightValue);
			} else if (leftValue.type() == typeid(int) || rightValue.type()
					== typeid(int)) {
				return boost::any_cast<int>(leftValue) * boost::any_cast<int>(
						rightValue);
			}
			break;
		}
		case EO_DIV: {
			boost::any leftValue = this->leftExpression->getValue(document);
			boost::any rightValue = this->rightExpression->getValue(document);
			assert(boost::any_cast<int>(rightValue)!=0);
			if (leftValue.type() == typeid(double) || rightValue.type()
					== typeid(double)) {
				return boost::any_cast<double>(leftValue) / boost::any_cast<
						double>(rightValue);
			} else if (leftValue.type() == typeid(float) || rightValue.type()
					== typeid(float)) {
				return boost::any_cast<float>(leftValue) / boost::any_cast<
						float>(rightValue);
			} else if (leftValue.type() == typeid(int) || rightValue.type()
					== typeid(int)) {
				return boost::any_cast<int>(leftValue) / boost::any_cast<int>(
						rightValue);
			}
			break;
		}
		case EO_MOD: {
			boost::any leftValue = this->leftExpression->getValue(document);
			boost::any rightValue = this->rightExpression->getValue(document);
			assert(leftValue.type()==typeid(int));
			assert(rightValue.type()==typeid(int));
			return boost::any_cast<int>(leftValue) % boost::any_cast<int>(
					rightValue);

			break;
		}
		default:
			break;
		}
	}
	;
};

enum AttrSource {
	_Any, _LEFT, _RIGHT, _GROUP_KEY_VAR, _GROUP_ARRAY,
};

class AggregationQueryExpression:public QueryExpressionAbstract
{
private:
	//todo wangyan

	AggregateOperation operation;
	std::string aggregationAttributeName;
	AttrSource attributeSource;
public:
	AggregationQueryExpression(void){};
	virtual ~AggregationQueryExpression(void){};
	virtual boost::any getValue(Document& document)
	{
		// todo dongchao
		boost::shared_ptr<QueryAttribute> queryAttribute(new QueryAttribute());
		boost::any idName;
		if (this->operation == AVG) {
			idName = "avg(" + this->aggregationAttributeName + ")";
		} else if (this->operation == SUM) {
			idName = "sum(" + this->aggregationAttributeName + ")";
		} else if (this->operation == COUNT) {
			idName = "count(" + this->aggregationAttributeName + ")";
		} else if (this->operation == MIN) {
			idName = "min(" + this->aggregationAttributeName + ")";
		} else if (this->operation == MAX) {
			idName = "max(" + this->aggregationAttributeName + ")";
		}
		return queryAttribute->getValueDirectByIdName(document, idName);
	};

	void setOperation(AggregateOperation operation) {
		this->operation = operation;
	}

	AggregateOperation getOperation() {
		return operation;
	}

	const std::string getAggregationAttributeName() const {
		return aggregationAttributeName;
	}

	void setAggregationAttributeName(
			const std::string aggregationAttributeName) {
		this->aggregationAttributeName = aggregationAttributeName;
	}

	AttrSource getAttributeSource() const {
		return attributeSource;
	}

	void setAttributeSource(AttrSource attributeSource) {
		this->attributeSource = attributeSource;
	}
};

//enum AttrSource
//{
//	_Any, _LEFT, _RIGHT, _GROUP_KEY_VAR, _GROUP_ARRAY,
//};

class IdentifierQueryExpression: public QueryExpressionAbstract {
private:
	boost::shared_ptr<QueryAttribute> attribute;

	AttrSource attributeSource;

public:
	IdentifierQueryExpression(void) {
	}
	;
	virtual ~IdentifierQueryExpression(void) {
	}
	;

	void setAttrSource(AttrSource attrSource) {
		this->attributeSource = attrSource;
	}
	void setAttibute(boost::shared_ptr<QueryAttribute> attribute) {
		this->attribute = attribute;
	}
	virtual boost::any getValue(Document& document) {

		//when attribute_source == group_key_var, need to pass the groupby_attribute_name?
		//or change the JSON format?   this works
		//dongchao

		return this->attribute->getValue(document);
	}
	;
	virtual boost::any getValue(Document& leftDocument, Document&rightDocument) {
		if (this->attributeSource == _LEFT) {
			return this->attribute->getValue(leftDocument);
		} else if (this->attributeSource == _RIGHT) {
			return this->attribute->getValue(rightDocument);
		}

	}
	;
	std::string getLastAttribute(void) {
		return this->attribute->getLastAttribute();

	}
};

class IntegerQueryExpression: public QueryExpressionAbstract {
private:
	int value;
public:
	IntegerQueryExpression(void) {
	}
	;
	virtual ~IntegerQueryExpression(void) {
	}
	;
	void setValue(int value) {
		this->value = value;
	}
	virtual boost::any getValue(Document& document) {
		return value;
	}
	;
};

class DoubleQueryExpression: public QueryExpressionAbstract {
private:
	double value;
public:
	DoubleQueryExpression(void) {
	}
	;
	virtual ~DoubleQueryExpression(void) {
	}
	;
	void setValue(double value) {
		this->value = value;
	}
	virtual boost::any getValue(Document& document) {
		return value;
	}
	;
};

class BoolQueryExpression: public QueryExpressionAbstract {
private:
	bool value;
public:
	BoolQueryExpression(void) {
	}
	;
	virtual ~BoolQueryExpression(void) {
	}
	;
	void setValue(bool value) {
		this->value = value;
	}
	virtual boost::any getValue(Document& document) {
		return value;
	}
	;
};

class StringQueryExpression: public QueryExpressionAbstract {
private:
	std::string value;
public:
	StringQueryExpression(void) {
	}
	;
	virtual ~StringQueryExpression(void) {
	}
	;
	void setValue(std::string value) {
		this->value = value;
	}
	virtual boost::any getValue(Document& document) {
		return value;
	}
	;
};

class NullQueryExpression: public QueryExpressionAbstract {
private:

public:
	NullQueryExpression(void) {
	}
	;
	virtual ~NullQueryExpression(void) {
	}
	;
	virtual boost::any getValue(Document& document) {
		return NULL;
	}
	;
};
