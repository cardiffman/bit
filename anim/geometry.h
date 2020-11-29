#ifndef INCLUDED_GEOMETRY_H
#define INCLUDED_GEOMETRY_H
struct Area {
	Area() : x(), y(), width(), height() {}
	Area(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
	int x; int y; int width; int height;
};
struct RectSize {
	RectSize() : width(), height() {}
	RectSize(int width, int height) : width(width), height(height) {}
	int width; int height;
};

#endif