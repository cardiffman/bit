/*
 * scene_builder.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */
#include "scene_builder.h"
#include "scene.h"
#include "engine.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <list>
#include <variant> // For error union.
#include "read_JPEG_file.h"
#include "read_png_file.h"
#include "parse_json_utils.h"
#include <algorithm>
#include <stdlib.h>
#include "ft2build.h"
#include FT_FREETYPE_H

using std::cout;
using std::endl;

#define LOG(A) cout << __FUNCTION__ << ' ' << A << ' ' << __FILE__ << ":" << __LINE__ << endl

#undef FTERRORS_H_
#define FT_ERROR_START_LIST     switch ( error_code ) {
#define FT_ERRORDEF( e, v, s )    case v: return s;
#define FT_ERROR_END_LIST       }
const char* ft_err_string(int error_code) {
#include FT_ERRORS_H
	return __FUNCTION__;
}

SceneBuilder::SceneBuilder() {
	container_id = 0;
	asset_id = 0;
}
std::wstring utf8tows(const std::string& input)
{
	std::wstring ws;
	auto pi = input.begin();
	while (pi != input.end())
	{
		unsigned b[4]; b[0]=b[1]=b[2]=b[3]=0;
		if ((uint8_t)*pi < 0xD0)
			ws.push_back(*pi++);
		else if (((uint8_t)*pi&0xe0) == 0xc0)
		{
			b[1] = *pi++ & 0x1F;
			b[0] = *pi++ & 0x3F;
			ws.push_back((b[1]<<6)|b[0]);
		}
		else if (((uint8_t)*pi&0xF0) == 0xE0)
		{
			b[2] = *pi++ & 0x0F;
			b[1] = *pi++ & 0x3F;
			b[0] = *pi++ & 0x3F;
			ws.push_back((b[2]<<12)|(b[1]<<6)|b[0]);
		}
		else //if ((uint8_t)*pi < 0xF0)
		{
			b[3] = *pi++ & 0x07;
			b[2] = *pi++ & 0x3F;
			b[1] = *pi++ & 0x3F;
			b[0] = *pi++ & 0x3F;
			ws.push_back((b[3]<<18)|(b[2]<<12)|(b[1]<<6)|b[0]);
		}
	}
	return ws;
}
#ifdef USE_JSMN
void SceneBuilder::parse_text(Container& c, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_STRING)
	{
		auto t = text.substr(ptokens->start, ptokens->end-ptokens->start);
		++ptokens;
		Text tt; tt.text = t;
		text_by_id[++asset_id] = tt;
		cout << "text: " << text_by_id[asset_id].text << " " << asset_id << " " << tt.text << endl;
		Asset a;
		a.id = asset_id;
		na.push_back(a);
		c.asset_id = asset_id;
	}
	else if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		Text tt;
		for (int i=0; i<members; ++i)
		{
			auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
			++ptokens;
			if (member_name == "string")
			{
				auto t = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				tt.text = t;
			}
			else if (member_name == "font")
			{
				auto f = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				tt.font = f;
			}
			else
			{
				cout << member_name << " value: " <<text.substr(ptokens->start, ptokens->end-ptokens->start) << endl;
				auto val = std::stod(text.substr(ptokens->start, ptokens->end-ptokens->start));
				tt.size = val;
				++ptokens;
			}
		}
		text_by_id[++asset_id] = tt;
		cout << "text: " << text_by_id[asset_id].text << " " << asset_id << " " << tt.text << endl;
		Asset a;
		a.id = asset_id;
		na.push_back(a);
		c.asset_id = asset_id;
	}
	else
	{
		cout << "parse_text " << ptokens->type << " is not handled here" << endl;
	}
}
#endif
#ifdef USE_JSMN
void SceneBuilder::parse_font(Container& c, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	auto t = text.substr(ptokens->start, ptokens->end-ptokens->start);
	++ptokens;
	font_by_id[++asset_id] = t;
	cout << "font: " << font_by_id[asset_id] << " " << asset_id << " " << t << endl;
}
#endif
#ifdef USE_JSMN
void SceneBuilder::parse_asset_label(unsigned& id, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	// File supplies a label. We define the ID here and map the label to the ID.
	if (ptokens->type == JSMN_STRING)
	{
		auto asset_label = text.substr(ptokens->start, ptokens->end-ptokens->start);
		++ptokens;
		Asset a;
		if (asset_label.find(".png")!=std::string::npos || asset_label.find(".jpeg")!=std::string::npos || asset_label.find(".jpg")!=std::string::npos)
		{
			++asset_id;
			//cout << "Unlabeled asset " << asset_label << " asset id " << asset_id << endl;
			id = asset_id;
			a.id = id;
			na.push_back(a);
			named_a[asset_label] = a;
			auto url = asset_label;
			urls_by_id[a.id] = url;
			return;
		}
		if (named_a.count(asset_label))
		{
			a = named_a[asset_label];
			id = a.id;
		}
		else
		{
			++asset_id;
			id = asset_id;
			a.id = asset_id;
			na.push_back(a);
			named_a[asset_label] = a;
		}
		//ids_of_named_assets[a.id] = asset_label;
	}
	else
	{
		throw "asset label has to be string";
	}
}
#endif
#ifdef USE_JSMN
void SceneBuilder::parse_color(unsigned& color, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_ARRAY)
	{
		int elements = ptokens->size;
		++ptokens;
		if (elements != 3 && elements != 4)
			throw "3 or 4 color components required";
		std::vector<unsigned> comps;
		for (int i=0; i<elements; ++i)
		{
			auto n = text.substr(ptokens->start, ptokens->end-ptokens->start);
			++ptokens;
			comps.push_back(std::stod(n));
		}
		if (elements == 3)
		{
			color = ((((0xFF00 | comps[0]) << 8) | comps[1]) << 8) | comps[2];
		}
		else
		{
			color = (((((comps[0] << 8) | comps[1]) << 8) | comps[2]) << 8) | comps[3];
		}
	}
	else
	{
		throw "color components must be array";
	}
}
#endif
#ifdef USE_JSMN
void SceneBuilder::parse_area(Area& area, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		for (int i=0; i<members; ++i)
		{
			auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
			++ptokens;
			auto val = std::stod(text.substr(ptokens->start, ptokens->end-ptokens->start));
			++ptokens;
			if (member_name == "x")
				area.x = val;
			else if (member_name == "y")
				area.y = val;
			else if (member_name == "width")
				area.width = val;
			else if (member_name == "height")
				area.height = val;
			else {
				throw "Bad member name in area";
			}
		}
	}
	else
	{
		throw "area components must be an object";
	}
}
#endif
#ifdef USE_JSMN
void SceneBuilder::container_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens, int parent)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		++container_id;
		Container c;
		c.id = container_id;
		c.parent_id = parent;
		c.asset_id = 0;
		c.color = 0;
		for (int i=0; i<members; ++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name == "label")
				{
					if (ptokens->type == JSMN_STRING)
					{
						auto label = text.substr(ptokens->start, ptokens->end-ptokens->start);
						named_container_ids[label] = c.id;
					}
					++ptokens;
				}
				else if (member_name == "area")
				{
					parse_area(c.area, text, ptokens);
					cout <<"Area for container " << c.id << " element " << i << endl;
				}
				else if (member_name == "fill")
				{
					parse_color(c.color, text, ptokens);
				}
				else if (member_name == "asset")
				{
					parse_asset_label(c.asset_id, text, ptokens);
					cout <<"Asset for container " << c.id << " element " << i << endl;
				}
				else if (member_name == "text")
				{
					cout << "text member so calling parse_text" << endl;
					parse_text(c, text, ptokens);
					cout <<"Text for container " << c.id << " element " << i << endl;
				}
