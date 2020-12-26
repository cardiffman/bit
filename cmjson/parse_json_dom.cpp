/*
 * parse_json_dom.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#include "parse_json_dom.h"

#ifdef USE_JSMN
#include "jsmn.h"
#endif

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> // strtoul
#include "JSValue.h"

using std::cout;
using std::endl;

bool isblank(char c) { return c==' ' || c=='\t' || c=='\n' || c=='\r'; }

bool parse_string(std::string& str, const char*& p, const char* end) {
	const char* start = p;
	if (*p=='"') 
	{
		std::string s;
		//cout << "Got quote now " << p << endl;
		p++;
		while (p != end && *p!='"') {
			//cout << "Remaineder " << p << endl;
			if (*p=='\\' && p < end-1) {
				switch (p[1]) {
				case '"':
				case '/':
				case '\\': s.push_back(p[1]); break; 
				case 'n': s.push_back('\n'); break;
				case 't': s.push_back('\t'); break;
				case 'r': s.push_back('\r'); break;
				case 'b': s.push_back('\b'); break;
				}
			} else if (*p=='\\' && p < end-5 && p[1]=='u' && isxdigit(p[2]) && isxdigit(p[3]) && isxdigit(p[4]) && isxdigit(p[5])) {
				std::string str(p+2,4);
				unsigned ch = strtoul(str.c_str(),NULL,16);
				s.push_back(ch);
				p+=6;
			}
			else {
				s.push_back(*p++);
			}
		}
			//cout << "Remaineder after 2nd quote " << p << endl;
		if (p != end && *p=='"') {
			str = s;
			++p;
			return true;
		}
	}
	return false;
}
JSValue parse_json(const char*& p, const char* end) {
	std::string s;
	while (p !=end && isblank(*p))
		++p;
	if (parse_string(s, p, end)) {
		return JSValue(s);
	}
	s.push_back(*p++);
	if (s[0]=='{') {
		JSValue values;
		while (p != end && isblank(*p)) p++;
		while (p != end && *p != '}')
		{
			std::string name;
			//cout <<  "Looking for name from " << p << endl;
			parse_string(name, p, end);
			cout << "name:" << name << endl;
			while (p != end && isblank(*p)) p++;
			cout << "Got colon now " << p << endl;
			if (p != end && *p==':') p++;
			//cout << "Looking at the next thing " << p << endl;
			JSValue value = parse_json(p, end);
			//cout << "Got the next thing what's left is " << p << endl;
			values[name] = value;
			cout << "Entered a value" << endl;
			while (p != end && isblank(*p)) p++;
			if (p != end && *p==',') p++;
			while (p != end && isblank(*p)) p++;
		}
		p++;
		return values;
	} else if (s[0]=='[') {
		JSValue values;
		while (p != end && isblank(*p)) p++;
		int index = 0;
		while (p != end && *p!=']') {
			JSValue v = parse_json(p, end);
			//cout << "Got the next thing what's left is " << p << endl;
			values[index++] = v;
			cout << "Entered a value" << endl;
			while (p != end && isblank(*p)) p++;
			if (p != end && *p==',') {
				p++;
				continue;
			}
		}
		p++;
		return JSValue(values);
	} else if (s[0]=='-' || isdigit(s[0])) {
		//cout << "Looking for digits starting with " << s << " and " << p << endl;
		bool flt = false;
		while (p != end && isdigit(*p)) {
			s.push_back(*p++);
		}
		//cout << "Found digits the rest is " << p << endl;
		if (p != end && *p=='.') {
			s.push_back(*p++);
			while (p != end && isdigit(*p)) {
				s.push_back(*p++);
				flt = true;
			}
		}
		if (p != end && *p=='e' || *p=='E') {
			flt = true;
			s.push_back(*p++);
			if (p != end && (*p=='-' || *p=='+')) {
				s.push_back(*p++);
			}
			while (p != end && isdigit(*p)) {
				s.push_back(*p++);
				flt = true;
			}
		}
		// at this point we have some number in s.
		cout << "Parser recognizes " << s << " as a number. It is" << (flt?"n't":"") << " an integer" << endl;
		return flt ? JSValue(strtod(s.c_str(), NULL)) : JSValue((int)strtol(s.c_str(),NULL,10));
	} else if (s[0]=='t' || s[0]=='f' || s[0]=='n') {
		while (p != end && isalpha(*p)) {
			s.push_back(*p++);
		}
		if (s=="true") {
			//cout <<"Parser recognizes " << s << " as a boolean" << endl;
			return JSValue(true);
		} else if (s=="false") {
			//cout <<"Parser recognizes " << s << " as a boolean" << endl;
			return JSValue(false);
		} else if (s=="null") {
			//cout <<"Parser recognizes " << s << " as a null" << endl;
			return JSValue();
		}
	} else{
		cout << p << endl;
		throw "Unexpected";
	}
	return JSValue("wrong");
}
#if 0
JSValue JSValue::operator[](int index) const {
	struct IndexVisitor : JSValueVisitor {
		IndexVisitor(int i) : i(i){}
		void visit(const JSValueNull& v) {}
		void visit(const JSValueInt& v) {}
		void visit(const JSValueDouble& v) {}
		void visit(const JSValueBool& v) {}
		void visit(const JSValueString& v) {}
		void visit(const JSValueArray& v){ p=&(v.value())[i];}
		void visit(const JSValueObject& v){}
		int i;
		const JSValue* p;
	};
	IndexVisitor iv(index);
	return *iv.p;
}
JSValue JSValue::operator[](const char* index) const {
	struct ObjectVisitor : JSValueVisitor {
		ObjectVisitor(const char* i) : i(i){}
		void visit(const JSValueNull& v) {}
		void visit(const JSValueInt& v) {}
		void visit(const JSValueDouble& v) {}
		void visit(const JSValueBool& v) {}
		void visit(const JSValueString& v) {}
		void visit(const JSValueArray& v) {}
		void visit(const JSValueObject& v){ auto pi = v.value().find(i); p = &pi->second;}
		const char* i;
		const JSValue* p;
	};
	ObjectVisitor iv(index);
	return *iv.p;
}
JSValue JSValue::operator[](const std::string& index) const {
	struct ObjectVisitor : JSValueVisitor {
		ObjectVisitor(const std::string& i) : i(i){}
		void visit(const JSValueNull& v) {}
		void visit(const JSValueInt& v) {}
		void visit(const JSValueDouble& v) {}
		void visit(const JSValueBool& v) {}
		void visit(const JSValueString& v) {}
		void visit(const JSValueArray& v) {}
		void visit(const JSValueObject& v){ auto pi = v.value().find(i); p = &pi->second;}
		const std::string& i;
		const JSValue* p;
	};
	ObjectVisitor iv(index);
	return *iv.p;
}
JSValue& JSValue::operator[](const std::string& index) {
	struct ObjectVisitor : JSValueVisitor {
		ObjectVisitor(const std::string& i) : i(i){}
		void visit(const JSValueNull& v) {}
		void visit(const JSValueInt& v) {}
		void visit(const JSValueDouble& v) {}
		void visit(const JSValueBool& v) {}
		void visit(const JSValueString& v) {}
		void visit(const JSValueArray& v) {}
		void visit(const JSValueObject& v){ auto pi = v.value().find(i); p = &pi->second;}
		const std::string& i;
		JSValue* p;
	};
	ObjectVisitor iv(index);
	return *iv.p;
}
#endif
#if 0
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
#endif

void print_object(JSValue* obj)
{
		cout << "object is type " << obj->index() << endl;
}

#ifdef NOTDEF
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
#endif

#ifdef notdef
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
#endif
