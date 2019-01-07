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
#include "bitbuffer.h"
#include "scene.h"
#include "scene_builder.h"
#include <iostream>

#include <signal.h>

using std::cout;
using std::endl;

//the global pixmap that will serve as our buffer
static GdkPixmap *pixmap = NULL;
Scene scene;
BitBuffer screen;

#if 0
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


static int currently_drawing = 0;
//do_draw will be executed in a separate thread whenever we would like to update
//our animation
void *do_draw(void *ptr){

    //prepare to trap our SIGALRM so we can draw when we recieve it!
    siginfo_t info;
    sigset_t sigset;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);

    while(1){
        //wait for our SIGALRM.  Upon receipt, draw our stuff.  Then, do it again!
        while (sigwaitinfo(&sigset, &info) > 0) {

            currently_drawing = 1;

            int width, height;
            gdk_threads_enter();
            gdk_drawable_get_size(pixmap, &width, &height);
            gdk_threads_leave();

#if 1
            draw_scene(scene,screen);
            void fill(uint32_t color
            		, BitBuffer& dst
            		, const Area& dstArea);
            //fill(0xFF00FF00, buf, Area(10,10, 1260, 710));
            cairo_surface_t *cst = cairo_image_surface_create_for_data(screen.mem, CAIRO_FORMAT_ARGB32, width, height, screen.rowbytes);

#else
            //create a gtk-independant surface to draw on
            cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
            //cairo_t *cr = cairo_create(cst);

            BitBuffer buf;

            buf.mem = cairo_image_surface_get_data (cst);

            //cairo_public cairo_format_t
            //cairo_image_surface_get_format (cairo_surface_t *surface);
            //gdk_drawable_get_data(pixmap, "");
            //GdkImage* gdk_drawable_get_image(pixmap, 0, 0, 1280, 720);

            buf.dims.width = cairo_image_surface_get_width (cst);

            buf.dims.height = cairo_image_surface_get_height (cst);

            buf.rowbytes = cairo_image_surface_get_stride (cst);
            cout << "width " << buf.dims.width << " rowbytes " << buf.rowbytes << endl;

            Scene scene;
            scene.containers = {{0},{{ 0,0,1280,720 },1,0,0,0xFFFF0000}};
            scene.assets = {{0}};
            draw_scene(scene,buf);
            cairo_surface_mark_dirty(cst);
            //do some time-consuming drawing
            //static int i = 0;
            //++i; i = i % 300;   //give a little movement to our animation
            //cairo_set_source_rgb (cr, .9, .9, .9);
            //cairo_paint(cr);
            //int j,k;
            //for(k=0; k<100; ++k){   //lets just redraw lots of times to use a lot of proc power
            //    for(j=0; j < 1000; ++j){
            //        cairo_set_source_rgb (cr, (double)j/1000.0, (double)j/1000.0, 1.0 - (double)j/1000.0);
            //        cairo_move_to(cr, i,j/2);
            //        cairo_line_to(cr, i+100,j/2);
            //        cairo_stroke(cr);
            //    }
            //}
            //cairo_destroy(cr);


            //When dealing with gdkPixmap's, we need to make sure not to
            //access them from outside gtk_main().
#endif
            gdk_threads_enter();

            cairo_t *cr_pixmap = gdk_cairo_create(pixmap);
            cairo_set_source_surface (cr_pixmap, cst, 0, 0);
            cairo_paint(cr_pixmap);
            cairo_destroy(cr_pixmap);

            gdk_threads_leave();

            cairo_surface_destroy(cst);

            currently_drawing = 0;

        }
    }
}

gboolean timer_exe(GtkWidget * window){
    static int first_time = 0;//1;
    //use a safe function to get the value of currently_drawing so
    //we don't run into the usual multithreading issues
    int drawing_status = 1;//g_atomic_int_get(&currently_drawing);

    //if this is the first time, create the drawing thread
    static pthread_t thread_info;
    if(first_time == 1){
        int  iret;
        iret = pthread_create( &thread_info, NULL, do_draw, NULL);
    }

    //if we are not currently drawing anything, send a SIGALRM signal
    //to our thread and tell it to update our pixmap
    if(drawing_status == 0){
        pthread_kill(thread_info, SIGALRM);
    }

    //tell our window it is time to draw our animation.
    int width, height;
    gdk_drawable_get_size(pixmap, &width, &height);
    gtk_widget_queue_draw_area(window, 0, 0, width, height);


    first_time = 0;
    return TRUE;

}


int main (int argc, char *argv[]){

    //Block SIGALRM in the main thread
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    //we need to initialize all these functions so that gtk knows
    //to be thread-aware
    //if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    int width = 1280;
    int height = 720;
    screen.rowbytes = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    screen.dims.width = width;
    screen.dims.height = height;
    screen.mem = (uint8_t*)malloc(screen.rowbytes * screen.dims.height);
    memset(screen.mem, 0, screen.rowbytes * screen.dims.height);
#if 1
	SceneBuilder builder;
	builder.parse_containers(argv[1]);
	scene.containers = builder.nc;
	scene.assets = builder.na;
#else
    scene.containers.push_back(Container());
    scene.containers.push_back(Container({{ 0,0,1280,720 },1,0,0,0xFFFF0000}));
    scene.assets = {{0}};
#endif

    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 1280, 720);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "expose_event", G_CALLBACK(on_window_expose_event), NULL);
    g_signal_connect(G_OBJECT(window), "configure_event", G_CALLBACK(on_window_configure_event), NULL);

    //this must be done before we define our pixmap so that it can reference
    //the colour depth and such
    gtk_widget_show_all(window);

    //set up our pixmap so it is ready for drawing
    pixmap = gdk_pixmap_new(window->window,1280,720,-1);
    //because we will be painting our pixmap manually during expose events
    //we can turn off gtk's automatic painting and double buffering routines.
    gtk_widget_set_app_paintable(window, TRUE);
    gtk_widget_set_double_buffered(window, FALSE);

    (void)g_timeout_add(33, (GSourceFunc)timer_exe, window);
    do_draw(NULL);
    gtk_widget_queue_draw_area(window, 0, 0, width, height);


    gtk_main();
    gdk_threads_leave();

    return 0;
}
#else
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

    screen.rowbytes = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    screen.dims.width = width;
    screen.dims.height = height;
    screen.mem = (uint8_t*)malloc(screen.rowbytes * screen.dims.height);
    memset(screen.mem, 0, screen.rowbytes * screen.dims.height);

#if 1
	SceneBuilder builder;
    try {
    	builder.parse_containers(argv[1]);
    } catch (char const * ex) {
    	cout << "Exception " << ex << endl;
    	return 1;
    } catch (const std::string& ex) {
    	cout << "Exception " << ex << endl;
    	return 1;
    }
	scene.containers = builder.nc;
	scene.assets = builder.na;

#else
    scene.containers.push_back(Container());
    scene.containers.push_back(Container({{ 0,0,1280,720 },1,0,0,0xFFFF0000}));
    scene.assets = {{0}};
#endif
    draw_scene(scene,screen);
    cairo_surface_t *cst = cairo_image_surface_create_for_data(screen.mem, CAIRO_FORMAT_ARGB32, width, height, screen.rowbytes);

    cairo_t *cr_pixmap = gdk_cairo_create(pixmap);
    cairo_set_source_surface (cr_pixmap, cst, 0, 0);
    cairo_paint(cr_pixmap);
    cairo_destroy(cr_pixmap);

    gtk_main();
    gdk_threads_leave();

    return 0;
}
#endif
