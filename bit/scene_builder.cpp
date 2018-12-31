/*
 * scene_builder.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */
#include "scene_builder.h"
#include "scene.h"
#include <iostream>
#include "read_JPEG_file.h"
#include "read_png_file.h"
#include "parse_json_utils.h"
#include <algorithm>
using std::cout;
using std::endl;

SceneBuilder::SceneBuilder() {
	container_id = 0;
	asset_id = 0;
}
void SceneBuilder::parse_asset_label(unsigned& id, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	++asset_id;
	// File supplies a label. We define the ID here and map the label to the ID.
	id = asset_id;
	if (ptokens->type == JSMN_STRING)
	{
		auto asset_label = text.substr(ptokens->start, ptokens->end-ptokens->start);
		++ptokens;
		Asset a;
		a.id = asset_id;
		na.push_back(a);
		named_a[asset_label] = a;
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
				<< c.id << ',' << c.parent_id << ',' << c.asset_id << ',' << c.color << '}' << " // " << c.children<< endl;
	}
	for (auto a : na)
	{
		cout << "{" << a.id << ',' << '{'<< a.image.dims.width << ','<< a.image.dims.height << '}'<< urls_by_id[a.id] << '}' << endl;
	}
}
bool SceneBuilder::by_id(const Container& a, const Container& b)
{
	return a.id < b.id;
}
void SceneBuilder::parse_containers(const char* file)
{
	std::vector<jsmntok_t> tokens;
	std::string jstext;
	tokenize_json(file, tokens, jstext);
	cout << "js: chars provided " << jstext.size() << endl;
	dump_jstokens(tokens, jstext);
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
	print_scene();
	for (auto u : urls_by_id)
	{
		if (u.second.find(".jpg")!=-1)
		{
			read_JPEG_file(u.second.c_str(), &na[u.first].image);
		} else if (u.second.find(".png")!=-1)
		{
			read_png_file(u.second.c_str(), &na[u.first].image);
		}
	}
	print_scene();
}


