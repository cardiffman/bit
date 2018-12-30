/*
 * parse_json_dom.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_PARSE_JSON_DOM_H_
#define BIT_PARSE_JSON_DOM_H_


#include "jsmn.h"
#include <vector>
#include <string>
#include <map>

struct JSValue
{
	virtual ~JSValue() {}
	virtual std::string to_string() const = 0;
};
struct JSInt : public JSValue
{
	int value;
	JSInt(int value): value(value){}
	virtual std::string to_string() const;
};
struct JSBool : public JSValue
{
	bool value;
	JSBool(bool value): value(value){}
	virtual std::string to_string() const;
};
struct JSNull : public JSValue
{
	virtual std::string to_string() const;
};
struct JSString : public JSValue
{
	std::string value;
	JSString(const std::string& value): value(value){}
	virtual std::string to_string() const;
};
struct JSObject : public JSValue
{
	std::map<std::string, JSValue*> value;
	virtual std::string to_string() const;
	std::string key;
};
struct JSArray : public JSValue
{
	std::vector<JSValue*> value;
	virtual std::string to_string() const;
};

bool tokenize_json(const char* file, std::vector<jsmntok_t>& tokens, std::string& jstext);
void dump_jstokens(const std::vector<jsmntok_t>& tokens, const std::string& jstext);
JSValue* parse_json(const char* file);
void print_object(JSValue* obj);


#endif /* BIT_PARSE_JSON_DOM_H_ */