#if 0
				else if (member_name == "font")
				{
					//parse_font(c, text, ptokens);
					//cout <<"Font for container " << c.id << " element " << i << endl;
				}
#endif
				else if (member_name == "containers")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						cout << "Container " << c.id << " Appears to have " << elements << " children" << endl;
						for (int i=0; i<elements; ++i)
						{
							container_read(text, ptokens, c.id);
						}
					}
					else
					{
						throw "containers member must be array";
					}
				}
				else
				{
					cout << "member " << member_name << endl;
					++ptokens;
				}
			}
			else
			{
				cout << "Member " << i << endl;
				++ptokens;
			}
		}
		nc.push_back(c);
		cout << "Containers: " << nc.size() << endl;
	}
}
#endif
#ifdef USE_JSMN
void SceneBuilder::asset_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		Asset a = {0}; a.id = 0; std::string url;
		for (int i=0; i<members; ++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name == "label")
				{
					auto label = text.substr(ptokens->start, ptokens->end-ptokens->start);
					++ptokens;
					if (!named_a.count(label))
					{
						a.id = ++asset_id;
						na.push_back(a);
						named_a[label] = a;
					}
					else
					{
						a = named_a[label];
					}
				}
				else if (member_name == "url")
				{
					url = text.substr(ptokens->start, ptokens->end-ptokens->start);
					++ptokens;
				}
			}
		}
		urls_by_id[a.id] = url;
	}
}
#endif
#ifdef USE_JSMN
void walk_unused_value(std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		for (int i=0; i<members; ++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				++ptokens;
				walk_unused_value(ptokens);
			}
			else
			{
				throw "Object requires member name";
			}
		}
	} else if (ptokens->type == JSMN_ARRAY) {
		int elements = ptokens->size;
		++ptokens;
		for (int i=0; i<elements; ++i)
		{
			walk_unused_value(ptokens);
		}
	} else if (ptokens->type == JSMN_STRING) {
		++ptokens;
	} else {
		++ptokens;
	}
}
#endif
#ifdef USE_JSMN
void SceneBuilder::user_input_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		std::string key;
		std::string animation;
		for (int member = 0; member<members; ++member)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name == "key")
				{
					if (ptokens->type == JSMN_STRING)
					{
						key = text.substr(ptokens->start, ptokens->end-ptokens->start);
						++ptokens;
					}
					else
					{
						cout << "key should be label";
						walk_unused_value(ptokens);
					}
				}
				else if (member_name == "animation")
				{
					if (ptokens->type == JSMN_STRING)
					{
						animation = text.substr(ptokens->start, ptokens->end-ptokens->start);
						++ptokens;
					}
					else
					{
						cout << "animation should be label";
						walk_unused_value(ptokens);
					}
				}
				else
				{
					cout << member_name << ": unknown user_input member" << endl;
					walk_unused_value(ptokens);
				}
			}
		}
		UserInput u; u.animation = animation; u.key = key;
		userInputs.push_back(u);
	}
}
#endif

