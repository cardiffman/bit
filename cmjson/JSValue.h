#pragma once
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <cstring>

enum JSValueIndex { JS_NULL, JS_INT, JS_DOUBLE, JS_BOOL, JS_STRING, JS_ARRAY, JS_OBJECT };
class JSValue {
    struct Key {
        enum KeyType { KT_STRING, KT_INT} type;
        std::string s;
        int i;
        Key() : type(KT_INT),i(0) {}
        Key(const char* s) : type(KT_STRING), s(s){}
        Key(int i) : type(KT_INT),i(i){}
        Key(const std::string& t) : type(KT_STRING), s(t) {}
        bool operator<(const Key& o) const {
            if (type==KT_STRING)
                return s<o.s;
            return i<o.i;
        }
        bool operator==(const Key& o) const {
            if (type==KT_STRING)
                return s==o.s;
            return i==o.i;
        }
    };
    static char* duptext(const char* t);
public:
	JSValue(int v) : type(JS_INT) { guts.i = v; }
	JSValue(double v) : type(JS_DOUBLE) { guts.d = v; }
	JSValue(bool v) : type(JS_BOOL) { guts.b = v; }
    JSValue(const std::string& v);
    JSValue(const char* v);
    void dupPayload(const JSValue&);
	//JSValue(const std::map<std::string,JSValue>& v) { pv = new JSValueObject(v); }
	//JSValue(const std::vector<JSValue>& v) { pv = new JSValueArray(v); }
	JSValue(const JSValue& other);
    JSValueIndex type;
    using Object = std::map<Key,JSValue>;
    union {
        int i;
        bool b;
        double d;
        const char* s;
        Object* o;
    } guts;
	JSValue(JSValueIndex index=JSValueIndex::JS_NULL) : type(index) {
        if (index == JS_ARRAY || index == JS_OBJECT) {
            guts.o = new Object();
        }
    }
	int index() const { return type; }
	//void visit(JSValueVisitor& visitor) const { pv->acceptVisit(visitor); }
	JSValue& operator=(const JSValue& other) {
        JSValue(other).swap(*this);
		return *this;
	}
    void swapPayload(JSValue& other) {
        std::swap(guts, other.guts);
        std::swap(type, other.type);
    }
    void swap(JSValue& other) {
        swapPayload(other);
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
		Iterator(const JSValue::Object::iterator& ptr) : m_ptr(ptr) {}
		const JSValue& operator*() const { return m_ptr->second; }
		const JSValue* operator->() const { return &m_ptr->second; }
		bool operator==(const Iterator& o) const {
            return m_ptr == o.m_ptr;
        }
		bool operator!=(const Iterator& o) const {
            return m_ptr != o.m_ptr;
        }
		Iterator& operator++() {
            m_ptr++;
            return *this;
        }
		Iterator operator++(int) {
            Iterator t(*this);
            m_ptr++;
            return t;
        }
		JSValue::Object::iterator m_ptr;
	};
	using iterator = Iterator;
	struct ConstIterator {
		using pointer = const JSValue*;
		using value_type = JSValue;
		using reference = JSValue&;
		using difference_type = int;
		ConstIterator(const JSValue::Object::iterator& ptr) : m_ptr(ptr) {}
		const JSValue& operator*() const { return m_ptr->second; }
		const JSValue* operator->() const { return &m_ptr->second; }
		bool operator==(const ConstIterator& o) const {
            return m_ptr == o.m_ptr;
        }
		bool operator!=(const ConstIterator& o) const {
            return m_ptr != o.m_ptr;
        }
		ConstIterator& operator++() {
            m_ptr++;
            return *this;
        }
		ConstIterator operator++(int) {
            ConstIterator t(*this);
            m_ptr++;
            return t;
        }
        JSValue key() const {
            Key k = m_ptr->first;
            return k.type==Key::KT_STRING ? JSValue(k.s) : JSValue(k.i);
        }
		JSValue::Object::iterator m_ptr;
	};
	using const_iterator = ConstIterator;
	Iterator begin() {
		return Iterator(guts.o->begin());
	}
	ConstIterator begin() const {
		return ConstIterator(guts.o->begin());
	}
	Iterator end() {
		return Iterator(guts.o->end());
	}
	ConstIterator end() const {
		return ConstIterator(guts.o->end());
	}
	//bool isMember(const char* index) const;
	bool isMember(const std::string& index) const;
	bool isObject() const;
	bool isString() const;
	std::string asString() const;
	int asInt() const;
    bool asBool() const;
    double asDouble() const;
	size_t size() const;
};
JSValue parse_json(const char*& p, const char* end);
//std::istream& operator>>(std::istream& in, JSValue& root);
std::ostream& operator<<(std::ostream& out, const JSValue& it);
