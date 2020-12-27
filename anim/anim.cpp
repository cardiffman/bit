/*
 * anim.cpp
 *
 *  Created on: Jan 3, 2019
 *      Author: menright
 */

#include "scene.h"
#include "engine.h"
#include <iostream>
#include <algorithm>
//#include <ranges>

using std::cout;
using std::endl;
using std::vector;

std::ostream& operator <<(std::ostream& out, const Area& diag)
{
	out << '('<<diag.x << ',' << diag.y << ',' << diag.width << ',' << diag.height << ')';
	return out;
}
std::ostream& operator <<(std::ostream& out, const RectSize& diag)
{
	out << '(' << diag.width << ',' << diag.height << ')';
	return out;
}
const Container* parent_container(Scene const& scene, const Container* container)
{
	const Container* parent = &scene.containers[container->parent_id];
	if (parent->id == ID_NULL)
		return nullptr;
	return parent;
}
int number_line(int low_limit, int size_limit, int low, int size, int* draw_low, int* draw_size)
{
	if (low+size < low_limit)
	{
		*draw_low = *draw_size = 0;
		return CLIP_OUT;
	}
	if (low > low_limit+size_limit)
	{
		*draw_low = *draw_size = 0;
		return CLIP_OUT;
	}
	if (low >= low_limit && low+size <= low_limit+size_limit)
	{
		*draw_low = low;
		*draw_size = size;
		return CLIP_IN;
	}
	int draw_left = low; int draw_right = low+size;
	if (draw_left < low_limit)
	{
		draw_left = low_limit;
	}
	if (draw_right > low_limit+size_limit)
	{
		draw_right = low_limit+size_limit;
	}
	*draw_low = draw_left;
	*draw_size = draw_right - draw_left;
	return CLIP_PARTIAL;
}
int clip_area_to_area(const Area& limit_area, const Area& area, Area& draw)
{
	int res1 = number_line(limit_area.x, limit_area.width, area.x, area.width, &draw.x, &draw.width);
	if (res1 == CLIP_OUT)
	{
		draw.y = 0; draw.height = 0;
		return res1;
	}
	int res2 = number_line(limit_area.y, limit_area.height, area.y, area.height, &draw.y, &draw.height);
	if (res2 == CLIP_OUT)
	{
		draw.x = 0; draw.width = 0;
		return res2;
	}
	if (res1 == CLIP_IN && res2 == CLIP_IN)
		return CLIP_IN;
	return CLIP_PARTIAL;
}
int clip_container_to_heirarchy(Scene& scene, const Container& container, Area& draw, Area& screen)
{
	draw = container.area;
	screen = container.area;
	const Container* parent = parent_container(scene, &container);
	int res = CLIP_IN;
	while (parent)
	{
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
		{
			return CLIP_OUT;
		}
		parent = parent_container(scene, parent);
	}
	res = clip_area_to_area(Area(0,0,1280,720), draw, draw);
	return res;
}
/*
 * @param container a Container that contains some area (such as asset area)
 * @param draw the area to start with and the final clipped area
 * @param screen the final area without clipping.
 */
int clip_area_to_heirarchy(const Scene& scene, const Container& container, Area& draw, Area& screen)
{
	draw.x += container.area.x;
	draw.y += container.area.y;
	screen = draw;
	const Container* parent = parent_container(scene, &container);
	int res = CLIP_IN;
	while (parent)
	{
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
		{
			return CLIP_OUT;
		}
		parent = parent_container(scene, parent);
	}
	return res;
}