#ifdef USE_JSMN
SceneBuilder::Parameter SceneBuilder::parse_parameter(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type != JSMN_OBJECT)
	{
		walk_unused_value(ptokens);
		return Parameter();
	}
	else {
		int members = ptokens->size;
		++ptokens;
		Parameter r;
		for (int member = 0; member < members; ++member)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				r.name = member_name;
				if (ptokens->type != JSMN_ARRAY)
				{
					walk_unused_value(ptokens);
				}
				else
				{
					int elements = ptokens->size;
					++ptokens;
					for (int element = 0; element<elements; ++element)
					{
						if (ptokens->type == JSMN_PRIMITIVE && text[ptokens->start]!='n' && text[ptokens->start]!='t' && text[ptokens->start]!='f')
						{
							r.values.push_back(std::stoi(text.substr(ptokens->start, ptokens->end-ptokens->start)));
							++ptokens;
						}
						else
						{
							walk_unused_value(ptokens);
						}
					}
				}
			}
		}
		return r;
	}
}
#endif

#ifdef USE_JSMN
void SceneBuilder::animation_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		std::string label;
		std::string container;
		int duration;
		std::vector<Parameter> parameters;
		for (int member = 0; member<members; ++member)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name == "label")
				{
					if (ptokens->type == JSMN_STRING)
					{
						label = text.substr(ptokens->start, ptokens->end-ptokens->start);
						++ptokens;
					}
					else
					{
						cout << "label should be label" << endl;
						walk_unused_value(ptokens);
					}
				}
				else if (member_name == "container")
				{
					if (ptokens->type == JSMN_STRING)
					{
						container = text.substr(ptokens->start, ptokens->end-ptokens->start);
						++ptokens;
					}
					else
					{
						cout << "container should be label" << endl;
						walk_unused_value(ptokens);
					}
				}
				else if (member_name == "duration")
				{
					if (ptokens->type == JSMN_PRIMITIVE && text[ptokens->start]!='n' && text[ptokens->start]!='t' && text[ptokens->start]!='f')
					{
						duration = std::stoi(text.substr(ptokens->start, ptokens->end-ptokens->start));
						++ptokens;
					}
					else
					{
						cout << "container should be number" << endl;
						walk_unused_value(ptokens);
					}
				}
				else if (member_name == "parameters")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						Parameter parameter = parse_parameter(text, ptokens);
						parameters.push_back(parameter);
					}
					else
					{
						cout << "container should be an array" << endl;
						walk_unused_value(ptokens);
					}
				}
				else
				{
					cout << member_name << ": unknown animation member" << endl;
					walk_unused_value(ptokens);
				}
			}
		}
		Animation a; a.container = container; a.msec = duration; a.parameters = parameters;
		animations.push_back(a);
	}
}
#endif

