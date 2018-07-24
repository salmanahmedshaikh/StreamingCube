//////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017 KDE Laboratory, University of Tsukuba, Tsukuba, Japan.            //
//                                                                                      //
// The JsSpinnerSPE/StreamingCube and the accompanying materials are made available     //
// under the terms of Eclipse Public License v1.0 which accompanies this distribution   //
// and is available at <https://eclipse.org/org/documents/epl-v10.php>                  //
//////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "../Common/stdafx.h"
#include "../Operator/Operator.h"
#include "../Common/Types.h"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
// an example of query intermediate representation
/*{"type":"root",
	"input":{"type":"istream",
	         "input":{"type":"projection",
			           "projection_attibute_set":["id"],
					   "input":{"type":"selection",
	                            "selection_condition":"num>0",
								"input":{"type":"join",
	                                     "left_join_attribute":"id",
	                                     "right_join_attribute":"id",
										 "left_input":{"type":"rowwindow",
	                                                   "windowsize":3,
													   "input":{"type":"leaf",
	                                                            "stream_source":"stream1"
	                                                           }
	                                                   },
                                          "right_input":{"type":"rowwindow",
										                "windowsize":3,
														"input":{"type":"leaf",
										                         "stream_source":"stream2"
										                        }
										                }
	                                     }
	                            }
	                  }
	       }
	}*/
static const std::string QUERY_TYPE = "type";
static const std::string QUERY_INPUT = "input";
static const std::string QUERY_ROOT = "root";
static const std::string QUERY_ISTREAM = "istream";
static const std::string QUERY_DSTREAM = "dstream";
static const std::string QUERY_RSTREAM = "rstream";
static const std::string QUERY_PROJECTION = "projection";
static const std::string QUERY_PROJECTION_CONTENT = "projection";
static const std::string QUERY_SELECTION = "selection";
static const std::string QUERY_SELECTION_CONDITION = "selection_condition";
static const std::string QUERY_JOIN = "join";
static const std::string QUERY_LEFT_OUTER = "left_outer";
static const std::string QUERY_RIGHT_OUTER = "right_outer";
static const std::string QUERY_LEFT_JOIN_ATTRIBUTE = "left_join_attribute";
static const std::string QUERY_RIGHT_JOIN_ATTRIBUTE = "right_join_attribute";
static const std::string QUERY_LEFT_INPUT = "left_input";
static const std::string QUERY_RIGHT_INPUT = "right_input";
static const std::string QUERY_ROWWINDOW = "rowwindow";
static const std::string QUERY_RANGEWINDOW = "rangewindow";
static const std::string QUERY_WINDOW_SIZE = "windowsize";
static const std::string QUERY_LEAF = "leaf";

static const std::string QUERY_RELATION_LEAF = "relation_leaf";
static const std::string QUERY_TABLE_NAME = "table_name";
static const std::string QUERY_TABLE_SOURCE = "table_source";

static const std::string QUERY_CSV_LEAF = "csv_leaf";
static const std::string QUERY_CSV_SOURCE = "csv_source";

static const std::string QUERY_CUBIFY = "cubify";

static const std::string QUERY_STREAM_SOURCE = "stream_source";
static const std::string QUERY_IS_MASTER_SOURCE = "is_master";
static const std::string QUERY_GROUPBY_AGGREGATION = "groupby_aggregation";
static const std::string QUERY_GROUPBY_ATTRIBUTENAME =  "groupby_attribute_name";
static const std::string QUERY_AGGRAGATION_ATTRIBUTENAME =  "aggregation_attribute_name";
static const std::string QUERY_RESULT_ATTRIBUTENAME =  "result_attribute_name";
static const std::string QUERY_SMART_ROW_WINDOW = "smartrowwinodw";
static const std::string QUERY_SMART_RANGE_WINDOW = "smartrangewinodw";

//condition for selection
static const std::string QUERY_CONDITION_TYPE = "condition_type";
static const std::string QUERY_LEFT_CONDITION = "left_condition";
static const std::string QUERY_RIGHT_CONDITION = "right_condition";
static const std::string QUERY_CONDITION = "condition";

static const std::string QUERY_EXPRESSION_TYPE = "expression_type";
static const std::string QUERY_LEFT_EXPRESSION = "left_expression";
static const std::string QUERY_RIGHT_EXPRESSION = "right_expression";
static const std::string QUERY_LEFT = "left";
static const std::string QUERY_GROUP_ARRAY = "group_array";
static const std::string QUERY_KEY_VAR = "group_key_var";
static const std::string QUERY_RIGHT = "right";
static const std::string QUERY_EXPRESSION = "expression";
static const std::string QUERY_ID_NAME = "id_name";
static const std::string QUERY_INT_VALUE = "int_value";
static const std::string QUERY_DOUBLE_VALUE = "double_value";
static const std::string QUERY_STRING_VALUE = "string_value";
static const std::string QUERY_BOOL_VALUE = "bool_value";
static const std::string QUERY_ATTRIBUTE_SOURCE = "attribute_source";
static const std::string QUERY_AGGREGATE_OPERATION = "aggregate_operation";
static const std::string QUERY_AGGREGATE_PROJECTION = "aggregate_projection";

static const std::string QUERY_NEED_RENAME = "need_rename";
static const std::string QUERY_RENAME = "rename";
static const std::string QUERY_PROJECTION_TYPE = "projection_type";
static const std::string QUERY_FIELDS = "fields";
static const std::string QUERY_ARRAY_ITEMS = "array_items";

class QueryEntity;
class QueryIntermediateRepresentationInterpreter
{
private:
	QueryIntermediateRepresentationInterpreter(void);
	static QueryIntermediateRepresentationInterpreter* queryIntermediateRepresentationInterpreter;
	int streamSourceCount;
	int nonMasterStreamSourceCount;
	bool replicateStreamSource;
	void connectTwoOperator(boost::shared_ptr<Operator>inputOperator,boost::shared_ptr<Operator>outputOperator);
	void connectThreeOperator(boost::shared_ptr<Operator>leftInputOperator,boost::shared_ptr<Operator>rightInputOperator,boost::shared_ptr<Operator>outputOperator);
public:
	static QueryIntermediateRepresentationInterpreter* getInstance();
	~QueryIntermediateRepresentationInterpreter(void);
	/*resolve an document specified in the query intermediate representation
	  create the corresponding operators and queues and connect them
	  return the operator created
	 */
    boost::shared_ptr<Operator> resolve(Document& document, QueryEntity* queryEntity);
};