void draw_container(const Container& g, Scene& scene, GraphicsBuffer* screen, GraphicsEngine* engine)
{
	if (g.children)
	{
		cout << "Container " << g.id << " has children, they will be drawn" << endl;
		return;
	}

	Area draw, container_screen;
	if (CLIP_OUT == clip_container_to_heirarchy(scene, g, draw, container_screen)
		|| draw.width == 0
		|| draw.height == 0)
	{
		cout << "Container " << g.id << " clipped out" << endl;
		return;
	}
	if (g.asset_id == ID_NULL)
	{
		cout << "Filling color " << std::hex << g.color << std::dec << " for container " << g.id << " at " << draw << endl;
		engine->fill(screen, draw, g.color);
	}
	else
	{
		Area asset_draw, asset_screen;
		if (g.color & 0xFF000000) // If this color is not fully transparent.
		{
			engine->fill(screen, draw, g.color);
		}
		Asset& asset = scene.assets[g.asset_id];
		Area asset_area = Area(0,0,asset.image->dims.width,asset.image->dims.height);
		asset_draw = asset_area;
		if (CLIP_OUT == clip_area_to_heirarchy(scene, g, asset_draw, asset_screen))
		{
			cout << "Container " << g.id << " asset " << asset.id << " clipped out" << endl;
			return;
		}
		bool unscaled = g.area.width == asset.image->dims.width && g.area.height == asset.image->dims.height;
		cout << "Drawing asset " << asset.id << " for container " << g.id << ' ' << (unscaled?"unscaled":"scaled") <<" at " << asset_draw << " from " << asset.image->dims << " as " << asset_screen << " csc " << draw << " cs " << container_screen << endl;
		// If the asset area and the container area are equal then
		// draw 1:1 using the clipping so far.
		if (g.area.width == asset.image->dims.width && g.area.height == asset.image->dims.height)
		{
			cout << "unscaled" << endl;
			engine->blit(screen
					, draw.x, draw.y
					, asset.image
					, asset_area
					);
		}
		else
		{
			// If the asset has to be stretched, then the clipped screen coordinates of the container
			// need to be mapped down to the asset's u,v space.
			// Believe it or not I got this right the second time I wrote it from scratch.
			// In this case, asset_draw set up with the asset's coordinates and then mapped to
			// the screen and clipped. asset_screen is the asset's screen coordinates without clipping.
			// However if the above conditional is false (which it is) then the rectangle
			// The lhs of the asset in u,v is asset_draw.x - asset_screen.x
			// The rhs of the asset in u,v is asset_draw.x + asset_draw.width - asset_screen.x
			int num_asset_width = asset.image->dims.width;
			int den_asset_width = container_screen.width;
			int num_asset_height = asset.image->dims.height;
			int den_asset_height = container_screen.height;
			asset_area.x = (draw.x - container_screen.x) * num_asset_width / den_asset_width;
			asset_area.width = draw.width * num_asset_width / den_asset_width;
			asset_area.y = (draw.y - container_screen.y) * num_asset_height / den_asset_height;
			asset_area.height = draw.height * num_asset_height /den_asset_height;
			cout << " Transformed " << asset_area << endl;
			cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
					<< den_asset_width << ',' << den_asset_height << ')' << endl;
			// Sanity check:
			if (asset_area.x < 0)
			{
				cout << asset_area << " asset draw left is negative" << endl;
				return;
			}
			if (asset_area.width > asset.image->dims.width)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw width is greater than asset width " << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				cout << " Input width " << draw.width << endl;
				return;
			}
			if (asset_area.x + asset_area.width > asset.image->dims.width)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw right exceeds asset right" << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			if (asset_area.y < 0)
			{
				cout << asset_area << " asset draw top is negative" << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			if (asset_area.height > asset.image->dims.height)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw height is greater than asset height " << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			if (asset_area.y + asset_area.height > asset.image->dims.height)
			{
				cout << asset_area << " vs " << asset.image->dims << " asset draw bottom exceeds asset bottom" << endl;
				cout << " Transform num (" << num_asset_width << ',' << num_asset_height << ") den ("
						<< den_asset_width << ',' << den_asset_height << ')' << endl;
				return;
			}
			engine->stretchSrcOver(screen, draw, asset.image, asset_area);
		}
	}

}

/* \brief return true if container's ancestors have a loop
 * @param container the container of interest.
 */
bool check_parent_loop(Scene& scene, const Container& container)
{
	const Container* parent = nullptr;
	const Container* g = &container;
	int count = 0;
	parent = parent_container(scene, g);
	while (parent && count < 100)
	{
		parent = parent_container(scene, parent);
		count++;
	}
	if (count >= 100)
	{
		cout << "check_parent_loop: broken container: " << container.id << ' ' << container.parent_id << endl;
		g = &container;
		parent = parent_container(scene, g);
		while (parent && count < 100)
		{
			cout << "check_parent_loop: parent chain:" << parent->id << ' ' << parent->parent_id << endl;
			parent = parent_container(scene, parent);
			count++;
		}
		return true;
	}
	return false;
}

