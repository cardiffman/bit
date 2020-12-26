#include "JSValue.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

#if 0
class print_visitor : public JSValueVisitor {
public:
    print_visitor(std::ostream& str) : str(str) {}
    void visit(JSValueNull const& v) { str << v.value();}
    void visit(JSValueInt const& v) { str << v.value();}
    void visit(JSValueDouble const& v) { str << v.value();}
    void visit(JSValueBool const& v) { str << (v.value()?"true":"false");}
    void visit(JSValueString const& v) { str << '"' << v.value() << '"';}
    void visit(JSValueArray const& v) { 
        str << "[";
        bool comma=false;
        for (vector<JSValue>::const_iterator p = v.value().begin(); p != v.value().end(); ++p) {
            if (comma)
                str<<',';
            p->visit(*this);
            comma = true;
        }
        str << "]";
    }
    void visit(JSValueObject const& v) {
        str << "{";
        bool comma=false;
        for (map<string,JSValue>::const_iterator p=v.value().begin(); p!=v.value().end(); ++p) {
            if (comma)
                str<<',';
            str<<'"'<<p->first<<"\":"; p->second.visit(*this);
            comma = true;
        }
        str << "}";
    }
    std::ostream& str;
};
#endif
int main(int argc, const char** argv) {
    try {
        #if 0
        print_visitor pv(cout);
        #endif
        if (argc==3 && strcmp(argv[1],"--cmd")==0) {
            JSValue js = parse_json(argv[2],argv[2]+strlen(argv[2]));
            #if 0
            js.visit(pv);
            #endif
            cout << js << endl;
        } else if (argc==2) {
            std::ifstream f(argv[1],std::ios_base::binary);
            std::stringstream ss;
            ss << f.rdbuf();
            std::string s = ss.str();
            const char* st = s.data();
            const char* en = st + s.size();
            JSValue js = parse_json(st, en);
            #if 0
            js.visit(pv);
            #endif
            cout << js << endl;
        }
    } catch (const char* c) {
        cout << "Caught " << c << endl;
    }
}
