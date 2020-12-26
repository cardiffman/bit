/*
 * win.cpp
 *
 *  Created on: Mar 12, 2020
 *      Author: micha
 */

#define STRICT
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include "cairo/cairo.h"
#include "engine.h"
#include "scene_builder.h"
#include <unistd.h>

using namespace std;

struct CairoBuffer : public GraphicsBuffer
{
    CairoBuffer(const RectSize& size)
    {
        dims = size;
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.width, size.height);
        //cairo_t* cr = cairo_create(surface);
        //cairo_set_source_rgba(cr, 0, 1, 0, 1);
        //cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        //cairo_rectangle(cr, 0, 0, size.width, size.height);
        //cairo_fill(cr);
        //cairo_destroy(cr);
    }
    CairoBuffer(const RectSize& size, void* data, unsigned rowbytes)
    {
        dims = size;
        surface = cairo_image_surface_create_for_data((uint8_t*)data, CAIRO_FORMAT_ARGB32, size.width, size.height, rowbytes);
    }
    void lock(uint8_t*& buffer, uint32_t& rowbytes)
    {
        cairo_surface_flush(surface);

        // modify the image
        buffer = cairo_image_surface_get_data(surface);
        rowbytes = cairo_image_surface_get_stride(surface);
    }
    void unlock()
    {
        // mark the image dirty so Cairo clears its caches.
        cairo_surface_mark_dirty (surface);
    }
    cairo_surface_t* surface;
};

struct CairoEngine : public GraphicsEngine
{
    static CairoEngine* factory(const RectSize& size)
    {
        static CairoEngine* instance = nullptr;
        if (!instance) instance = new CairoEngine(new CairoBuffer(size));
        return instance;
    }
    bool supportsBuffer(GraphicsBuffer* buf)
    {
        return true;// if it supports lock we can handle it.if (!dynamic_cast<BaseBufffer*>(buf) && !dynamic_cast<CairoBuffe)
    }
    GraphicsBuffer* getScreenBuffer()
    {
        return screen;
    }
    GraphicsBuffer* makeBuffer(const RectSize& size, void* data, uint32_t rowbytes)
    {
        return new CairoBuffer(size, data, rowbytes);
    }
    GraphicsBuffer* makeBuffer(const RectSize& size)
    {
        return new CairoBuffer(size);
    }
    void blit(GraphicsBuffer* dst, int dstX, int dstY, GraphicsBuffer* src, const Area& srcArea)
    {
        CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
        CairoBuffer* csrc = dynamic_cast<CairoBuffer*>(src);
        if (!cdst)
            return;
        else if (!csrc)
            return;
        else
        {
            cairo_t* cr = cairo_create(cdst->surface);
            cairo_set_source_surface(cr, csrc->surface, dstX - srcArea.x, dstY - srcArea.y);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_rectangle(cr, dstX, dstY, srcArea.width, srcArea.height);
            cairo_fill(cr);
            cairo_destroy(cr);
        }
    }
    void fill(GraphicsBuffer* dst, const Area& dstArea, uint32_t color)
    {
        CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
        if (!cdst)
            return;
        else
        {
            cairo_t* cr = cairo_create(cdst->surface);
            double r = ((color&0xFF0000)>>16)/255.0;
            double g = ((color&0xFF00)>> 8)/255.0;
            double b =  (color&0xFF)       /255.0;
            double a = ((color&0xFF000000)>>24)/255.0;
            cairo_set_source_rgba(cr, r, g, b, a);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_rectangle(cr, dstArea.x, dstArea.y, dstArea.width, dstArea.height);
            cairo_fill(cr);
            cairo_destroy(cr);
        }
    }
    void stretchSrcCopy(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea)
    {
        if (srcArea.width == dstArea.width && srcArea.height == dstArea.height)
        {
            blit(dst, dstArea.x, dstArea.y, src, srcArea);
            return;
        }
        CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
        CairoBuffer* csrc = dynamic_cast<CairoBuffer*>(src);
        if (!cdst)
            return;
        else if (!csrc)
            return;
        else
        {
            cairo_t* cr = cairo_create(cdst->surface);
            //cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_save(cr);
            /* CAIRO_OPERATOR_SOURCE has an unbounded interpretation
            so we need a clipping region to keep from drawing outside dstArea
            */
            cairo_rectangle(cr, dstArea.x, dstArea.y, dstArea.width, dstArea.height);
            cairo_clip(cr);
            cairo_scale(cr, 1.*dstArea.width/srcArea.width, 1.*dstArea.height/srcArea.height);
            cairo_set_source_surface(cr, csrc->surface, 1.*dstArea.x*srcArea.width/dstArea.width - 1.*srcArea.x*srcArea.width/dstArea.width, dstArea.y*1.*srcArea.height/dstArea.height - srcArea.y);
            cairo_paint(cr);
            cairo_restore(cr);
            cairo_destroy(cr);
        }
    }
    void stretchSrcOver(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea)
    {
        CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
        CairoBuffer* csrc = dynamic_cast<CairoBuffer*>(src);
        if (!cdst)
            return;
        else if (!csrc)
            return;
        else
        {
            cairo_t* cr = cairo_create(cdst->surface);
            cairo_save(cr);
            /* CAIRO_OPERATOR_OVER has a bounded interpretation
            so we don't need a clipping region to keep from drawing outside dstArea
            */
            cairo_scale(cr, 1.*dstArea.width/srcArea.width, 1.*dstArea.height/srcArea.height);
            cairo_set_source_surface(cr, csrc->surface, 1.*dstArea.x*srcArea.width/dstArea.width - 1.*srcArea.x*srcArea.width/dstArea.width, dstArea.y*1.*srcArea.height/dstArea.height - srcArea.y);
            cairo_paint(cr);
            cairo_restore(cr);
            cairo_destroy(cr);
        }
    }
private:
    CairoEngine(CairoBuffer* screen)
    : screen(screen)
    {
    	fill(screen, Area(0,0,1280,720), 0xFFEECCDD);//0x3FFF1010);//0x00000000);//0xFFFFF0F0);
    }
    CairoBuffer* screen;
};