int clip_container_to_parents(const Scene& scene, const Container& container, Area& draw)
{
	draw = container.area;
	const Container* parent = parent_container(scene, &container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		parent = parent_container(scene, parent);
	}
	return res;
}
void make_screen_rect(Scene& scene, const Container& container, Area& screen)
{
	screen = container.area;
	auto parent = parent_container(scene, &container);
	while (parent)
	{
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		parent = parent_container(scene, parent);
	}
}
void draw_scene(Scene& scene, GraphicsEngine* engine)
{
	cout << "Drawing" << endl;
	for (auto& g : scene.containers) {
		g.children = 0;
	}
	for (auto& g : scene.containers) {
		if (g.parent_id != ID_NULL)
			scene.containers[g.parent_id].children++;
	}
	Container* parent = nullptr;
	for (unsigned ig = 1; ig<scene.containers.size(); ++ig) {
		Container& g = scene.containers[ig];
		if (check_parent_loop(scene, g))
			continue;
		draw_container(g, scene, engine->getScreenBuffer(), engine);
	}
}

struct Region {
	typedef std::pair<int,int> Box;
	typedef std::vector<Box> Boxes;
	struct Band {
		int top;
		int height;
		Boxes boxes; // left,width, non-overlapping sorted by left.
	};
	typedef std::vector<Band> Bands;
	Bands bands; // invariants: bands don't overlap with each other. Sorted by top
				// The pairs in each band are non-overlapping integer intervals.
				// If an area overlaps the bounding box, it must also overlap a box in one
				// of the bands.
	Area boundingBox; // The bounding box of the region, for even quicker trivial elimination.
	typedef std::pair<Bands::const_iterator,Bands::const_iterator> BandRange;
	BandRange vertical_overlap(const Area& area) const;
	typedef std::pair<Boxes::const_iterator,Boxes::const_iterator> BoxRange;
	BoxRange horizontal_overlap(const Band& band, const Area& area) const;
	bool overlaps(const Area& area) const;
	Region get_overlap(const Area& area) const;
	static Area union_area(const Area& a, const Area& b) {
		auto x = std::min(a.x, b.x);
		auto right = std::max(a.x+a.width, b.x+b.width);
		auto y = std::min(a.y, b.y);
		auto bottom = std::max(a.y+a.height, b.y+b.height);
		return Area(x, right-x, y, bottom-y);
	}
	void union_area(const Area& in) {
		// TK
		// determine bands, then do this:
		boundingBox = union_area(boundingBox, in); /// dumbest way.
	}
	Region subtract_region_from_rect(const Area& area) const;

};
Region::BandRange Region::vertical_overlap(const Area& area) const {
	Region::Bands::const_iterator first,last,p;
	for (first = bands.begin(); first!=bands.end(); ++first) {
		int overlap_top, overlap_height;
		auto overlap = number_line(first->top, first->height, area.y, area.height, &overlap_top, &overlap_height);
		if (overlap != CLIP_OUT)
			break;
	}
	for (last = first; last!=bands.end(); ++last) {
		int overlap_top, overlap_height;
		auto overlap = number_line(last->top, last->height, area.y, area.height, &overlap_top, &overlap_height);
		if (overlap == CLIP_OUT)
			break;
	}
	return std::make_pair(first,last);
}
//Region::BoxRange Region::horizontal_overlap(const Region::Band& band, const Area& area) const {}

Region::BoxRange Region::horizontal_overlap(const Region::Band& band, const Area& area) const {
	std::vector<std::pair<int,int>>::const_iterator first,last;
	for (first = band.boxes.begin(); first!=band.boxes.end(); ++first) {
		int overlap_left, overlap_width;
		auto overlap = number_line(first->first, first->second, area.y, area.height, &overlap_left, &overlap_width);
		if (overlap != CLIP_OUT)
			break;
	}
	for (last = first; last!=band.boxes.end(); ++last) {
		int overlap_top, overlap_height;
		auto overlap = number_line(last->first, last->second, area.y, area.height, &overlap_top, &overlap_height);
		if (overlap == CLIP_OUT)
			break;
	}
	return std::make_pair(first,last);
}
bool Region::overlaps(const Area& area) const {
	if (!intersect(area, boundingBox))
		return false; // trivial reject
	if (bands.empty())
		return true; // optimization when region decays to a rectangle.
	auto vrange = vertical_overlap(area);
	if (vrange.first == bands.end())
		return false;
	// [vrange.first,vrange.second) is the set of bands that vertically match the area.
	// Now we need to look within each band to see if the area touches one of the boxes.
	for (auto pband = vrange.first; pband != vrange.second; ++pband)
	{
		auto hrange = horizontal_overlap(*pband, area);
		if (hrange.first != pband->boxes.end()) {
			// One of the bands has a box interval that overlaps the area.
			return true;
		}
	}
	return false;
}
Region Region::get_overlap(const Area& area) const {
	Region overlaps;
	if (!intersect(area, boundingBox))
		return overlaps; // trivial reject
	if (bands.empty()) {
		Area overlap;
		clip_area_to_area(boundingBox, area, overlap);
		overlaps.union_area(overlap);
		return overlaps; // optimization when region decays to a rectangle.
	}
	for (auto pband = bands.begin(); pband != bands.end(); pband++)
	{
		int overlap_top, overlap_height;
		auto voverlap = number_line(pband->top, pband->height, area.y, area.height, &overlap_top, &overlap_height);
		if (voverlap == CLIP_OUT)
			continue;
		for (auto pbox = pband->boxes.begin(); pbox != pband->boxes.end(); ++pbox) {
			int overlap_left, overlap_width;
			auto overlap = number_line(pbox->first, pbox->second, area.x, area.width, &overlap_left, &overlap_width);
			if (overlap != CLIP_OUT) {
				Area output;
				output.x = overlap_left;
				output.y = overlap_top;
				output.height = overlap_height;
				output.width = overlap_width;
				overlaps.union_area(output); // !! This does a search :( FIXME
			}
		}
	}
	return overlaps;
}
/*
	Take the region out of the rect to make a new region.
	E.g. if the region overlaps the upper-left-hand corner of the rect then remove that upper left hand corner area from
	the rect.
	The resulting piece of the plane has to be represented by a region in general, even if this region is merely a rectangle.
*/
Region Region::subtract_region_from_rect(const Area& area) const
{
	//
	return Region();
}

