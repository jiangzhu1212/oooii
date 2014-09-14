// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oString/json.h>
#include <oString/string_codec.h>
#include "../../test_services.h"

namespace ouro { namespace tests {

static const char* sJSONTestReferenceResult = 
	"{"
		"\"Bool\":true,"
		"\"Char\":-126,"
		"\"Uchar\":254,"
		"\"Int\":518504670,"
		"\"Llong\":54491065317,"
		"\"Float\":-1.5,"
		"\"Double\":1.50505e-015,"
		"\"String\":\"Some test text for \\\"JSON\\\" with some\\r\\n\\tcharacters:\\f\\b\\t\\\\ that need to be escaped and/or turned into unicode format:\\u000b\\u0007\\u001b\","
		"\"Array\":[0,1,2,3],"
		"\"ArrayOfArray\":[[true],[54491065317],[-1.5],[{\"Bool\":true}],\"Text\",true,-1,null],"
		"\"Struct\":"
			"{"
				"\"StructValue1\":null,"
				"\"StructValue2\":[0,1,2,3]"
			"}"
	"}";

static void TESTjson_node(test_services& services, const json& _JSON, int _RootNode, int _Node, const char* _Name, json_node_type _Type, json_value_type _ValueType, const char* _Value)
{
	auto node = json::node(_Node);
	if (_Name)
	{
		// Test find
		oTEST0(_JSON.first_child((json::node)_RootNode, _Name) == node);
	}
	const char* NodeName = _JSON.node_name(node);
	const char* NodeValue = _Type == json_node_type::value ? _JSON.node_value(node) : nullptr;
	oTEST0(0 == strcmp(NodeName ? NodeName : "", _Name ? _Name : ""));
	oTEST0(_JSON.node_type(node) == _Type);
	oTEST0(_JSON.value_type(node) == _ValueType);
	oTEST0(0 == strcmp(NodeValue ? NodeValue : "", _Value ? _Value : ""));
}

void TESTjson(test_services& services)
{
	json JSON("Test JSON", sJSONTestReferenceResult, default_allocator);

	// Test common API 
	oTEST0(JSON.size() >= strlen(sJSONTestReferenceResult));
	oTEST0(0 == strcmp(JSON.name(), "Test JSON"));

	// Test nodes
	TESTjson_node(services, JSON, 0, 0, nullptr, json_node_type::object, json_value_type::object, nullptr);
	TESTjson_node(services, JSON, 1, 2, "Bool", json_node_type::value, json_value_type::true_, "true");
	TESTjson_node(services, JSON, 1, 3, "Char", json_node_type::value, json_value_type::number, "-126");
	TESTjson_node(services, JSON, 1, 4, "Uchar", json_node_type::value, json_value_type::number, "254");
	TESTjson_node(services, JSON, 1, 5, "Int", json_node_type::value, json_value_type::number, "518504670");
	TESTjson_node(services, JSON, 1, 6, "Llong", json_node_type::value, json_value_type::number, "54491065317");
	TESTjson_node(services, JSON, 1, 7, "Float", json_node_type::value, json_value_type::number, "-1.5");
	TESTjson_node(services, JSON, 1, 8, "Double", json_node_type::value, json_value_type::number, "1.50505e-015");
	TESTjson_node(services, JSON, 1, 9, "String", json_node_type::value, json_value_type::string, "\"Some test text for \\\"JSON\\\" with some\\r\\n\\tcharacters:\\f\\b\\t\\\\ that need to be escaped and/or turned into unicode format:\\u000b\\u0007\\u001b\"");
	TESTjson_node(services, JSON, 1, 10, "Array", json_node_type::array, json_value_type::array, nullptr);
	// Array
	TESTjson_node(services, JSON, 10, 11, nullptr, json_node_type::value, json_value_type::number, "0");
	TESTjson_node(services, JSON, 10, 12, nullptr, json_node_type::value, json_value_type::number, "1");
	TESTjson_node(services, JSON, 10, 13, nullptr, json_node_type::value, json_value_type::number, "2");
	TESTjson_node(services, JSON, 10, 14, nullptr, json_node_type::value, json_value_type::number, "3");
	// ArrayOfArray
	TESTjson_node(services, JSON, 1, 15, "ArrayOfArray", json_node_type::array, json_value_type::array, nullptr);
	TESTjson_node(services, JSON, 15, 16, nullptr, json_node_type::array, json_value_type::array, nullptr);
	TESTjson_node(services, JSON, 16, 17, nullptr, json_node_type::value, json_value_type::true_, "true");
	TESTjson_node(services, JSON, 15, 18, nullptr, json_node_type::array, json_value_type::array, nullptr);
	TESTjson_node(services, JSON, 18, 19, nullptr, json_node_type::value, json_value_type::number, "54491065317");
	TESTjson_node(services, JSON, 15, 20, nullptr, json_node_type::array, json_value_type::array, nullptr);
	TESTjson_node(services, JSON, 20, 21, nullptr, json_node_type::value, json_value_type::number, "-1.5");
	TESTjson_node(services, JSON, 15, 22, nullptr, json_node_type::array, json_value_type::array, nullptr);
	TESTjson_node(services, JSON, 22, 23, nullptr, json_node_type::object, json_value_type::object, nullptr);
	TESTjson_node(services, JSON, 23, 24, "Bool", json_node_type::value, json_value_type::true_, "true");
	TESTjson_node(services, JSON, 15, 25, nullptr, json_node_type::value, json_value_type::string, "\"Text\"");
	TESTjson_node(services, JSON, 15, 26, nullptr, json_node_type::value, json_value_type::true_, "true");
	TESTjson_node(services, JSON, 15, 27, nullptr, json_node_type::value, json_value_type::number, "-1");
	TESTjson_node(services, JSON, 15, 28, nullptr, json_node_type::value, json_value_type::null, "null");
	// Struct
	TESTjson_node(services, JSON, 1, 29, "Struct", json_node_type::object, json_value_type::object, nullptr);
	TESTjson_node(services, JSON, 29, 30, "StructValue1", json_node_type::value, json_value_type::null, "null");
	TESTjson_node(services, JSON, 29, 31, "StructValue2", json_node_type::array, json_value_type::array, nullptr);
	TESTjson_node(services, JSON, 31, 32, nullptr, json_node_type::value, json_value_type::number, "0");
	TESTjson_node(services, JSON, 31, 33, nullptr, json_node_type::value, json_value_type::number, "1");
	TESTjson_node(services, JSON, 31, 34, nullptr, json_node_type::value, json_value_type::number, "2");
	TESTjson_node(services, JSON, 31, 35, nullptr, json_node_type::value, json_value_type::number, "3");

	lstring String;
	oTEST0(json_escape_decode(String.c_str(), String.capacity(), JSON.node_value(json::node(9))));
	oTEST0(0 == strcmp(String.c_str(), "Some test text for \"JSON\" with some\r\n\tcharacters:\f\b\t\\ that need to be escaped and/or turned into unicode format:\v\a\x1b"));

	lstring EscapedString;
	json_escape_encode(EscapedString.c_str(), EscapedString.capacity(), "Some test text for \"JSON\" with some\r\n\tcharacters:\f\b\t\\ that need to be escaped and/or turned into unicode format:\v\a\x1b");
	oTEST0(0 == strcmp(EscapedString.c_str(), JSON.node_value(json::node(9))));
}

}}