#ifdef USE_JSMN
void SceneBuilder::scene_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		for (int i=0;i<members;++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name=="containers")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						cout << "Top Level containers: " << elements << endl;
						for (int i=0; i<elements; ++i)
						{
							container_read(text, ptokens, 0);
						}
						cout << "Containers parsed "<< nc.size() << endl;
					}
					else
					{
						throw "Expect containers in array";
					}
				}
				else if (member_name == "assets")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						for (int i=0; i<elements; ++i)
						{
							asset_read(text, ptokens);
						}
					}
					else
					{
						throw "Expect assets in array";
					}
				}
				else if (member_name == "animations")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						for (int i=0; i<elements; ++i)
						{
							animation_read(text, ptokens);
						}
					}
					else
					{
						throw "Expect animations in array";
					}
				}
				else if (member_name == "user_input")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						for (int i=0; i<elements; ++i)
						{
							user_input_read(text, ptokens);
						}
					}
					else
					{
						throw "Expect animations in array";
					}
				}
				else {
					cout << member_name + ": Unknown member" << endl;
					walk_unused_value(ptokens);
				}
			}
			else
			{
				throw "Bad object format, need string for member name";
			}
		}
	}
}
#endif

void SceneBuilder::print_scene()
{
	for (auto c : nc)
	{
		cout << "{{" << c.area.x <<',' << c.area.y << ',' << c.area.width << ',' << c.area.height <<"}, "
				<< c.id << ',' << c.parent_id << ',' << c.asset_id << ',' << std::hex << c.color << std::dec << '}' << " // " << c.children<< endl;
	}
	for (auto a : na)
	{
		if (a.id != 0)
		cout << "{" << a.id << ',' << '{'<< a.image->dims.width << ','<< a.image->dims.height << '}'<< urls_by_id[a.id] << '}' << endl;
	}
}
bool SceneBuilder::by_id(const Container& a, const Container& b)
{
	return a.id < b.id;
}
void SceneBuilder::parse_containers_from_string(const char* text, GraphicsEngine* engine)
{
	#ifdef USE_JSMN
	std::vector<jsmntok_t> tokens;
	std::string jstext = text;
	tokenize_json_text(tokens, jstext);
	cout << "js: chars provided " << jstext.size() << endl;
	// refactoring opportunity
	//dump_jstokens(tokens, jstext);
	auto ptokens = tokens.begin();
	cout << "Scene" << endl;
	scene_read(&jstext[0], ptokens);
	cout << "Thats it" << endl;
	nc.insert(nc.begin(), Container());
	std::sort(nc.begin(), nc.end(), by_id);
	for (auto& g : nc) {
		g.children = 0;
	}
	for (auto& g : nc) {
		if (g.parent_id != ID_NULL)
			nc[g.parent_id].children++;
	}
	na.insert(na.begin(), Asset());
	//print_scene();
	for (auto u : urls_by_id)
	{
		if (u.second.find(".jpg")!=-1)
		{
			read_JPEG_file(u.second.c_str(), engine, na[u.first].image);
		} else if (u.second.find(".png")!=-1)
		{
			read_png_file(u.second.c_str(), engine, na[u.first].image);
		}
	}
	#endif
	print_scene();
}
#if 0
void draw_glyph(uint8_t* buf_ptr, uint32_t buf_pitch, uint8_t* bitmap, const Area& area, int pen_x, int pen_y)
{
	uint8_t* buf = buf_ptr+4*pen_x;
	for (int i=0; i<area.height; i++)
	{
		for (int j=0; j<area.width; ++j)
		{
			((uint32_t*)buf)[j] = bitmap[i*area.width+j]*0x01010101;
		}
		buf += buf_pitch;
	}
}
#endif