#define CONCEPT3
#ifdef CONCEPT3
struct Piece {
	unsigned cid;
	unsigned aid;
	Area  screen_unclipped;
	Area  screen;
	Area  asset_area;
};
bool piece_from_container(const Scene& scene, const Container& container, Piece& piece) {
	//Piece piece;
	piece.cid = container.id;
	piece.aid = container.asset_id;
	piece.screen_unclipped = container.area;
	piece.screen = container.area;
	piece.asset_area = scene.assets[piece.aid].image->dims;
	// WRONG need to modify screen and asset_area according to the actual drawn part of
	// the container.
	return CLIP_OUT != clip_area_to_heirarchy(scene, container, piece.screen, piece.screen_unclipped);
}
typedef vector<Piece> Pieces;
void pieces_from_scene(const Scene& scene, Pieces& pieces) {
	for (auto container : scene.containers) {
		Piece piece;
		if (piece_from_container(scene, container, piece)) {
			pieces.push_back(piece);
		}
	}
}
#endif
void draw_scene2(Scene& scene, GraphicsEngine* engine)
{
	cout << "Drawing2" << endl;
	for (auto& g : scene.containers) {
		g.children = 0;
	}
	for (auto& g : scene.containers) {
		if (g.parent_id != ID_NULL)
			scene.containers[g.parent_id].children++;
		if (check_parent_loop(scene, g)) {
			cout << "Container id " << g.id << " has an ancestor loop" << endl;
			return;
		}
	}
	//Container* parent = nullptr;
	/*
	Concept 1:
		Start with the set of containers to be drawn,
		and a union of all containers drawn so far.
		Loop:
		Obtain the highest priority container.
		Subtract from it the containers drawn so far (the union of them), giving new_draw.
		Add new_draw to the union.
		Draw new_draw.
		Repeat Loop until all containers have been drawn.
	Concept 2:
		Based on Edge Table + Active Edge Table polygon scan conversion such as described at:
		https://people.eecs.berkeley.edu/~ug/slide/pipeline/assignments/scan/
		But our figures are axis-aligned rectangles and therefore only have four active edges. The reference treats
		horizontal edges as a rare special case, where as for us they are the other signifcant case and optimize it.
		Also our vertical edges' slopes are not 'interesting'.
		Create a static edge table which has all the left and right edges of all the containers.
		Each edge has a known container ID, top, bottom, x, and left/right flag.
		Gather these edges in the static edge table by top then x.
		For a given container in the edge table, scan-convert the container starting from its left edge to its right edge.
		Performing scan conversion:
		Intent is to scan from left edge to right edge and render the Area appropriately in between.
		Create an active edge table which has the edges that cross/touch the first scanline.
		These edges should be gathered by x.
		Walk across the active edge table from left to right.
		For the current pixel, determine the active containers and within this the priority container.
		Scan across until the priority container changes as an edge is crossed, rendering from this priority container.
	Concept 3: Very vague:
		Make the static Edge Table as in concept 2.
		Gather up a collection of container blocks which are areas where only one container has to be painted to render
		the correct image in that area. Use the Edge Table to determine these blocks.
		Parallelism may result from having a producer which computes the blocks and a consumer which renders the blocks.
	*/
#ifdef CONCEPT1
	Region painted;
	// int number_line(int low_limit, int size_limit, int low, int size, int* draw_low, int* draw_size);
	unsigned sz = scene.containers.size();
	for (unsigned ig = 1; ig<scene.containers.size(); ++ig) {
		Container& g = scene.containers[sz-ig];
		Area& a = g.area;
		auto findUnpaintedAreas = [painted](const Area& a) {
			if (painted.bands.empty() || !intersect(a, painted.boundingBox)) {
				return std::vector<Area>({a}); // No overlap so the whole Area may be painted.
			}
			std::vector<Area> r;
			for (auto p : painted.bands) {
				if (bottom(a) < p.top)
					break;
				//
			}
			return r;
		};
		std::vector<Area> unpainted = findUnpaintedAreas(a);
		draw_container(g, scene, engine->getScreenBuffer(), engine);
	}
#elif defined(CONCEPT2)
	vector<Container> transformed_containers;
	std::transform(scene.containers.begin(), scene.containers.end(), back_inserter(transformed_containers),[&scene](const Container& c) {
		//clip_area_to_heirarchy(scene, container, draw, screen);
		//clip_area_to_heirarchy(scene, c, draw, screen);
		#if 1
		Area draw;
		Area screen;
		const Container& container = c;
		Container r = c;
		draw.x += container.area.x;
		draw.y += container.area.y;
		screen = draw;
		Container* parent = parent_container(scene, &container);
		int res = CLIP_IN;
		while (parent)
		{
			draw.x += parent->area.x;
			draw.y += parent->area.y;
			screen.x += parent->area.x;
			screen.y += parent->area.y;
			res = clip_area_to_area(parent->area, draw, draw);
			if (res == CLIP_OUT)
			{
				r.area = draw;
				return r;
			}
			parent = parent_container(scene, parent);
		}
		//return draw;
		r.area = draw;
		return r;
		//return res;
		#else
		Container r = c;
		int x=0; int y=0;
		const Container* parent = c.parent_id?&scene.containers[c.parent_id]:nullptr;
		while (parent) {
			clip_area_to_area(parent->area, c.area, r.area);
			x+=parent->area.x;
			y+=parent->area.y;
			parent = parent->parent_id?&scene.containers[parent->parent_id]:nullptr;
		}
		r.area.x += x;
		r.area.y += y;
		r.parent_id = 0;
		return r;
		#endif
	});
	vector<unsigned> ordered_containers;
	cout << "scene containers " << scene.containers.size() << endl;
	std::transform(transformed_containers.begin(), transformed_containers.end(), back_inserter(ordered_containers),[](const Container& c) { return c.id; });
	cout << "Ordered containers size " << ordered_containers.size() << endl;
	std::sort(ordered_containers.begin(), ordered_containers.end(), [transformed_containers](int cid1, int cid2){
		Container const & c1 = transformed_containers[cid1];
		Container const & c2 = transformed_containers[cid2];
		return c1.area.y<c2.area.y || (c1.area.y==c2.area.y && (c1.area.x<c2.area.x || (c1.area.x==c1.area.x && c1.id>c2.id)));
	});
	cout << "sorted ";
	for (auto cid: ordered_containers)
		cout << cid << '.' << transformed_containers[cid].area.y << ',' << transformed_containers[cid].area.x << ' ';
	cout << endl;
	vector<unsigned> active_containers;
	int first_ordered=0;
	int last_ordered=0;
	for (int y=0; y<16/*engine->getScreenBuffer()->dims.height*/; y++) {
		cout << "Drawing2 " << y << endl;
		while (transformed_containers[ordered_containers[first_ordered]].area.y!=y){
			cout << "Drawing2 first_ordered " << first_ordered << endl;
			first_ordered++;
		}
		last_ordered = std::min(last_ordered,first_ordered);
		while (transformed_containers[ordered_containers[last_ordered]].area.y==y){
			cout << "Drawing2 last_ordered " << last_ordered << " " << ordered_containers[last_ordered] <<'.' << transformed_containers[ordered_containers[last_ordered]].area.y << endl;
			last_ordered++;
		}
		for (int ci = first_ordered; ci<=last_ordered; ++ci){
			cout << "Actually drawing container " << ordered_containers[ci] << endl;
			draw_container(transformed_containers[ordered_containers[ci]], scene, engine->getScreenBuffer(), engine);
		}
		break;
	}
#elif defined(CONCEPT3)

#endif
}

