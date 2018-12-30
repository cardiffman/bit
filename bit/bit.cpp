/*
 * bit.cpp
 *
 *  Created on: Nov 17, 2018
 *      Author: menright
 */

#include "bitwindow.h"
#include "json/json.h"
#include <fstream>
#include <iostream>
#include <poll.h>
#include <vector>
#include <list>
#include <unistd.h>
#include <climits>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include "png.h"
#include "jpeglib.h"
#include "jerror.h"

using std::cin;
using std::cout;
using std::vector;
using std::cerr;
using std::endl;

struct Asset {
	unsigned id; BitBuffer image;
};
struct Container {
	Area area; unsigned id; unsigned parent_id; unsigned asset_id; unsigned color; unsigned children;
};
struct Scene {
	vector<Asset> assets;
	vector<Container> containers;
};
static const unsigned ID_NULL = 0;
Scene scene = {
		{
		{ 0, {NULL, {0,0}, 0} },
		{ 1, {NULL, {1280, 720}, 1280*4 }},
		{ 2, {NULL, {100, 100}, 100*4 }},
		{ 3, 0},
		{ 4, 0},
		{ 5, 0},
		{ 6, 0},
		{ 7, 0},
		{ 8, 0},
		{ 9, 0},
		{10, 0},
		{11, 0},
		{12, 0},
		{13, 0},
		{14, 0},
		{15, 0},
		},
		{
				{ {0,0,0,0}, 0, 0, 0 },
				{ {  0,  0, 1280, 720}, 1, ID_NULL, ID_NULL, 0xff808080 },
				{ {  0,  0,  100, 100}, 2, ID_NULL, 3},//ID_NULL, 0xffff0000},
				{ {100,100, 1080, 520}, 3, ID_NULL, 1,  },
				{ {  0,  0,  127, 190}, 4, 3,       3},
				{ {127,  0,  127, 190}, 5, 3,       4, },
				{ {254,  0,  127, 190}, 6, 3,       5, },
				{ {381,  0,  127, 190}, 7, 3,      6, },
				{ {508,  0, 127, 190}, 8, 3,      7, },
				{ {635,  0, 127, 190}, 9, 3,      8, },
				{ {762,  0, 127, 190},10, 3,      9, },
				{ {889,  0, 127, 190},11, 3,     10, },
				{ {1016, 0, 127, 190},12, 3,     11, },
				{ {1143, 0, 127, 190},13, 3,     12, },
				{ {1270, 0, 127, 190},14, 3,     2, },
		}
};

void blit(BitBuffer& src
		, const Area& srcArea
		, BitBuffer& dst
		, const Area& dstArea)
{
	for (unsigned y = 0; y<srcArea.height; y++) {
		uint32_t* dstrow = (uint32_t*)(dst.mem + (dstArea.y+y)*dst.rowbytes);
		uint32_t* srcrow = (uint32_t*)(src.mem + (srcArea.y+y)*src.rowbytes);
		for (unsigned x = 0; x<srcArea.width; x++) {
			dstrow[x+dstArea.x] = srcrow[x+srcArea.x];
		}
	}
}
void stretch(BitBuffer& src
		, const Area& srcArea
		, BitBuffer& dst
		, const Area& dstArea)
{
	int x_ratio = (int)((srcArea.width<<16)/dstArea.width) +1;
    int y_ratio = (int)((srcArea.height<<16)/dstArea.height) +1;
    for (int i=0; i<dstArea.height; ++i)
    {
		int y2 = ((i*y_ratio)>>16);
		uint32_t* dstmem = (uint32_t*)(dst.mem + dst.rowbytes*(i+dstArea.y)+4*dstArea.x);
		uint32_t* srcmem = (uint32_t*)(src.mem + src.rowbytes*(y2+srcArea.y)+4*srcArea.x);
    	for (int j=0; j<dstArea.width; ++j)
    	{
    		int x2 = ((j*x_ratio)>>16);
    		dstmem[j] = srcmem[x2];
    		//dst.mem[((i*dst.dims.width)+j)*4+0]=src.mem[((y2*src.dims.width)+x2)*4+0];
    		//dst.mem[((i*dst.dims.width)+j)*4+1]=src.mem[((y2*src.dims.width)+x2)*4+1];
    		//dst.mem[((i*dst.dims.width)+j)*4+2]=src.mem[((y2*src.dims.width)+x2)*4+2];
    		//dst.mem[((i*dst.dims.width)+j)*4+3]=src.mem[((y2*src.dims.width)+x2)*4+3];
    	}
    }
}
void fill(uint32_t color
		, BitBuffer& dst
		, const Area& dstArea)
{
	for (unsigned y = 0; y<dstArea.height; y++) {
		uint32_t* dstrow = (uint32_t*)(dst.mem + (dstArea.y+y)*dst.rowbytes);
		for (unsigned x = 0; x<dstArea.width; x++) {
			dstrow[x+dstArea.x] = color;
		}
	}
}