static const int SCREEN_WIDTH = 1280;
static const int SCREEN_HEIGHT = 720;
struct MyBitmapInfo {
	BITMAPINFOHEADER bmih;
};
void RenderingToDib(GraphicsBuffer* bs, MyBitmapInfo& bmi, uint8_t*& lpBits) {
    unsigned rowBytes;
    bs->lock(lpBits, rowBytes);
    bmi.bmih.biSize = sizeof(bmi.bmih);
    bmi.bmih.biBitCount = 32;
    bmi.bmih.biHeight = -SCREEN_HEIGHT;
    bmi.bmih.biWidth = SCREEN_WIDTH;
    bmi.bmih.biCompression = BI_RGB;
    bmi.bmih.biPlanes = 1;
    bmi.bmih.biSizeImage = SCREEN_HEIGHT*SCREEN_WIDTH*4;
}
GraphicsEngine* engine;
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rect;
    SCROLLINFO scrollinfo;
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CREATE:
    	break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
#ifdef CAIRO_HAS_WIN32_SURFACE
            cairo_win32_surface_create_with_format(hdc, CAIRO_SURFACE_TYPE_WIN32);
            CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
            cairo_t* cr = cairo_create(cdst->surface);
            cairo_save(cr);
            cairo_scale(cr, 1.*dstArea.width/srcArea.width, 1.*dstArea.height/srcArea.height);
            cairo_set_source_surface(cr, csrc->surface, 1.*dstArea.x*srcArea.width/dstArea.width - 1.*srcArea.x*srcArea.width/dstArea.width, dstArea.y*1.*srcArea.height/dstArea.height - srcArea.y);
            cairo_paint(cr);
            cairo_restore(cr);
            cairo_destroy(cr);
#else
        	static bool needsRender = true;
        	static GraphicsBuffer* bs;
			static uint8_t* lpBits;
			static MyBitmapInfo bmi;
			static LPBITMAPINFO lpbmi = (LPBITMAPINFO)&bmi.bmih;
            if (needsRender) {
				bs = engine->getScreenBuffer();//rendering();
				needsRender  = false;
				//RenderingToDib(bs, bmi, lpBits);
            }
			RenderingToDib(bs, bmi, lpBits);
            uint32_t iUsage = DIB_RGB_COLORS;
            StretchDIBits(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
            		0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
					lpBits,
					lpbmi,
					iUsage,
					SRCCOPY);

#endif
            EndPaint(hwnd, &ps);
        }
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#if 0
std::string WidestringToString(std::wstring wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
#if defined WIN32
    int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
    std::string ret = std::string(size, 0);
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], wstr.size(), &ret[0], size, NULL, NULL);
#else
    size_t size = 0;
    _locale_t lc = _create_locale(LC_ALL, "en_US.UTF-8");
    errno_t err = _wcstombs_s_l(&size, NULL, 0, &wstr[0], _TRUNCATE, lc);
    std::string ret = std::string(size, 0);
    err = _wcstombs_s_l(&size, &ret[0], size, &wstr[0], _TRUNCATE, lc);
    _free_locale(lc);
    ret.resize(size - 1);
#endif
    return ret;
}
#endif

#include <locale>
#include <codecvt>
std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE _h, LPSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const TCHAR CLASS_NAME[]  = TEXT("Anim");

    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);
    string cmdA(GetCommandLineA());
    cerr << "Commandline " << cmdA << endl;
    int argc=0;
    auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    wstring cmd(GetCommandLineW());
    wcerr << "Commandline" << cmd << endl;
    wcerr << "Args " << argc << endl;
    for (auto p = argv; p != argv+argc+1; ++p)
    {
    	wcerr << *p << endl;
    }
	//return 0;

	engine = CairoEngine::factory(RectSize(SCREEN_WIDTH, SCREEN_HEIGHT));
	Scene scene;

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"GB",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_WIDTH, SCREEN_HEIGHT, //CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
    	sleep(10);
        return 0;
    }
    SceneBuilder builder;
    bool rendering = false;
    bool parsed = false;
    bool testing = false;
    int input_arg = 1;
    if (argc > 1)
    {
    	do {
			try {
                auto testing = argc > 2 && ws2s(argv[1])=="--testing";
                if (testing)
                    input_arg = 2;
                cout << "Started parsing of " << ws2s(argv[input_arg]) << endl;
				builder.parse_containers(ws2s(argv[input_arg]).c_str(), engine);
                cout << "Completed parsing" << endl;
                scene.containers = builder.nc;
                scene.assets = builder.na;
                parsed = true;
                rendering = true;
			} catch (char const * ex) {
				cout << "Exception " << ex << endl;
				break;
			} catch (const std::string& ex) {
				cout << "Exception " << ex << endl;
				break;
            } catch (std::exception& ex) {
                cout << "Exception " << ex.what() << endl;
                break;
			} catch (...) {
				cout << "Exception unknown"  << endl;
				break;
			}
    	} while (false);
    }
    if (!rendering)
    {
        scene.containers.push_back(Container());
        scene.containers.push_back(Container({{ 0,0,1280,720 },1,0,0,0xFF00FF00}));
        scene.assets.push_back(Asset());
        rendering = true;
    }
    if (rendering)
    {
        if (testing)
        	draw_scene2(scene, engine);
        else
            draw_scene(scene, engine);
    }
    ShowWindow(hwnd, nCmdShow);
    MSG msg = { 0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}



