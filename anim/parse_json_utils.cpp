/*
 * parse_json_utils.cpp
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#include "parse_json_utils.h"

#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

using std::cout;
using std::endl;

bool tokenize_json(const char* file, std::vector<jsmntok_t>& tokens, std::string& jstext)
{
	jsmn_parser jp;
	jsmn_init(&jp);
	int fd = open(file, O_RDONLY);
	int jslength = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	jstext.resize(jslength);
	read(fd, &jstext[0], jslength);
	int tokes = jsmn_parse(&jp, jstext.data(), jslength, NULL, 0);
	tokens.resize(tokes);
	jsmn_init(&jp);
	jsmn_parse(&jp, jstext.data(), jslength, tokens.data(), tokes);
	close(fd);
	return true;
}
void dump_jstokens(const std::vector<jsmntok_t>& tokens, const std::string& jstext)
{
	for (int i=0; i<tokens.size(); ++i)
	{
		const char* types[] = {
				"JSMN_UNDEFINED",
				"JSMN_OBJECT",
				"JSMN_ARRAY",
				"JSMN_STRING",
				"JSMN_PRIMITIVE"
		};
		cout << i << ": type=" << types[tokens[i].type] << " start: " << tokens[i].start << " end: " << tokens[i].end
			<< " size: " << tokens[i].size;
		switch (tokens[i].type)
		{
		case JSMN_STRING:
			if (tokens[i].end - tokens[i].start < 20 || (tokens[i].end - tokens[i].start < 100))
			{
				cout << " \"" << jstext.substr(tokens[i].start, tokens[i].end-tokens[i].start) << '"';
			}
			cout << endl;
			break;
		case JSMN_PRIMITIVE:
			cout << " " << jstext.substr(tokens[i].start, tokens[i].end-tokens[i].start);
			cout << endl;
			break;
		case JSMN_ARRAY:
			cout << endl;
			break;
		case JSMN_OBJECT:
			cout << endl;
			break;
		default:
			cout << endl;
			break;
		}
	}
}