void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;

void read_png_file(char* file_name)
{
	unsigned char header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("[read_png_file] File %s could not be opened for reading",
				file_name);
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		abort_("[read_png_file] File %s is not recognized as a PNG file",
				file_name);

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[read_png_file] png_create_info_struct failed");

	if (setjmp (png_jmpbuf(png_ptr)))abort_("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp (png_jmpbuf(png_ptr)))abort_("[read_png_file] Error during read_image");

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y = 0; y < height; y++)
		row_pointers[y] = (png_byte*) malloc(
				png_get_rowbytes(png_ptr, info_ptr));

	png_read_image(png_ptr, row_pointers);

	fclose(fp);
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

int read_JPEG_file (const char * filename, BitBuffer* bits)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  uint8_t* output;
  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.txt for more info.
   */

  cinfo.out_color_space = JCS_EXT_BGRA;//JCS_EXT_ARGB;
  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */
  bits->rowbytes = (cinfo.output_width * cinfo.output_components + 3) & ~3;
  bits->dims.width = cinfo.output_width;
  bits->dims.height = cinfo.output_height;
  bits->mem = (uint8_t*)malloc(bits->dims.height * bits->rowbytes);
  uint8_t* output_line = bits->mem;
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    memcpy(output_line, buffer[0], row_stride);
    output_line += bits->rowbytes;
  }

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return 1;
}


extern BitBuffer screen;
Container* parent_container(const Container* container)
{
	Container* parent = &scene.containers[container->parent_id];
	if (parent->id == ID_NULL)
		return nullptr;
	return parent;
}
/* \brief return true if container's ancestors have a loop
 * @param container the container of interest.
 */
bool check_parent_loop(const Container& container)
{
	Container* parent = nullptr;
	const Container* g = &container;
	int count = 0;
	parent = parent_container(g);
	while (parent && count < 100)
	{
		parent = parent_container(parent);
		count++;
	}
	if (count >= 100)
	{
		cout << "check_parent_loop: broken container: " << container.id << ' ' << container.parent_id << endl;
		g = &container;
		parent = parent_container(g);
		while (parent && count < 100)
		{
			cout << "check_parent_loop: parent chain:" << parent->id << ' ' << parent->parent_id << endl;
			parent = parent_container(parent);
			count++;
		}
		return true;
	}
	return false;
}
enum { CLIP_IN, CLIP_PARTIAL, CLIP_OUT };
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
int clip_container_to_parents(const Container& container, Area& draw)
{
	draw = container.area;
	Container* parent = parent_container(&container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		parent = parent_container(parent);
	}
	return res;
}
int clip_container_to_heirarchy(const Container& container, Area& draw, Area& screen)
{
	draw = container.area;
	screen = container.area;
	Container* parent = parent_container(&container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		parent = parent_container(parent);
	}
	res = clip_area_to_area(Area(0,0,1280,720), draw, draw);
	return res;
}
/*
 * @param container a Container that contains some area (such as asset area)
 * @param draw the area to start with and the final clipped area
 * @param screen the final area without clipping.
 */