void draw_glyph(uint8_t* buf_ptr, uint32_t buf_pitch, int width, int height, FT_Bitmap* bitmap, int pen_x, int pen_y)
{
	Area draw;
	int res = clip_area_to_area(Area(0,0,width,height),Area(pen_x,pen_y,bitmap->width,bitmap->rows),draw);
	if (res == CLIP_OUT)
	{
		cout << "Glyph clipped out" << endl;
		return;
	}
	uint8_t* buf = buf_ptr+draw.y*buf_pitch+4*draw.x;
	for (int i=draw.y-pen_y; i<draw.y+draw.height-pen_y; i++)
	{
		for (int j=draw.x-pen_x; j<draw.x+draw.width-pen_x; ++j)
		{
			//((uint32_t*)buf)[j] = bitmap->buffer[i*bitmap->pitch+j]*0x01010101;
			//((uint32_t*)buf)[j] = bitmap->buffer[i*bitmap->pitch+j]<<24;//*0x01010101;
			//((uint32_t*)buf)[j] = (bitmap->buffer[i*bitmap->pitch+j]<<24)|0x00FFFFFF;
			buf[4*j+3] = bitmap->buffer[i*bitmap->pitch+j];
#if 0
			// GTK+cairo (cairo always) wants premultiplied alpha
			// Bit works with premultiplied or NPM.
			buf[4*j+2] = buf[4*j+1] = buf[4*j+0] = buf[4*j+3];
#elif 1
			// GTK+cairo (cairo always) wants premultiplied alpha
			// Bit works with premultiplied or NPM.
			// Using integers which is not sensitive to endianness.
			((uint32_t*)buf)[j] = bitmap->buffer[i*bitmap->pitch+j]*0x01010101;
#else
			// NPM, fails with cairo
			buf[4*j+2] = buf[4*j+1] = buf[4*j+0] = 0xFF;
#endif
		}
		buf += buf_pitch;
	}
}

struct FontUsage
{
	FT_Library* library;
	std::string font;
	FT_Face face;
	int size;
};
std::list<FontUsage> fonts;
FT_Library library;
typedef std::variant<FontUsage,int> FontExpected;
FontExpected make_font_expected(int error) {
	return FontExpected(error);
}

