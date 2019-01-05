/*
 * glgtkbit.cpp
 *
 *  Created on: Dec 25, 2018
 *      Author: menright
 */
#if 1
#include <gtk/gtk.h>
#include <glib.h>
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
    static int first_time = 1;
    //use a safe function to get the value of currently_drawing so
    //we don't run into the usual multithreading issues
    int drawing_status = g_atomic_int_get(&currently_drawing);

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


    gtk_main();
    gdk_threads_leave();

    return 0;
}
#else
/*
 * 3-D gear wheels.  This program is in the public domain.
 *
 * Brian Paul
 */

/* Conversion to GLUT by Mark J. Kilgard */

/* Conversion to GtkGLExt by Naofumi Yasufuku */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <gtk/gtkgl.h>
#include <gtk/gtkglwidget.h>
#include <gdk/gdkglconfig.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifdef GDK_WINDOWING_QUARTZ
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

/*
 * Draw a gear wheel.  You'll probably want to call this function when
 * building a display list since we do a lot of trig here.
 *
 * Input:  inner_radius - radius of hole at center
 * outer_radius - radius at center of teeth
 * width - width of gear
 * teeth - number of teeth
 * tooth_depth - depth of tooth
 */

static void
gear(GLfloat inner_radius,
     GLfloat outer_radius,
     GLfloat width,
     GLint   teeth,
     GLfloat tooth_depth)
{
  GLint i;
  GLfloat r0, r1, r2;
  GLfloat angle, da;
  GLfloat u, v, len;

  r0 = inner_radius;
  r1 = outer_radius - tooth_depth / 2.0;
  r2 = outer_radius + tooth_depth / 2.0;

  da = 2.0 * G_PI / teeth / 4.0;

  glShadeModel(GL_FLAT);

  glNormal3f(0.0, 0.0, 1.0);

  /* draw front face */
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * G_PI / teeth;
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    if (i < teeth) {
      glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
      glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    }
  }
  glEnd();

  /* draw front sides of teeth */
  glBegin(GL_QUADS);
  da = 2.0 * G_PI / teeth / 4.0;
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * G_PI / teeth;

    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
  }
  glEnd();

  glNormal3f(0.0, 0.0, -1.0);

  /* draw back face */
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * G_PI / teeth;
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    if (i < teeth) {
      glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
      glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    }
  }
  glEnd();

  /* draw back sides of teeth */
  glBegin(GL_QUADS);
  da = 2.0 * G_PI / teeth / 4.0;
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * G_PI / teeth;

    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
  }
  glEnd();

  /* draw outward faces of teeth */
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i < teeth; i++) {
    angle = i * 2.0 * G_PI / teeth;

    glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
    glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    u = r2 * cos(angle + da) - r1 * cos(angle);
    v = r2 * sin(angle + da) - r1 * sin(angle);
    len = sqrt(u * u + v * v);
    u /= len;
    v /= len;
    glNormal3f(v, -u, 0.0);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
    glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
    glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
    u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
    v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
    glNormal3f(v, -u, 0.0);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
    glNormal3f(cos(angle), sin(angle), 0.0);
  }

  glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
  glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

  glEnd();

  glShadeModel(GL_SMOOTH);

  /* draw inside radius cylinder */
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= teeth; i++) {
    angle = i * 2.0 * G_PI / teeth;
    glNormal3f(-cos(angle), -sin(angle), 0.0);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
  }
  glEnd();

}

static GLfloat view_rotx = 20.0, view_roty = 30.0, view_rotz = 0.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

static GTimer *timer = NULL;
static gint frames = 0;

static gboolean is_sync = TRUE;

