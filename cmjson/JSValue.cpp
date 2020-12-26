#include "JSValue.h"
#include <iostream>
using namespace std;

char* JSValue::duptext(const char* t) {
    auto len = *(int*)t;
    char* s = new char[len+sizeof(int)];
    std::copy(t+sizeof(int), t+sizeof(int)+len, s+sizeof(int));
    //copy(t+sizeof(int),t+sizeof(int)+len,t+sizeof(int));
    *(int*)s = len;
    return (char*)s;
}

char* duptext(const char* t, size_t len) {
    char* s = new char[len+sizeof(int)];
    std::copy(t, t+len, s+sizeof(int));
    //copy(t,t+len,t+sizeof(int));
    *(int*)s = len;
    return (char*)s;
}

char* duptext(const char* t) {
    auto len = strlen(t);
    char* s = new char[len+sizeof(int)];
    std::copy(t, t+len, s+sizeof(int));
    *(int*)s = len;
    return (char*)s;
}

JSValue::JSValue(const std::string& v) : type(JS_STRING) {
    guts.s = ::duptext(v.data(), v.length());
}
JSValue::JSValue(const char* v) : type(JS_STRING) {
    guts.s = ::duptext(v);
}

JSValue::JSValue(const JSValue& other) {
  //dupPayload(other);
  //dupMeta(other);

    type = other.type;
    switch (index()) {
    case JS_STRING:
        guts.s = duptext(other.guts.s);
        break;
    default:
        guts = other.guts;
        break;
    }
}
JSValue JSValue::operator[](int index) const {
    return (*guts.o)[index];
}
JSValue JSValue::operator[](const char* index) const {
    return (*guts.o)[index];
}
JSValue JSValue::operator[](const std::string& index) const {
    return (*guts.o)[index];
}
JSValue& JSValue::operator[](int index) {
    std::cout << "reference access at index " << index << std::endl;
    Key k(index);
    cout << "Value type is " << type << " instead of " << JS_ARRAY << " or " << JS_OBJECT << endl;
    if (type == JS_NULL)
        *this = JSValue(JS_ARRAY);
    cout << "Value type is " << type << " instead of " << JS_ARRAY << " or " << JS_OBJECT << endl;
    std::cout << "Made index, map at " << guts.o << " map size " << guts.o->size() << endl;
    auto p = guts.o->lower_bound(k);
    cout << "Search for lower bound " << endl;
    if (p != guts.o->end() && k == p->first)
        return p->second;
    cout << "Adding entry to map " << endl;
    Object::value_type v(k,JSValue());
    p = guts.o->insert(p,v);
    return p->second;
}
JSValue& JSValue::operator[](const char* index) {
    return (*guts.o)[index];
}
JSValue& JSValue::operator[](const std::string& index) {
    std::cout << "reference access at index " << index << std::endl;
    Key k(index);
    cout << "Value type is " << type << " instead of " << JS_ARRAY << " or " << JS_OBJECT << endl;
    if (type == JS_NULL)
        *this = JSValue(JS_OBJECT);
    cout << "Value type is " << type << " instead of " << JS_ARRAY << " or " << JS_OBJECT << endl;
    std::cout << "Made index, map at " << guts.o << " map size " << guts.o->size() << endl;
    auto p = guts.o->lower_bound(k);
    cout << "Search for lower bound " << endl;
    if (p != guts.o->end() && k == p->first) {
        cout << "Lower bound is not end and key " << p->first.s << " matches " << index << endl;
        return p->second;
    }
    cout << "Adding entry to map " << endl;
    Object::value_type v(k,JSValue());
    p = guts.o->insert(p,v);
    return p->second;
}
int JSValue::asInt() const {
    return guts.i;
}
std::string JSValue::asString() const {
    return std::string(guts.s+sizeof(int), *(int*)guts.s);
}
bool JSValue::isMember(const std::string& s) const {
    switch (index()) {
    case JS_OBJECT:
        return guts.o->find(s) != guts.o->end();
    default:
        return false;
    }
}
size_t JSValue::size() const {
    switch (index()) {
    case JS_ARRAY:
    case JS_OBJECT:
        return guts.o->size();
    default:
        return 0;
    }
}
bool JSValue::isObject() const {
    return index()==JS_OBJECT;
}
bool JSValue::isString() const {
    return index()==JS_STRING;
}
bool JSValue::asBool() const {
    return guts.b;
}
double JSValue::asDouble() const {
    return guts.d;
}
std::ostream& operator<<(std::ostream& out, const JSValue& j) {
    int p;
    #if 0
    if (j.index()==JS_BOOL) {
        bool b = j.asBool();
        const char* c = b?"true":"false";
        //out<<c;
        out.write(c,strlen(c));
        return out;
        //return out<<(j.asBool()?"true":"false");
    }
    #endif
    switch (j.index()) {
    case JS_NULL: out<<"null"; return out;
    case JS_BOOL: return out<<(j.asBool()?"true":"false");
    case JS_DOUBLE: return out<<j.asDouble();
    case JS_INT: 
    #if 0
    p=j.asInt(); 
    out << "&&&"; 
    out<<p; 
    out << "&&&"; 
    #endif
        return out<<j.asInt();
    break;
    case JS_STRING: return out<<'"'<<j.asString()<<'"';
    case JS_ARRAY: {
        out << '[';
        bool comma = false;
        for (auto e : j) {
            if (comma)
                out << ',';
            out << e;
            comma = true;
        }
        return out << ']';
        }
    case JS_OBJECT: {
        out << '{';
        bool comma = false;
        for (auto pe = j.begin(); pe != j.end(); ++pe) {
            if (comma)
                out << ',';
            out << pe.key();
            out << ':';
            out << *pe;
            comma = true;
        }
        return out << '}';
    }
    }
    return out;
}