#pragma once
#include <string>
#include <map>

namespace html {

class Font;
class Text;
class Img;

class ElementVisitor {
    public:
    virtual ~ElementVisitor() {}
    virtual void visitFont(const Font&) = 0;
    virtual void visitText(const Text&) = 0;
    virtual void visitImg(const Img&) = 0;
};
class Element {
public:
    Element(const std::wstring& name, bool endTag=false) : name(name), endTag(endTag) {}
    Element(const std::wstring& name, const std::map<std::wstring,std::wstring>& atr) : name(name), endTag(false), attributes(atr) {}
    std::wstring name;
    bool endTag;
    std::map<std::wstring,std::wstring> attributes;
    virtual void visit(ElementVisitor& visitor) const = 0;
};
class Font : public Element {
    public:
    Font(const std::map<std::wstring,std::wstring>& atr) : Element(L"font",atr) {}
    Font(bool endTag) : Element(L"font",endTag) {}
    void visit(ElementVisitor& visitor) const { visitor.visitFont(*this); }
};
class Text : public Element {
    public:
    Text(const std::wstring& text) : Element(L"#text"), text(text) {}
    void visit(ElementVisitor& visitor) const { visitor.visitText(*this); }
    std::wstring text;
};

class Img : public Element {
    public:
    Img(const std::map<std::wstring,std::wstring>& atr) : Element(L"img",atr) {}
    Img(bool endTag) : Element(L"img",endTag) {}
    void visit(ElementVisitor& visitor) const { visitor.visitImg(*this); }
    private:
};

class InputReader {
public:
    virtual wchar_t peek() const = 0;
    virtual wchar_t read() = 0;
    virtual bool eof() const = 0;
};
bool parseHTML(InputReader* reader, ElementVisitor& visitor);
}