static gboolean
draw (GtkWidget      *widget,
      GdkEventExpose *event,
      gpointer        data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix ();
    glRotatef (view_rotx, 1.0, 0.0, 0.0);
    glRotatef (view_roty, 0.0, 1.0, 0.0);
    glRotatef (view_rotz, 0.0, 0.0, 1.0);

    glPushMatrix ();
      glTranslatef (-3.0, -2.0, 0.0);
      glRotatef (angle, 0.0, 0.0, 1.0);
      glCallList (gear1);
    glPopMatrix ();

    glPushMatrix ();
      glTranslatef (3.1, -2.0, 0.0);
      glRotatef (-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
      glCallList (gear2);
    glPopMatrix ();

    glPushMatrix ();
      glTranslatef (-3.1, 4.2, 0.0);
      glRotatef (-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
      glCallList (gear3);
    glPopMatrix ();

  glPopMatrix ();

  if (gdk_gl_drawable_is_double_buffered (gldrawable))
    gdk_gl_drawable_swap_buffers (gldrawable);
  else
    glFlush ();

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  frames++;

  {
    gdouble seconds = g_timer_elapsed (timer, NULL);
    if (seconds >= 5.0) {
      gdouble fps = frames / seconds;
      g_print ("%d frames in %6.3f seconds = %6.3f FPS\n", frames, seconds, fps);
      g_timer_reset (timer);
      frames = 0;
    }
  }

  return TRUE;
}

/* new window size or exposure */
static gboolean
reshape (GtkWidget         *widget,
	 GdkEventConfigure *event,
	 gpointer           data)
{
  GtkAllocation allocation;
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  GLfloat h;

  gtk_widget_get_allocation (widget, &allocation);
  h = (GLfloat) (allocation.height) / (GLfloat) (allocation.width);

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return FALSE;

  glViewport (0, 0, allocation.width, allocation.height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glFrustum (-1.0, 1.0, -h, h, 5.0, 60.0);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (0.0, 0.0, -40.0);

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  return TRUE;
}

static void
init(GtkWidget *widget,
     gpointer   data)
{
  GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
  GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

  static GLfloat pos[4] = {5.0, 5.0, 10.0, 0.0};
  static GLfloat red[4] = {0.8, 0.1, 0.0, 1.0};
  static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0};
  static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};

  /*** OpenGL BEGIN ***/
  if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
    return;

  glLightfv (GL_LIGHT0, GL_POSITION, pos);
  glEnable (GL_CULL_FACE);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_DEPTH_TEST);

  /* make the gears */
  gear1 = glGenLists (1);
  glNewList (gear1, GL_COMPILE);
    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    gear (1.0, 4.0, 1.0, 20, 0.7);
  glEndList ();

  gear2 = glGenLists (1);
  glNewList (gear2, GL_COMPILE);
    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
    gear (0.5, 2.0, 2.0, 10, 0.7);
  glEndList ();

  gear3 = glGenLists (1);
  glNewList (gear3, GL_COMPILE);
    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    gear (1.3, 2.0, 0.5, 10, 0.7);
  glEndList ();

  glEnable (GL_NORMALIZE);

  g_print ("\n");
  g_print ("GL_RENDERER   = %s\n", (char *) glGetString (GL_RENDERER));
  g_print ("GL_VERSION    = %s\n", (char *) glGetString (GL_VERSION));
  g_print ("GL_VENDOR     = %s\n", (char *) glGetString (GL_VENDOR));
  g_print ("GL_EXTENSIONS = %s\n", (char *) glGetString (GL_EXTENSIONS));
  g_print ("\n");

  gdk_gl_drawable_gl_end (gldrawable);
  /*** OpenGL END ***/

  /* create timer */
  if (timer == NULL)
    timer = g_timer_new ();

  g_timer_start (timer);
}

static gboolean
idle (GtkWidget *widget)
{
  GtkAllocation allocation;
  GdkWindow *window;
  angle += 2.0;

  window = gtk_widget_get_window (widget);
  gtk_widget_get_allocation (widget, &allocation);

  /* Invalidate the whole window. */
  gdk_window_invalidate_rect (window, &allocation, FALSE);

  /* Update synchronously (fast). */
  if (is_sync)
    gdk_window_process_updates (window, FALSE);

  return TRUE;
}

static guint idle_id = 0;

static void idle_add(GtkWidget *widget)
{
	if (idle_id == 0)
	{
		idle_id = g_idle_add_full(GDK_PRIORITY_REDRAW, (GSourceFunc) idle,
				widget,
				NULL);
	}
}

static void idle_remove(GtkWidget *widget)
{
	if (idle_id != 0)
	{
		g_source_remove(idle_id);
		idle_id = 0;
	}
}

static gboolean
map (GtkWidget   *widget,
     GdkEventAny *event,
     gpointer     data)
{
  idle_add (widget);

  return TRUE;
}

