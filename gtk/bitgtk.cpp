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

//the global pixmap that will serve as our buffer
static GdkPixmap *pixmap = NULL;
Scene scene;

#include <gtk/gtk.h>

gboolean on_window_configure_event(GtkWidget * da, GdkEventConfigure * event, gpointer user_data){
    static int oldw = 0;
    static int oldh = 0;
    //make our selves a properly sized pixmap if our window has been resized
    if (oldw != event->width || oldh != event->height){
        //create our new pixmap with the correct size.
        GdkPixmap *tmppixmap = gdk_pixmap_new(da->window, event->width,  event->height, -1);
        //copy the contents of the old pixmap to the new pixmap.  This keeps ugly uninitialized
        //pixmaps from being painted upon resize
        int minw = oldw, minh = oldh;
        if( event->width < minw ){ minw =  event->width; }
        if( event->height < minh ){ minh =  event->height; }
        gdk_draw_drawable(tmppixmap, da->style->fg_gc[GTK_WIDGET_STATE(da)], pixmap, 0, 0, 0, 0, minw, minh);
        //we're done with our old pixmap, so we can get rid of it and replace it with our properly-sized one.
        g_object_unref(pixmap);
        pixmap = tmppixmap;
    }
    oldw = event->width;
    oldh = event->height;
    return TRUE;
}

gboolean on_window_expose_event(GtkWidget * da, GdkEventExpose * event, gpointer user_data){
    gdk_draw_drawable(da->window,
        da->style->fg_gc[GTK_WIDGET_STATE(da)], pixmap,
        // Only copy the area that was exposed.
        event->area.x, event->area.y,
        event->area.x, event->area.y,
        event->area.width, event->area.height);
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
struct CairoBuffer : public GraphicsBuffer
{
    CairoBuffer(const RectSize& size)
    {
        this->dims = size;
        rowbytes = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, size.width);
        mem = (uint8_t*)malloc(rowbytes * dims.height);
        memset(mem, 0, rowbytes * dims.height);
    }
    void lock(uint8_t*& buffer, uint32_t& rowbytes)
    {
        buffer = mem;
        rowbytes = this->rowbytes;
    }
    void unlock() {}
    uint8_t* mem;
    uint32_t rowbytes;
};

int main (int argc, char *argv[]){

    //we need to initialize all these functions so that gtk knows
    //to be thread-aware
    //if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 1280, 720);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "expose_event", G_CALLBACK(on_window_expose_event), NULL);
    g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(on_window_configure_event), NULL);

    g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (on_key_press), NULL);
    g_signal_connect (G_OBJECT (window), "key_release_event", G_CALLBACK (on_key_release), NULL);

    gtk_widget_show_all(window);

    GdkPixmap *tmppixmap = gdk_pixmap_new(window->window, 1280,  720, -1);
    {
    cairo_t *cr_pixmap = gdk_cairo_create(tmppixmap);
    cairo_set_source_rgb (cr_pixmap, 250, 0, 0);
    cairo_paint(cr_pixmap);
    cairo_destroy(cr_pixmap);
    pixmap = tmppixmap;
    }
    int width=1280, height=720;
    //cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    GraphicsEngine* engine = init_base_engine(new CairoBuffer(RectSize(1280,720)));

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
        scene.containers.push_back(Container({{ 0,0,1280,720 },1,0,0,0xFFFF0000}));
        scene.assets.push_back(Asset());
    }
    draw_scene(scene, engine);
    auto screen = engine->getScreenBuffer();
    uint8_t* scrmem; uint32_t pitch;
    screen->lock(scrmem, pitch);
    cairo_surface_t *cst = cairo_image_surface_create_for_data(scrmem, CAIRO_FORMAT_ARGB32, width, height, pitch);

    cairo_t *cr_pixmap = gdk_cairo_create(pixmap);
    cairo_set_source_surface (cr_pixmap, cst, 0, 0);
    cairo_paint(cr_pixmap);
    cairo_destroy(cr_pixmap);
    screen->unlock();

    gtk_main();
    gdk_threads_leave();

    return 0;
}
