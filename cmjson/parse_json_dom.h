/*
 * parse_json_dom.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef ANIM_PARSE_JSON_DOM_H_
#define ANIM_PARSE_JSON_DOM_H_


#include <map>
#include <string>
#include <vector>

#if 0
enum JSValueIndex { JS_NULL, JS_INT, JS_DOUBLE, JS_BOOL, JS_STRING, JS_ARRAY, JS_OBJECT };
class JSValueNull;
class JSValueInt;
class JSValueDouble;
class JSValueBool;
class JSValueString;
class JSValueArray;
class JSValueObject;
class JSValueVisitor {
public:
	virtual void visit(JSValueNull const & v)=0;
	virtual void visit(JSValueInt const & v)=0;
	virtual void visit(JSValueDouble const & v)=0;
	virtual void visit(JSValueBool const & v)=0;
	virtual void visit(JSValueString const & v)=0;
	virtual void visit(JSValueArray const & v)=0;
	virtual void visit(JSValueObject const & v)=0;
};
class JSValue;
class JSValueBase {
public:
	virtual ~JSValueBase() {}
	virtual int index() const = 0;
	virtual void acceptVisit(JSValueVisitor& visitor) = 0;
};
class JSValueInt : public JSValueBase {
public:
	JSValueInt(int v) : v(v) {}
	virtual int index() const { return JS_INT; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	int const & value() const { return v; }
private:
	int v;
};
class JSValueDouble : public JSValueBase {
public:
	JSValueDouble(double v) : v(v) {}
	virtual int index() const { return JS_DOUBLE; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	double const& value() const { return v; }
private:
	double v;
};
class JSValueNull : public JSValueBase {
public:
	JSValueNull(void*) : v(0) {}
	virtual int index() const { return JS_NULL; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	void* const& value() const { return v; }
private:
	void* v;
};
class JSValueBool : public JSValueBase {
public:
	JSValueBool(bool v) : v(v) {}
	virtual int index() const { return JS_BOOL; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	bool const& value() const { return v; }
private:
	bool v;
};
class JSValueString : public JSValueBase {
public:
	JSValueString(const std::string& v) : v(v) {}
	virtual int index() const { return JS_STRING; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	std::string const& value() const { return v; }
private:
	std::string v;
};
class JSValueArray : public JSValueBase {
public:
	JSValueArray(const std::vector<JSValue>& v) : v(v) {}
	virtual int index() const { return JS_ARRAY; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	std::vector<JSValue> const& value() const { return v; }
private:
	std::vector<JSValue> v;
};
class JSValueObject : public JSValueBase {
public:
	JSValueObject(const std::map<std::string,JSValue>& v) : v(v) {}
	virtual int index() const { return JS_OBJECT; };
	virtual void acceptVisit(JSValueVisitor& visitor) {
		visitor.visit(*this);
	};
	std::map<std::string,JSValue> const& value() const { return v; }
private:
	std::map<std::string,JSValue> v;
};
class JSValue {
public:
	JSValue() { pv = new JSValueInt(0); }
	JSValue(int v) { pv = new JSValueInt(v); }
	JSValue(double v) { pv = new JSValueDouble(v); }
	JSValue(bool v) { pv = new JSValueBool(v); }
	JSValue(void* v) { pv = new JSValueNull(v); }
	JSValue(const std::string& v) { pv = new JSValueString(v); }
	JSValue(const std::map<std::string,JSValue>& v) { pv = new JSValueObject(v); }
	JSValue(const std::vector<JSValue>& v) { pv = new JSValueArray(v); }
	JSValue(const JSValue& other) {
		copyVisitor cv;
		other.visit(cv);
		pv = cv.copy;
	}
	int index() const { return pv->index(); }
	void visit(JSValueVisitor& visitor) const { pv->acceptVisit(visitor); }
	JSValue& operator=(const JSValue& other) {
		delete pv; 
		copyVisitor cv;
		other.visit(cv);
		pv = cv.copy;
		return *this;
	}
	JSValue operator[](int index) const;
	JSValue operator[](const char* index) const;
	JSValue operator[](const std::string& index) const;
	JSValue& operator[](int index);
	JSValue& operator[](const char* index);
	JSValue& operator[](const std::string& index);
	struct Iterator {
		using pointer = JSValue*;
		using value_type = JSValue;
		using reference = JSValue&;
		using difference_type = int;
		Iterator(pointer ptr) : m_ptr(ptr) {}
		const JSValue& operator*() const { return *m_ptr; }
		const JSValue* operator->() const { return m_ptr; }
		JSValue* m_ptr;
		bool operator==(const Iterator& o) const;
		bool operator!=(const Iterator& o) const;
		Iterator& operator++();
		Iterator& operator++(int);
	};
	using iterator = Iterator;
	struct ConstIterator {
		using pointer = const JSValue*;
		using value_type = JSValue;
		using reference = JSValue&;
		using difference_type = int;
		ConstIterator(pointer ptr) : m_ptr(ptr) {}
		const JSValue& operator*() const { return *m_ptr; }
		const JSValue* operator->() const { return m_ptr; }
		const JSValue* m_ptr;
		bool operator==(const Iterator& o) const;
		bool operator!=(const Iterator& o) const;
		Iterator& operator++();
		Iterator& operator++(int);
	};
	using const_iterator = ConstIterator;
	Iterator begin() {
		return Iterator(this);
	}
	ConstIterator begin() const {
		return ConstIterator(this);
	}
	Iterator end() {
		return Iterator(this+1);
	}
	ConstIterator end() const {
		return ConstIterator(this+1);
	}
	bool isMember(const char* index) const;
	bool isMember(const std::string& index) const;
	bool isObject() const;
	bool isString() const;
	std::string asString() const;
	int asInt() const;
	size_t size() const;
private:
	class copyVisitor : public JSValueVisitor {
		void visit(JSValueNull const & c) { copy = new JSValueNull(c); }
		void visit(JSValueInt const & c) { copy = new JSValueInt(c); }
		void visit(JSValueDouble const & c) { copy = new JSValueDouble(c); }
		void visit(JSValueBool const & c) { copy = new JSValueBool(c); }
		void visit(JSValueString const & c) { copy = new JSValueString(c); }
		void visit(JSValueArray const & c) { copy = new JSValueArray(c); }
		void visit(JSValueObject const & c) { copy = new JSValueObject(c); }
	public:
		JSValueBase* copy;
	};
	JSValueBase* pv;
};
#endif

#include "JSValue.h"
JSValue parse_json(const char*& p, const char* end);
std::istream& operator>>(std::istream& in, JSValue& root);
std::ostream& operator<<(std::ostream& out, const JSValue& it);
#if 0
JSValue* parse_json(const char* file);
void print_object(JSValue* obj);
#endif


#endif /* BIT_PARSE_JSON_DOM_H_ */