int clip_area_to_heirarchy(const Container& container, Area& draw, Area& screen)
{
	draw.x += container.area.x;
	draw.y += container.area.y;
	screen = draw;
	Container* parent = parent_container(&container);
	int res = CLIP_IN;
	while (parent)
	{
		res = clip_area_to_area(parent->area, draw, draw);
		if (res == CLIP_OUT)
			return CLIP_OUT;
		draw.x += parent->area.x;
		draw.y += parent->area.y;
		screen.x += parent->area.x;
		screen.y += parent->area.y;
		parent = parent_container(parent);
	}
	return res;
}
void draw_container(const Container& g)
{
	if (g.children)
	{
		cout << "Container " << g.id << " has children, they will be drawn" << endl;
		return;
	}

	Area draw, container_screen;
	if (CLIP_OUT == clip_container_to_heirarchy(g, draw, container_screen))
	{
		cout << "Container " << g.id << " clipped out" << endl;
		return;
	}
	if (g.asset_id == ID_NULL)
	{
		cout << "Filling color " << g.color << " for container " << g.id << " at " << g.area.x << "," << g.area.y << endl;
		fill(g.color, screen, draw);
	}
	else
	{
		Area asset_draw, asset_screen;
		Asset& asset = scene.assets[g.asset_id];
		Area asset_area = Area(0,0,asset.image.dims.width,asset.image.dims.height);
		asset_draw = asset_area;
		if (CLIP_OUT == clip_area_to_heirarchy(g, asset_draw, asset_screen))
		{
			cout << "Container " << g.id << " asset " << asset.id << " clipped out" << endl;
			return;
		}
		asset_area;
		cout << "Drawing asset " << asset.id << " for container " << g.id << endl;
		if (g.area.width == asset_area.width && g.area.height == asset_area.height)
		blit(asset.image
				, asset_area
				, screen
				, g.area
				);
		else
			stretch(asset.image, asset_area, screen, g.area);
	}

}
void draw_scene()
{
	cout << "Drawing" << endl;
	for (auto g : scene.containers) {
		g.children = 0;
	}
	for (auto g : scene.containers) {
		if (g.parent_id != ID_NULL)
			scene.containers[g.parent_id].children++;
	}
	Container* parent = nullptr;
	for (unsigned ig = 1; ig<scene.containers.size(); ++ig) {
		Container& g = scene.containers[ig];
		if (check_parent_loop(g))
			continue;
		draw_container(g);
	}
}
double get_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + (ts.tv_nsec*1e-9);
}
#include "parse_json_utils.h"

