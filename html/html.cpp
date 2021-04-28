#include "html.h"
#include <fstream>
#include <iostream>

using namespace std;
namespace html {

/*
The rules:
All whitespace in a #text counts.
attribute and tag names must be lowercase.
Only FONT and IMG tags matter. The rest are dropped.
The IMG element is represented by an open tag, the close tag will be ignored.
If an element is written as an XML void element that will be ignored since there's no
purpose for that.
*/
bool parseHTML(InputReader* reader, ElementVisitor& visitor)
{
    wstring text;
    while (!reader->eof()) {
        auto ch = reader->read();
        //wcout << ch << '.' << endl;
        while (!reader->eof() && ch != '<') {
            text.push_back(ch);
            ch = reader->read();
        }
        if (text.size())
        {
            Text t(text);
            t.visit(visitor);
            text.clear();
        }
        if (reader->eof())
            break;
        /// Read up to '<' now read a tag of some kind.
        ch = reader->read(); // now read next char.
        bool endTag = false;
        if (ch=='!') {
            while (ch != '>' && !reader->eof())
                ch = reader->read();
            continue;
        }
        if (ch=='/') {
            endTag = true;
            ch = reader->read();
        }
        wstring name;
        while (isalpha(ch))
        {
            name.push_back(ch);
            ch = reader->read();
        }
        while (!reader->eof() && (ch==' ' || ch=='\t'))
            ch = reader->read();
        map<wstring,wstring> atrs;
        wstring atr; wstring value;
        while (ch != '>' && ch != '/')
        {
            atr.clear();
            value.clear();
            while (isalpha(ch) || ch=='-') // a dash is expected in attribute names.
            {
                atr.push_back(ch);
                ch = reader->read();
            }
            while (!reader->eof() && (ch==' ' || ch=='\t'))
                ch = reader->read();
            if (ch=='=')
            {
                /*
                Reading attribulte value here
                equals sign, whitespace, quoted-value or
                equals sign, whitespace, non-white-non-gt-sequence
                */
                ch = reader->read();
                while (!reader->eof() && (ch==' ' || ch=='\t'))
                    ch = reader->read();
                while (!reader->eof())
                {
                    if (ch=='"')
                    {
                        ch = reader->read();
                        while (ch != '"')
                        {
                            value.push_back(ch);
                            ch = reader->read();
                        }
                        ch = reader->read();
                        break;
                    }
                    else
                    {
                        value.push_back(ch);
                        ch = reader->read();
                        while (ch != ' ' && ch != '\t' && ch != '/' && ch != '>')
                        {
                            value.push_back(ch);
                            ch = reader->read();
                        }
                        if (ch=='/')
                            ch = reader->read();
                        break;
                    }
                }
            }
            if (atr.size())
                atrs[atr]=value;
        }
        if (ch == '/')
            ch = reader->read();
        if (name==L"font")
        {
            if (endTag)
            {
                Font font(true);
                visitor.visitFont(font);
            }
            else
            {
                Font font(atrs);
                visitor.visitFont(font);
            }
        }
        else if (name==L"img")
        {
            if (!endTag)
            {
                Img img(atrs);
                visitor.visitImg(img);
            }
        }
    }
    return true;
}
}

using namespace html;
struct IfstreamReader : public InputReader
{
    IfstreamReader(std::wifstream& file) : file(file) {}
    std::wifstream& file;
    bool eof() const { return file.eof(); }
    wchar_t read() { return file.get(); }
    wchar_t peek() const { return file.peek(); }
};

struct StringReader : public InputReader
{
    StringReader(std::wstring const& file) : file(file) { pc=file.begin(); }
    std::wstring const& file;
    std::wstring::const_iterator pc;
    bool eof() const { return pc!=file.end(); }
    wchar_t read() { return pc==file.end()?(wchar_t)-1:*pc++; }
    wchar_t peek() const { return pc==file.end()?(wchar_t)-1:*pc; }
};

struct Dumper : public ElementVisitor
{
    void visitText(const Text& t) { wcout << t.name << '[' << t.text << ']' << endl; }
    void visitFont(const Font& f) { wcout << f.name; if (f.endTag) wcout << '/'; else { wcout << '['; dumpAtrs(f.attributes); wcout << ']';} wcout << endl; }
    void visitImg(const Img& f) { wcout << f.name << '['; dumpAtrs(f.attributes); wcout << ']' << endl; }
    void dumpAtrs(const std::map<std::wstring,std::wstring>& avs) {
        auto pav = avs.begin();
        if (pav != avs.end()) {
            wcout << pav->first << '=' << pav->second;
            ++pav;
            while (pav != avs.end()) {
                wcout << ' ' << pav->first << '=' << pav->second;
                ++pav;
            }
        }
    }
};
int main(int argc, char** argv)
{
    auto fr = wifstream(argv[1],ios::in);
    IfstreamReader reader(fr);
    Dumper dumper;
    parseHTML(&reader, dumper);
    StringReader dummy(L"");
    return 0;
}