static gboolean
unmap (GtkWidget   *widget,
       GdkEventAny *event,
       gpointer     data)
{
  idle_remove (widget);

  return TRUE;
}

static gboolean
visible (GtkWidget          *widget,
	 GdkEventVisibility *event,
	 gpointer            data)
{
  if (event->state == GDK_VISIBILITY_FULLY_OBSCURED)
    idle_remove (widget);
  else
    idle_add (widget);

  return TRUE;
}

/* change view angle, exit upon ESC */
static gboolean
key (GtkWidget   *widget,
     GdkEventKey *event,
     gpointer     data)
{
  GtkAllocation allocation;

	switch (event->keyval)
	{
	case GDK_z:
		view_rotz += 5.0;
		break;
	case GDK_Z:
		view_rotz -= 5.0;
		break;
	case GDK_Up:
		view_rotx += 5.0;
		break;
	case GDK_Down:
		view_rotx -= 5.0;
		break;
	case GDK_Left:
		view_roty += 5.0;
		break;
	case GDK_Right:
		view_roty -= 5.0;
		break;
	case GDK_Escape:
		gtk_main_quit();
		break;
	default:
		return FALSE;
	}

  gtk_widget_get_allocation (widget, &allocation);
  gdk_window_invalidate_rect (gtk_widget_get_window (widget), &allocation, FALSE);

  return TRUE;
}

int
main (int   argc,
      char *argv[])
{
  GdkGLConfig *glconfig;
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *drawing_area;
  GtkWidget *button;
  int i;

  /*
   * Init GTK.
   */

  gtk_init (&argc, &argv);

  /*
   * Init GtkGLExt.
   */

  gtk_gl_init (&argc, &argv);

  /*
   * Command line options.
   */

  for (i = 0; i < argc; i++)
    {
      if (strcmp (argv[i], "--async") == 0)
        is_sync = FALSE;
    }

  /*
   * Configure OpenGL-capable visual.
   */

  /* Try double-buffered visual */
  glconfig = gdk_gl_config_new_by_mode ((GdkGLConfigMode)(GDK_GL_MODE_RGB    |
					GDK_GL_MODE_DEPTH  |
					GDK_GL_MODE_DOUBLE));
	if (glconfig == NULL)
	{
		g_print("*** Cannot find the double-buffered visual.\n");
		g_print("*** Trying single-buffered visual.\n");

		/* Try single-buffered visual */
		glconfig = gdk_gl_config_new_by_mode(
				(GdkGLConfigMode) (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH));
		if (glconfig == NULL)
		{
			g_print("*** No appropriate OpenGL-capable visual found.\n");
			exit(1);
		}
	}

  /*
   * Top-level window.
   */

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "gears");

  /* Get automatically redrawn if any of their children changed allocation. */
  gtk_container_set_reallocate_redraws (GTK_CONTAINER (window), TRUE);

  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_main_quit), NULL);


  /*
   * Drawing area for drawing OpenGL scene.
   */

  drawing_area = gtk_drawing_area_new ();
  gtk_widget_set_size_request (drawing_area, 300, 300);

  /* Set OpenGL-capability to the widget. */
  gtk_widget_set_gl_capability (drawing_area,
				glconfig,
				NULL,
				TRUE,
				GDK_GL_RGBA_TYPE);

  gtk_widget_add_events (drawing_area,
			 GDK_VISIBILITY_NOTIFY_MASK);

  g_signal_connect_after (G_OBJECT (drawing_area), "realize",
                          G_CALLBACK (init), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "configure_event",
		    G_CALLBACK (reshape), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "expose_event",
		    G_CALLBACK (draw), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "map_event",
		    G_CALLBACK (map), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "unmap_event",
		    G_CALLBACK (unmap), NULL);
  g_signal_connect (G_OBJECT (drawing_area), "visibility_notify_event",
		    G_CALLBACK (visible), NULL);

  g_signal_connect_swapped (G_OBJECT (window), "key_press_event",
			    G_CALLBACK (key), drawing_area);

  gtk_widget_show (drawing_area);
  gtk_container_add(GTK_CONTAINER(window), drawing_area);

  /*
   * Show window.
   */

  gtk_widget_show (window);

  /*
   * Main loop.
   */
//idle_add (drawing_area);
  gtk_main ();

  return 0;
}

#endif
