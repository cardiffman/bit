/*
 * bitbuffer.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_BITBUFFER_H_
#define BIT_BITBUFFER_H_

#include <stdint.h>
#include "geometry.h"
struct BitBuffer {
	uint8_t* mem; RectSize dims; unsigned rowbytes;
};



#endif /* BIT_BITBUFFER_H_ */
