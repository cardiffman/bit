/*
 * read_JPEG_file.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_READ_JPEG_FILE_H_
#define BIT_READ_JPEG_FILE_H_

class GraphicsEngine;
class GraphicsBuffer;

int read_JPEG_file (const char * filename, GraphicsEngine* engine, GraphicsBuffer*& buffer);




#endif /* BIT_READ_JPEG_FILE_H_ */
