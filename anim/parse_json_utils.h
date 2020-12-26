/*
 * parse_json_utils.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_PARSE_JSON_UTILS_H_
#define BIT_PARSE_JSON_UTILS_H_

#ifdef USE_JSMN
#include "jsmn.h"
#endif

#include <vector>
#include <string>

#ifdef USE_JSMN
bool tokenize_json(const char* file, std::vector<jsmntok_t>& tokens, std::string& jstext);
bool tokenize_json_text(std::vector<jsmntok_t>& tokens, std::string& jstext);

void dump_jstokens(const std::vector<jsmntok_t>& tokens, const std::string& jstext);
#endif



#endif /* BIT_PARSE_JSON_UTILS_H_ */
