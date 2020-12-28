/*
 * parse_json_dom.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef ANIM_PARSE_JSON_DOM_H_
#define ANIM_PARSE_JSON_DOM_H_

#include "JSValue.h"
JSValue parse_json(const char*& p, const char* end);
std::istream& operator>>(std::istream& in, JSValue& root);
std::ostream& operator<<(std::ostream& out, const JSValue& it);


#endif /* BIT_PARSE_JSON_DOM_H_ */
