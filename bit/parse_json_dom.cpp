/*
 * parse_json_dom.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#include "parse_json_dom.h"

#include "jsmn.h"

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

using std::cout;
using std::endl;

bool tokenize_json(const char* file, std::vector<jsmntok_t>& tokens, std::string& jstext)
{
	jsmn_parser jp;
	jsmn_init(&jp);
	int fd = open(file, O_RDONLY);
	int jslength = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	jstext.resize(jslength);
	read(fd, &jstext[0], jslength);
	int tokes = jsmn_parse(&jp, jstext.data(), jslength, NULL, 0);
	tokens.resize(tokes);
	jsmn_init(&jp);
	jsmn_parse(&jp, jstext.data(), jslength, tokens.data(), tokes);
	close(fd);
	return true;
}
void dump_jstokens(const std::vector<jsmntok_t>& tokens, const std::string& jstext)
{
	for (int i=0; i<tokens.size(); ++i)
	{
		const char* types[] = {
				"JSMN_UNDEFINED",
				"JSMN_OBJECT",
				"JSMN_ARRAY",
				"JSMN_STRING",
				"JSMN_PRIMITIVE"
		};
		cout << i << ": type=" << types[tokens[i].type] << " start: " << tokens[i].start << " end: " << tokens[i].end
			<< " size: " << tokens[i].size;
		switch (tokens[i].type)
		{
		case JSMN_STRING:
			if (tokens[i].end - tokens[i].start < 20 || (tokens[i].end - tokens[i].start < 100))
			{
				cout << " \"" << jstext.substr(tokens[i].start, tokens[i].end-tokens[i].start) << '"';
			}
			cout << endl;
			break;
		case JSMN_PRIMITIVE:
			cout << " " << jstext.substr(tokens[i].start, tokens[i].end-tokens[i].start);
			cout << endl;
			break;
		case JSMN_ARRAY:
			cout << endl;
			break;
		case JSMN_OBJECT:
			cout << endl;
			break;
		default:
			cout << endl;
			break;
		}
	}
}

std::string JSInt::to_string() const {
	return std::to_string(value);
}
std::string JSBool::to_string() const {
	return std::to_string(value);
}
std::string JSNull::to_string() const {
	return "null";
}
std::string JSString::to_string() const {
	return value;
}
std::string JSObject::to_string() const {
	std::ostringstream os;
	os << "{";
	bool comma = false;
	for (auto m : value)
	{
		if (comma)
			os << ',';
		os << m.first << ":" << m.second->to_string();
		comma = true;
	}
	os << "}\n";
	return os.str();
}
std::string JSArray::to_string() const {
	std::ostringstream os;
	os << "[";
	bool comma = false;
	for (auto m : value)
	{
		if (comma)
			os << ',';
		os << m->to_string();
		comma = true;
	}
	os << "]\n";
	return os.str();
}

void print_object(JSValue* obj)
{
	cout << obj->to_string() << endl;
}

JSValue* jsread(const char* jstext, std::vector<jsmntok_t>::iterator& ptoken)
{
	JSArray* array;
	JSObject* object;
	JSString* str;
	JSNull* null;
	JSInt* num;
	JSBool* truth;
	unsigned expected;
	switch (ptoken->type)
	{
	case JSMN_PRIMITIVE:
		switch (jstext[ptoken->start])
		{
		case 'n':
			null = new JSNull();
			ptoken++;
			return null;
		case 't': case 'f':
			truth = new JSBool(jstext[ptoken->start]=='t');
			ptoken++;
			return truth;
		default:
			num = new JSInt(std::stod(std::string(jstext+ptoken->start, ptoken->end-ptoken->start)));
			ptoken++;
			return num;
		}
		break;
	case JSMN_STRING:
		str = new JSString(std::string(jstext+ptoken->start, ptoken->end-ptoken->start));
		ptoken++;
		return str;
	case JSMN_ARRAY:
		array = new JSArray();
		expected = ptoken->size;
		ptoken++;
		for (unsigned i=0; i<expected; ++i)
			array->value.push_back(jsread(jstext, ptoken));
		//cout << "array: "; print_object(array); cout << endl;
		return array;
	case JSMN_OBJECT:
		object = new JSObject();
		expected = ptoken->size;
		ptoken++;
		//cout << "obj: expect " << expected << endl;
		for (unsigned i=0; i<expected; ++i)
		{
			JSValue* jskey = jsread(jstext, ptoken);
			std::string key = dynamic_cast<JSString*>(jskey)->value;
			JSValue* value = jsread(jstext,ptoken);
			object->value[key] = value;
		}
		//cout << "obj: "; print_object(object); cout << endl;
		return object;
	}
	return new JSNull();
}
JSValue* parse_json(const char* file)
{
	std::vector<jsmntok_t> tokens;
	std::string jstext;
	tokenize_json(file, tokens, jstext);
	cout << "js: chars provided " << jstext.size() << endl;
	dump_jstokens(tokens, jstext);
	auto ptokens = tokens.begin();
	JSValue* root = jsread(&jstext[0], ptokens);
	print_object(root);
	return root;
}



