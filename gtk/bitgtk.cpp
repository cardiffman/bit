/*
 * bitgtk.cpp
 *
 *  Created on: Dec 25, 2018
 *      Author: menright
 */
#include <gtk/gtk.h>
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <string.h>
#include "scene.h"
#include "scene_builder.h"
#include "engine.h"
#include <iostream>

#include <signal.h>

using std::cout;
using std::endl;

#define EXPOSE_USES_ENGINE 1
#ifndef EXPOSE_USES_ENGINE
//the global pixmap that will serve as our buffer
static GdkPixmap *g_pixmap = NULL;
#endif
Scene scene;

#include <gtk/gtk.h>

struct CairoBuffer : public GraphicsBuffer
{
    CairoBuffer(const RectSize& size)
    {
        dims = size;
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.width, size.height);
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
    static CairoEngine* factory()
    {
        static CairoEngine* instance = nullptr;
        if (!instance) instance = new CairoEngine(new CairoBuffer(RectSize(1280, 720)));
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
        return new CairoBuffer(size);
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
            ;
        else if (!csrc)
            ;
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
            ;
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
        CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
        CairoBuffer* csrc = dynamic_cast<CairoBuffer*>(src);
        if (!cdst)
            ;
        else if (!csrc)
            ;
        else
        {
            cairo_t* cr = cairo_create(cdst->surface);
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_set_source_surface(cr, csrc->surface, dstArea.x - srcArea.x, dstArea.y - srcArea.y);
            cairo_rectangle(cr, dstArea.x, dstArea.y, srcArea.width, srcArea.height);
            cairo_fill(cr);
            cairo_destroy(cr);
        }
    }
    void stretchSrcOver(GraphicsBuffer* dst, const Area& dstArea, GraphicsBuffer* src, const Area& srcArea)
    {
        CairoBuffer* cdst = dynamic_cast<CairoBuffer*>(dst);
        CairoBuffer* csrc = dynamic_cast<CairoBuffer*>(src);
        if (!cdst)
            ;
        else if (!csrc)
            ;
        else
        {
            cairo_t* cr = cairo_create(cdst->surface);
            cairo_set_source_surface(cr, csrc->surface, dstArea.x - srcArea.x, dstArea.y - srcArea.y);
            cairo_rectangle(cr, dstArea.x, dstArea.y, srcArea.width, srcArea.height);
            cairo_fill(cr);
            cairo_destroy(cr);
        }
    }
private:
    CairoEngine(CairoBuffer* screen)
    : screen(screen)
    {

    }
    CairoBuffer* screen;
};


gboolean on_window_configure_event(GtkWidget * da, GdkEventConfigure * event, gpointer user_data){
    #ifndef EXPOSE_USES_ENGINE
    #else
    // Need something, maybe
    #endif
    return TRUE;
}

gboolean on_window_expose_event(GtkWidget * da, GdkEventExpose * event, gpointer user_data){
    cairo_t* win = gdk_cairo_create(da->window);
    CairoEngine* engine = (CairoEngine*)user_data;
    GraphicsBuffer* generic_buf = engine->getScreenBuffer();
    CairoBuffer* cairo_buf = dynamic_cast<CairoBuffer*>(generic_buf);
    cairo_surface_t* buf_surf = cairo_buf->surface;
    cairo_set_source_surface(win, buf_surf, 0, 0);
    cairo_paint(win);
    cairo_destroy(win);
    return TRUE;
}
gboolean on_key_press(GtkWidget* da, GdkEventKey* event, gpointer user_data)
{
	cout << "KEY PRESS " << gdk_keyval_name(event->keyval) << endl;
	GDK_KEY_space;
	return TRUE;
}
gboolean on_key_release(GtkWidget* da, GdkEventKey* event, gpointer user_data)
{
	cout << "KEY RELEASE " << gdk_keyval_name(event->keyval) << endl;
	GDK_KEY_space;
	return TRUE;
}
int main (int argc, char *argv[]){

    //we need to initialize all these functions so that gtk knows
    //to be thread-aware
    //if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);
    try {
        GraphicsEngine* engine = CairoEngine::factory();//init_base_engine(new CairoBuffer(RectSize(1280,720)));



        GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_widget_set_size_request (window, 1280, 720);
        gtk_widget_add_events(window, GDK_KEY_PRESS_MASK  | GDK_EXPOSURE_MASK);
        g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
        g_signal_connect(G_OBJECT(window), "expose_event", G_CALLBACK(on_window_expose_event), engine);
        g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(on_window_configure_event), NULL);

        g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);
        g_signal_connect (G_OBJECT (window), "key_release_event", G_CALLBACK (on_key_release), NULL);

        gtk_widget_show_all(window);

        int width=1280, height=720;

        SceneBuilder builder;
        if (argc > 1)
        {
            try {
                builder.parse_containers(argv[1], engine);
            } catch (char const * ex) {
                cout << "Exception " << ex << endl;
                return 1;
            } catch (const std::string& ex) {
                cout << "Exception " << ex << endl;
                return 1;
            }
            scene.containers = builder.nc;
            scene.assets = builder.na;
        }
        else
        {
            scene.containers.push_back(Container());
            scene.containers.push_back(Container({{ 0,0,1280,720 },1,0,0,0xFF00FF00}));
            scene.assets.push_back(Asset());
        }
        draw_scene(scene, engine);
        gtk_widget_queue_draw_area(window, 0, 0, 1280, 720);
    } catch (char const * ex) {
        cout << "Exception " << ex << endl;
        return 1;
    } catch (const std::string& ex) {
        cout << "Exception " << ex << endl;
        return 1;
    }

    gtk_main();
    gdk_threads_leave();

    return 0;
}