FontExpected
make_font(const std::string& fontfile, int size)
{
	auto finalpath = fontfile;
	if (!finalpath.size())
		finalpath = "/usr/share/fonts-droid-fallback/truetype/DroidSansFallback.ttf";
	for (auto& font: fonts)
	{
		if (font.font == finalpath && font.size == size)
			return std::variant<FontUsage,int>(font);
	}
	FontUsage font;
	font.font = finalpath;
	font.library = &library;
	int fterror;
	fterror = FT_New_Face(library, finalpath.c_str(), 0, &font.face);
	if (fterror == FT_Err_Unknown_File_Format)
	{
		LOG("Font not valid" << ft_err_string(fterror));
		return std::variant<FontUsage,int>(-1);
	}
	else if (fterror)
	{
		LOG("FreeType font " << finalpath << " error " << ft_err_string(fterror));
		return std::variant<FontUsage,int>(-1);
	}
	else if (font.face == NULL)
	{
		LOG("FreeType face is null but no error?");
		return std::variant<FontUsage,int>(-1);
	}

	font.size = size ? size : 16;
	fterror = FT_Set_Pixel_Sizes(font.face, 0, font.size); //16 pixels high
	if (fterror)
	{
		LOG("Set pixel sizes failed. " << ft_err_string(fterror));
		//continue; /* ignore errors */
	}
	fonts.push_back(font);
	return fonts.back();
}
void SceneBuilder::prepare_text(GraphicsEngine* engine)
{
	cout << "Text assets:" << text_by_id.size() << endl;
	int fterror = FT_Init_FreeType(&library);
	if (fterror)
	{
		LOG("freetype library " << ft_err_string(fterror));
		exit(1);
	}
	for (auto t : text_by_id)
	{
		//FT_Face face;
		auto exp_font = make_font(t.second.font, t.second.size);
		if (exp_font.index() != 0)
			exit(1);
		FontUsage font = std::get<FontUsage>(exp_font);
		FT_Face& face = font.face;
		FT_GlyphSlot slot = face->glyph; /* a small shortcut */
		int pen_x, pen_y, n;
		FT_UInt glyph_index;
		int height = face->size->metrics.height >> 6;
		pen_x = 0;
		pen_y = height +(face->size->metrics.descender >>6);
		std::wstring tw = utf8tows(t.second.text);
		FT_UInt previous = 0;
		for (auto ch : tw)
		{
			/* retrieve glyph index from character code */
			glyph_index = FT_Get_Char_Index( face, ch );

			/* load glyph image into the slot (erase previous one) */
			fterror = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
			if ( fterror )
				continue;  /* ignore errors */


			if (previous)
			{
				FT_Vector kerning;
				FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_UNFITTED, &kerning);
				pen_x += kerning.x >> 6;
			}
			previous = glyph_index;
			/* convert to an anti-aliased bitmap */
			fterror = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
			if ( fterror )
				continue;

			/* increment pen position */
			pen_x += slot->advance.x >> 6;
			pen_y += slot->advance.y >> 6; /* not useful for now */
			//if (slot->bitmap.rows > height) height = slot->bitmap.rows;
		}

		height = face->size->metrics.height >> 6;
		int width = pen_x;
		auto gbuffer = engine->makeBuffer(RectSize(pen_x,height));
		engine->fill(gbuffer, Area(0,0,width,height), 0);
		uint8_t* buf_ptr; uint32_t buf_pitch;
		gbuffer->lock(buf_ptr, buf_pitch);

		/* locate the pen on the font's baseline, which in FT's
		 * case is 'descender' pixels above bottom.
		 */
		pen_x = 0;
		pen_y = height +(face->size->metrics.descender >>6);
		std::wcout << L"TEXT: " << tw << endl;
		cout << "METRICS specified size "<<font.size
				<<" ascender "<<(face->size->metrics.ascender>>6)
				<<" descender "<<(face->size->metrics.descender>>6)
				<<" height "<<(face->size->metrics.height>>6)
				<<" max_advance "<<(face->size->metrics.max_advance>>6)
				<< endl;
		previous = 0;
		for (auto ch : tw)
		{
			/* retrieve glyph index from character code */
			glyph_index = FT_Get_Char_Index( face, ch );

			/* load glyph image into the slot (erase previous one) */
			fterror = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
			if ( fterror )
				continue;  /* ignore errors */

			/* convert to an anti-aliased bitmap */
			fterror = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL );
			if ( fterror )
				continue;

			if (previous)
			{
				FT_Vector kerning;
				FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_UNFITTED, &kerning);
				pen_x += kerning.x >> 6;
			}
			previous = glyph_index;
			/* now, draw to our target surface */
			draw_glyph(buf_ptr, buf_pitch, width, height, &slot->bitmap, pen_x+slot->bitmap_left, pen_y-slot->bitmap_top);

			/* increment pen position */
			pen_x += slot->advance.x >> 6;
			pen_y += slot->advance.y >> 6; /* not useful for now */
		}
		gbuffer->unlock();
		urls_by_id[t.first] = t.second.text;
		na[t.first].image = gbuffer;
	}
}