std::vector<Container> nc;
std::vector<Asset> na;
std::map<std::string,Asset> named_a;
std::map<std::string,int> named_container_ids;
std::map<unsigned,std::string> urls_by_id;
int container_id = 0;
int asset_id = 0;
void parse_asset_label(unsigned& id, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	++asset_id;
	// File supplies a label. We define the ID here and map the label to the ID.
	id = asset_id;
	if (ptokens->type == JSMN_STRING)
	{
		auto asset_label = text.substr(ptokens->start, ptokens->end-ptokens->start);
		++ptokens;
		Asset a;
		a.id = asset_id;
		na.push_back(a);
		named_a[asset_label] = a;
		//ids_of_named_assets[a.id] = asset_label;
	}
	else
	{
		throw "asset label has to be string";
	}
}
void parse_color(unsigned& color, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_ARRAY)
	{
		int elements = ptokens->size;
		++ptokens;
		if (elements != 3 && elements != 4)
			throw "3 or 4 color components required";
		std::vector<unsigned> comps;
		for (int i=0; i<elements; ++i)
		{
			auto n = text.substr(ptokens->start, ptokens->end-ptokens->start);
			++ptokens;
			comps.push_back(std::stod(n));
		}
		if (elements == 3)
		{
			color = ((((0xFF00 | comps[0]) << 8) | comps[1]) << 8) | comps[2];
		}
		else
		{
			color = (((((comps[0] << 8) | comps[1]) << 8) | comps[2]) << 8) | comps[3];
		}
	}
	else
	{
		throw "color components must be array";
	}
}
void parse_area(Area& area, const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		for (int i=0; i<members; ++i)
		{
			auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
			++ptokens;
			auto val = std::stod(text.substr(ptokens->start, ptokens->end-ptokens->start));
			++ptokens;
			if (member_name == "x")
				area.x = val;
			else if (member_name == "y")
				area.y = val;
			else if (member_name == "width")
				area.width = val;
			else if (member_name == "height")
				area.height = val;
			else {
				throw "Bad member name in area";
			}
		}
	}
	else
	{
		throw "area components must be an object";
	}
}
void container_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens, int parent)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		++container_id;
		Container c;
		c.id = container_id;
		c.parent_id = parent;
		for (int i=0; i<members; ++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name == "label")
				{
					if (ptokens->type == JSMN_STRING)
					{
						auto label = text.substr(ptokens->start, ptokens->end-ptokens->start);
						named_container_ids[label] = c.id;
					}
					++ptokens;
					continue;
				}
				if (member_name == "area")
				{
					parse_area(c.area, text, ptokens);
				}
				else if (member_name == "fill")
				{
					parse_color(c.color, text, ptokens);
				}
				else if (member_name == "asset")
				{
					parse_asset_label(c.asset_id, text, ptokens);
				}
				else if (member_name == "containers")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						for (int i=0; i<elements; ++i)
						{
							container_read(text, ptokens, c.id);
						}
					}
					else
					{
						throw "containers member must be array";
					}
				}
			}
		}
		nc.push_back(c);
	}
}
void asset_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		Asset a = {0}; a.id = 0; std::string url;
		for (int i=0; i<members; ++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name == "label")
				{
					auto label = text.substr(ptokens->start, ptokens->end-ptokens->start);
					++ptokens;
					if (!named_a.count(label))
					{
						a.id = ++asset_id;
						na.push_back(a);
						named_a[label] = a;
					}
					else
					{
						a = named_a[label];
					}
				}
				else if (member_name == "url")
				{
					url = text.substr(ptokens->start, ptokens->end-ptokens->start);
					++ptokens;
				}
			}
		}
		urls_by_id[a.id] = url;
	}
}
//template <typename P>
void scene_read(const std::string& text, std::vector<jsmntok_t>::iterator& ptokens)
{
	if (ptokens->type == JSMN_OBJECT)
	{
		int members = ptokens->size;
		++ptokens;
		for (int i=0;i<members;++i)
		{
			if (ptokens->type == JSMN_STRING)
			{
				auto member_name = text.substr(ptokens->start, ptokens->end-ptokens->start);
				++ptokens;
				if (member_name=="containers")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						for (int i=0; i<elements; ++i)
						{
							container_read(text, ptokens, 0);
						}
					}
					else
					{
						throw "Expect containers in array";
					}
				}
				else if (member_name == "assets")
				{
					if (ptokens->type == JSMN_ARRAY)
					{
						int elements = ptokens->size;
						++ptokens;
						for (int i=0; i<elements; ++i)
						{
							asset_read(text, ptokens);
						}
					}
					else
					{
						throw "Expect assets in array";
					}
				}
			}
			else
			{
				throw "Bad object format, need string for member name";
			}
		}
	}
}
void print_scene()
{
	for (auto c : nc)
	{
		cout << "{{" << c.area.x <<',' << c.area.y << ',' << c.area.width << ',' << c.area.height <<"}, "
				<< c.id << ',' << c.parent_id << ',' << c.asset_id << ',' << c.color << '}' << " // " << c.children<< endl;
	}
	for (auto a : na)
	{
		cout << "{" << a.id << ',' << '{'<< a.image.dims.width << ','<< a.image.dims.height << '}'<< urls_by_id[a.id] << '}' << endl;
	}
}
bool by_id(const Container& a, const Container& b)
{
	return a.id < b.id;
}
void parse_containers(const char* file)
{
	std::vector<jsmntok_t> tokens;
	std::string jstext;
	tokenize_json(file, tokens, jstext);
	cout << "js: chars provided " << jstext.size() << endl;
	dump_jstokens(tokens, jstext);
	auto ptokens = tokens.begin();
	cout << "Scene" << endl;
	scene_read(&jstext[0], ptokens);
	cout << "Thats it" << endl;
	nc.insert(nc.begin(), Container());
	std::sort(nc.begin(), nc.end(), by_id);
	for (auto& g : nc) {
		g.children = 0;
	}
	for (auto& g : nc) {
		if (g.parent_id != ID_NULL)
			scene.containers[g.parent_id].children++;
	}
	na.insert(na.begin(), Asset());
	print_scene();
	for (auto u : urls_by_id)
	{
		if (u.second.find(".jpg")!=-1)
		{
			read_JPEG_file(u.second.c_str(), &na[u.first].image);
		}
	}
	print_scene();
}
int main(int argc, char**argv)
{
	try {
		//parse_json(argv[1]);
		if (argc==2)
		{
		parse_containers(argv[1]);
		scene.containers = nc;
		scene.assets = na;
		}
	} catch (const char* ex) {
		cout << ex << endl;
		return 1;
	}
	if (argc == 1)
	{
	scene.assets[1].image.mem = new uint8_t[720*1280*4];
	scene.assets[1].image.rowbytes = 1280*4;
	scene.assets[1].image.dims = RectSize(1280,720);
	uint32_t* pixels = (uint32_t*)scene.assets[1].image.mem;
	for (unsigned y = 0; y<720; y++)
		for (unsigned x = 0; x<1280; x++)
			pixels[y*1280+x] = ((x^y)&1)?0xFFFFFFFF:0xFF000000;
	scene.assets[2].image.mem = new uint8_t[100*100*4];
	scene.assets[2].image.rowbytes = 100*4;
	scene.assets[2].image.dims = RectSize(100,100);
	pixels = (uint32_t*)scene.assets[2].image.mem;
	for (unsigned y = 0; y<100; y++)
		for (unsigned x = 0; x<100; x++)
			pixels[y*100+x] = 0xFFFF0000;

	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/Vice2018.jpg", &scene.assets[3].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/BB_Online_Dom_Payoff_1-Sheet_H-Steinfeld_BB_Bridge_Autobot.jpg", &scene.assets[4].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/GRC_Tsr1Sheet_GrinchAndMax_.jpg", &scene.assets[5].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MULE_VERT_MAIN_DOM_2764x4096_master.jpg", &scene.assets[6].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/TheFavourite2018.jpg", &scene.assets[7].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/SecondAct_27x40_1Sheet_RGB.jpg", &scene.assets[8].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/AQAMN_VERT_MAIN_DUO_DOM_2764x4096_master.jpg", &scene.assets[9].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/TSNGO_TicketingBanner_250x375_r2.jpg", &scene.assets[10].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/HolmesAndWatson2018.jpg", &scene.assets[11].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/SpiderManIntoTheSpiderVerse2018.jpg", &scene.assets[12].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/WTM_HeroPoster.jpg", &scene.assets[13].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MQOS_OneSheet.jpg", &scene.assets[14].image);
	read_JPEG_file("/home/menright/wip/bits/res/Lesson5/MPR-Payoff_1-Sheet_v8a_Sm.jpg", &scene.assets[15].image);
	}
	bitwindow* win = bitwindow::create();
	win->configure(Rect((1920-1280)/2,(1080-720)/2,1280,720));
	draw_scene();
	win->repaint();
	//XCBWindow::eventLoop();

	double anim_last = get_time();
	double anim_next = anim_last+1.0/60;
	double poll_xcb = 0;
	while (true)
	{
		if (!XCBWindow::pollEvents())
			break;
	}
	return 0;
}


