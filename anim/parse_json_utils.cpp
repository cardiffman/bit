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
#include <string.h> // strerror

using std::cout;
using std::endl;

bool tokenize_json(const char* file, std::vector<jsmntok_t>& tokens, std::string& jstext)
{
	jsmn_parser jp;
	jsmn_init(&jp);
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		cout << "Opening " << file << strerror(errno) << endl;
		return false;
	}
	int jslength = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	jstext.resize(jslength);
	read(fd, &jstext[0], jslength);
	return tokenize_json_text(tokens, jstext);
}
bool tokenize_json_text(std::vector<jsmntok_t>& tokens, std::string& jstext)
{
	jsmn_parser jp;
	jsmn_init(&jp);
	int tokes = jsmn_parse(&jp, jstext.data(), jstext.length(), NULL, 0);
	if (tokes < 1)
	{
		switch (tokes)
		{
		/* Not enough tokens were provided */
		case JSMN_ERROR_NOMEM: throw "JSMN_ERROR_NOMEM"; return false;
		/* Invalid character inside JSON string */
		case JSMN_ERROR_INVAL: throw "JSMN_ERROR_INVAL"; return false;
		/* The string is not a full JSON packet, more bytes expected */
		case JSMN_ERROR_PART: throw "JSMN_ERROR_PART"; return false;
		case 0: throw "Not even one token"; return false;
		}
		return false;
	}
	tokens.resize(tokes);
	jsmn_init(&jp);
	jsmn_parse(&jp, jstext.data(), jstext.length(), tokens.data(), tokes);
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



