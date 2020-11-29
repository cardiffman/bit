/*
 * read_png_file.h
 *
 *  Created on: Dec 30, 2018
 *      Author: menright
 */

#ifndef BIT_READ_PNG_FILE_H_
#define BIT_READ_PNG_FILE_H_

class GraphicsEngine;
class GraphicsBuffer;

void read_png_file(const char* file_name, GraphicsEngine* engine, GraphicsBuffer*& DEST);


#endif /* BIT_READ_PNG_FILE_H_ */
