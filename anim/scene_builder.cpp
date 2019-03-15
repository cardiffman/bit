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
#include "read_JPEG_file.h"
#include "read_png_file.h"
#include "parse_json_utils.h"
#include <algorithm>
#include "ft2build.h"
#include FT_FREETYPE_H

using std::cout;
using std::endl;

SceneBuilder::SceneBuilder() {
	container_id = 0;
	asset_id = 0;
}
void SceneBuilder::parse_text(Container& c, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	auto t = text.substr(ptokens->start, ptokens->end-ptokens->start);
	text_by_id[++asset_id] = t;
	Asset a;
	a.id = asset_id;
	na.push_back(a);
	c.asset_id = asset_id;
}
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
					continue;
				}
				if (member_name == "area")
				{
					parse_area(c.area, text, ptokens);
				}
				else if (member_name == "fill")
				{
					parse_color(c.color, text, ptokens);
				}
				else if (member_name == "asset")
				{
					parse_asset_label(c.asset_id, text, ptokens);
				}
				else if (member_name == "text")
				{
					parse_text(c, text, ptokens);
				}
				else if (member_name == "containers")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
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
			}
		}
		nc.push_back(c);
	}
}
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
						for (int i=0; i<elements; ++i)
						{
							container_read(text, ptokens, 0);
						}
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
	print_scene();
}
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
void SceneBuilder::parse_containers(const char* file, GraphicsEngine* engine)
{
	std::vector<jsmntok_t> tokens;
	std::string jstext;
	tokenize_json(file, tokens, jstext);
	cout << "js: chars provided " << jstext.size() << endl;
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
	FT_Library library;
	int fterror = FT_Init_FreeType(&library);
	for (auto t:text_by_id)
	{
		FT_Face face;
		if (fterror)
		{
			puts("freetype library");
			exit(1);
		}
		fterror = FT_New_Face(library, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 0, &face);
		if (fterror == FT_Err_Unknown_File_Format)
		{
			puts("Font not valid");
			exit(1);
		}
		else if (fterror)
		{
			printf("FreeType error %d\n", fterror);
			exit(1);
		}
		else if (face == NULL)
		{
			printf("FreeType face is null but no error?\n");
			exit(1);
		}
		fterror = FT_Set_Pixel_Sizes(face, 0, 32); //16 pixels high
		if (fterror)
		{
			cout << "Set pixel sizes failed. " << fterror << endl;
			continue;  /* ignore errors */
		}
		FT_GlyphSlot  slot = face->glyph;  /* a small shortcut */
		int           pen_x, pen_y, n;
		FT_UInt  glyph_index;

		pen_x = 0;
		pen_y = 32;
		struct Char { FT_UInt glyphIndex; int pen_x;int pen_y;Area a; uint8_t* bitmap; unsigned pitch; };
		std::vector<Char> chars;

		std::wstring tw = utf8tows(t.second.text);
		FT_UInt previous = 0;
		for (auto ch : tw)
		{
			/* retrieve glyph index from character code */
			glyph_index = FT_Get_Char_Index( face, ch );

			/* load glyph image into the slot (erase previous one) */
			fterror = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
			if ( fterror )
			{
				cout << "Load glyph failed" << endl;
				continue;  /* ignore errors */
			}


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
		auto gbuffer = engine->makeBuffer(RectSize(pen_x,height));
		uint8_t* buf_ptr; uint32_t buf_pitch;
		gbuffer->lock(buf_ptr, buf_pitch);

		/* locate the pen on the font's baseline, which in FT's
		 * case is 'descender' pixels above bottom.
		 */
		pen_x = 0;
		pen_y = height +(face->size->metrics.descender >>6);
		std::wcout << L"TEXT: " << tw << endl;
		cout << "METRICS specified size "<<font_size
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
		urls_by_id[t.first] = t.second;
		auto& x = na[t.first];
		x.image;
		na[t.first].image = gbuffer;
#if 0
		DEST = engine->makeBuffer(dims, mem, rowbytes);
		xcb_gcontext_t gcontext = xcb_generate_id(xcb_connection);
		uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND; // | XCB_GC_FONT;
		uint32_t value_list[3];
		value_list[0] = xcb_screen->black_pixel;
		value_list[1] = xcb_screen->white_pixel;
		//value_list[2] = font;
		xcb_void_cookie_t cookie_gc = xcb_create_gc_checked(xcb_connection,
				gcontext, window, mask, value_list);
		xcb_generic_error_t* error = xcb_request_check(xcb_connection, cookie_gc);
		xcb_void_cookie_t chk = xcb_put_image_checked(xcb_connection,
				XCB_IMAGE_FORMAT_Z_PIXMAP, window, gcontext, (uint16_t)1280/*width*/, (uint16_t)720/*height*/, 0,
				0, 0, XCBWindow::xcb_screen->root_depth, bytes, Mem);
		xcb_flush(xcb_connection);
		//cerr << "Image put, checking\n";
		/*xcb_generic_error_t**/ error = xcb_request_check(xcb_connection, chk);
		if (error)
		{
			cerr << "ERROR: can't put_image :"<<
					unsigned(error->error_code) << endl;
			xcb_disconnect(xcb_connection);
			exit(-1);
		}
		xcb_free_gc(xcb_connection, gcontext);
#endif
	}
	print_scene();
}


