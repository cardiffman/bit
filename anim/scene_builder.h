/*
 * scene_builder.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_SCENE_BUILDER_H_
#define BIT_SCENE_BUILDER_H_

#include <vector>
#include <map>

#include "jsmn.h"

#include "scene.h"

struct SceneBuilder
{
	std::vector<Container> nc;
	std::vector<Asset> na;
	std::map<std::string,Asset> named_a;
	std::map<std::string,int> named_container_ids;
	std::map<unsigned,std::string> urls_by_id;
	struct Parameter {
		std::string name;
		std::vector<int> values;
	};
	struct Animation {
		std::string container; int msec; std::vector<Parameter> parameters;
	};
	struct UserInput {
		std::string key; std::string animation;
	};
	std::vector<Animation> animations;
	std::vector<UserInput> userInputs;
	int container_id;
	int asset_id;
	SceneBuilder();
	void parse_containers(const char* file);
private:
	void parse_asset_label(unsigned& id, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	void parse_color(unsigned& color, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	void parse_area(Area& area, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	void scene_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	void container_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens, int parent);
	void asset_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	void print_scene();
	static bool by_id(const Container& a, const Container& b);
	void user_input_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	void animation_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
	Parameter parse_parameter(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens);
};

#endif /* BIT_SCENE_BUILDER_H_ */