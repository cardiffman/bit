#ifndef INCLUDED_GEOMETRY_H
#define INCLUDED_GEOMETRY_H
struct RectSize;
struct Area {
	Area() : x(), y(), width(), height() {}
	Area(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
	Area(const RectSize& size);
	int x; int y; int width; int height;
};
inline bool bottom(const Area& a) {
	return a.y+a.height;
};

enum { CLIP_IN, CLIP_PARTIAL, CLIP_OUT };
int number_line(int low_limit, int size_limit, int low, int size, int* draw_low, int* draw_size);
int clip_area_to_area(const Area& limit_area, const Area& area, Area& draw);
inline bool intersect(const Area& a, const Area& b) {
	Area draw;
	int clip = clip_area_to_area(a, b, draw);
	return clip != CLIP_OUT;
};

struct RectSize {
	RectSize() : width(), height() {}
	RectSize(int width, int height) : width(width), height(height) {}
	int width; int height;
};

inline Area::Area(const RectSize& size) {
	x = 0; y = 0; width = size.width; height = size.height;
}
#endif