void SceneBuilder::parse_asset_label(Json::Value c, unsigned& asset_ref)
{
	auto asset_label = c["asset"].asString();
	Asset a;
	if (asset_label.find(".png")!=std::string::npos || asset_label.find(".jpeg")!=std::string::npos || asset_label.find(".jpg")!=std::string::npos)
	{
		++asset_id;
		//cout << "Unlabeled asset " << asset_label << " asset id " << asset_id << endl;
		a.id = asset_id;
		na.push_back(a);
		named_a[asset_label] = a;
		auto url = asset_label;
		urls_by_id[a.id] = url;
		return;
	}
	if (named_a.count(asset_label))
	{
		a = named_a[asset_label];
	}
	else
	{
		++asset_id;
		a.id = asset_id;
		na.push_back(a);
		named_a[asset_label] = a;
	}
	asset_ref = a.id;
}
void SceneBuilder::parse_containers(const char* file, GraphicsEngine* engine)
{
#ifdef USE_JSMN
	std::vector<jsmntok_t> tokens;
	std::string jstext;
	if (!tokenize_json(file, tokens, jstext))
	{
		throw std::runtime_error("tokenizing failed");
	}
	cout << "js: chars provided " << jstext.size() << endl;
	//dump_jstokens(tokens, jstext);
	auto ptokens = tokens.begin();
	cout << "Scene" << endl;
	scene_read(&jstext[0], ptokens);
	cout << "Thats it" << endl;
#endif
#ifdef USE_JSONCPP
	cout << __FUNCTION__ << endl;

	Json::Value root;
	std::ifstream ifs;
	ifs.open(file);

	Json::CharReaderBuilder builder;
	builder["collectComments"] = true;
	JSONCPP_STRING errs;
	if (!parseFromStream(builder, ifs, &root, &errs)) {
		std::cout << errs << std::endl;
		exit(EXIT_FAILURE);
	}
	for (auto a : root["assets"]) {
		Asset asset = {0}; asset.id = 0; std::string url;
		if (a.isMember("label")) {
			auto label = a["label"].asString();
			if (!named_a.count(label))
			{
				asset.id = ++asset_id;
				na.push_back(asset);
				named_a[label] = asset;
			}
			else
			{
				asset = named_a[label];
			}
		}
		if (a.isMember("url")) {
			url = a["url"].asString();
		}
		urls_by_id[asset.id] = url;
	}
	std::vector<std::pair<Json::Value,int>> work;
	for (auto c : root["containers"]) {
		work.push_back(std::make_pair(c,0));
	}
	int id = 1;
	//int asset_id = 0;
	std::map<std::string,int> ids;
	//int parent = 0;
	while (work.size()) {
		//auto pc = work.back(); work.pop_back();
		auto pc = work.front(); work.erase(work.begin());//work.pop_front();
		Json::Value c = pc.first;
		int parent = pc.second;
		Container out;
		out.asset_id = 0;
		out.id = id++;
		out.parent_id = pc.second;
		if (c.isMember("label")) {
			 ids[c["label"].asString()] = out.id;
		}
		if (c.isMember("area")) {
			out.area.x = c["area"]["x"].asInt(); out.area.y = c["area"]["y"].asInt(); out.area.width= c["area"]["width"].asInt(); out.area.height =c["area"]["height"].asInt();
		}
		if (c.isMember("fill")) {
			auto comps = c["fill"];
			out.color = ((comps[0].asInt()*256+comps[1].asInt())*256+comps[2].asInt())*256+comps[3].asInt();
		} else {
			out.color = 0;//0xFF0000FF;
		}
		if (c.isMember("text")) {
			/*
		Text tt; tt.text = t;
		text_by_id[++asset_id] = tt;
		cout << "text: " << text_by_id[asset_id].text << " " << asset_id << " " << tt.text << endl;
		Asset a;
		a.id = asset_id;
		na.push_back(a);
		c.asset_id = asset_id;
			*/
			Text tt;
			auto jt = c["text"];
			if (jt.isString()) {
				cout << "Incoming string asset " << jt << endl;
				tt.text = jt.asString();
			} else if (jt.isObject()) {
				cout << "Incoming text asset " << jt << endl;
				tt.font = jt["font"].asString();
				tt.text = jt["string"].asString();
				tt.size = jt["size"].asInt();
			}
			cout << "Resulting text asset " << tt.font << ' ' << tt.size << ' ' << tt.text << endl;
			text_by_id[++asset_id] = tt;
			Asset ta;
			ta.id = out.asset_id;
			ta.image = nullptr;
			na.push_back(ta);
			out.asset_id = asset_id;
		}
		if (c.isMember("asset")) {
			parse_asset_label(c, out.asset_id);
			cout <<"Asset for container " << out.id << " asset " << out.asset_id << endl;
		}
		//out.asset_id = 0;
		out.children = 0;
		
		nc.push_back(out);
		if (c.isMember("containers")) {
			for (auto cc : c["containers"]) {
				work.push_back(std::make_pair(cc,id-1));
			}
		}
	}
	std::sort(nc.begin(), nc.end(), by_id);
	//std::cout << root << std::endl;
#endif
	nc.insert(nc.begin(), Container());
	std::sort(nc.begin(), nc.end(), by_id);
	for (auto& g : nc) {
		g.children = 0;
	}
	for (auto& g : nc) {
		if (g.parent_id != ID_NULL)
			nc[g.parent_id].children++;
	}
	na.insert(na.begin(), Asset());
	//print_scene();
	for (auto u : urls_by_id)
	{
		if (u.second.find(".jpg")!=-1)
		{
			read_JPEG_file(u.second.c_str(), engine, na[u.first].image);
		} else if (u.second.find(".png")!=-1)
		{
			read_png_file(u.second.c_str(), engine, na[u.first].image);
		}
	}
	prepare_text(engine);
	print_scene();
}
