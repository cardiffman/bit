/*
 * gtkbit.cpp
 *
 *  Created on: Dec 25, 2018
 *      Author: menright
 */
#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>

static void hello(GtkWidget* w)
{
	g_print("Hello world\n");
}
static gboolean delete_event(GtkWidget* w, GdkEvent* e, gpointer data)
{
	return FALSE;
}
static void destroy(GtkWidget* w, gpointer data)
{
	gtk_main_quit();
}
namespace GTKProject {
struct Rectangle {
	unsigned x;
	unsigned y;
	unsigned width;
	unsigned height;
};
struct RectSize {
	unsigned width;
	unsigned height;
};
struct Asset {
	unsigned id;
	RectSize size;
	GdkPixbuf* data;
};
struct Container {
	unsigned id;
	unsigned assetId;
	unsigned parentId;
	Rectangle area;
	unsigned color;
	unsigned properties;
};
Container containers[] = {
		{ 0 },
		{ 1, 1, 12, { 0*145, 0, 145, 192 }, 0, 0 },
		{ 2, 2, 13, { 1*145, 0, 145, 192 }, 0, 0 },
		{ 3, 3, 13, { 2*145, 0, 145, 192 }, 0, 0 },
		{ 4, 4, 13, { 3*145, 0, 145, 192 }, 0, 0 },
		{ 5, 5, 13, { 4*145, 0, 145, 192 }, 0, 0 },
		{ 6, 6, 13, { 5*145, 0, 145, 192 }, 0, 0 },
		{ 7, 7, 13, { 6*145, 0, 145, 192 }, 0, 0 },
		{ 8, 8, 13, { 7*145, 0, 145, 192 }, 0, 0 },
		{ 9, 9, 13, { 8*145, 0, 145, 192 }, 0, 0 },
		{ 10, 10, 13, { 9*145, 0, 145, 192 }, 0, 0 },
		{ 11, 11, 13, { 10*145, 0, 145, 192 }, 0, 0 },
		{ 12, 0, 13, { 0*145, 0, 145, 192 }, 0, 0 },
		{ 13, 0, 0, { 0, 340, 11*145, 192 }, 0, 0 },
};
const int numContainers = sizeof(containers)/sizeof(containers[0]);
Asset assets[] = {
		{ 0},
		{ 1, { 127, 192 }, 0 },
		{ 2, { 127, 192 }, 0 },
		{ 3, { 127, 192 }, 0 },
		{ 4, { 127, 192 }, 0 },
		{ 5, { 127, 192 }, 0 },
		{ 6, { 127, 192 }, 0 },
		{ 7, { 127, 192 }, 0 },
		{ 8, { 127, 192 }, 0 },
		{ 9, { 127, 192 }, 0 },
		{ 10, { 127, 192 }, 0 },
		{ 11, { 127, 192 }, 0 },
		{ 12, { 127, 192 }, 0 },
		{ 13, { 127, 192 }, 0 },
		{ 14, { 127, 192 }, 0 },
};
const int numAssets = sizeof(assets)/sizeof(assets[0]);
bool loadImages()
{
	const char* files[] = {
			"image.png",
		"AQAMN_VERT_MAIN_DUO_DOM_2764x4096_master.jpg",
		"SecondAct_27x40_1Sheet_RGB.jpg",
		"Vice2018.jpg",
		"BB_Online_Dom_Payoff_1-Sheet_H-Steinfeld_BB_Bridge_Autobot.jpg",
		"MPR-Payoff_1-Sheet_v8a_Sm.jpg",
		"SpiderManIntoTheSpiderVerse2018.jpg",
		"WTM_HeroPoster.jpg",
		"GRC_Tsr1Sheet_GrinchAndMax_.jpg",
		"MQOS_OneSheet.jpg",
		"TheFavourite2018.jpg",
		"HolmesAndWatson2018.jpg",
		"MULE_VERT_MAIN_DOM_2764x4096_master.jpg",
		"TSNGO_TicketingBanner_250x375_r2.jpg",
	};
	const int numFiles = sizeof(files)/sizeof(files[0]);
	for (unsigned i=0; i<numFiles; ++i)
	{
		GError* err = NULL;
		auto image = gdk_pixbuf_new_from_file(files[i], &err);
		if (!image)
		{
		    g_printerr("%s\n", err->message);
		    g_error_free(err);
		    return false;
		}
		assets[i+1].data = image;
		assets[i+1].size.width = gdk_pixbuf_get_height(image);
		assets[i+1].size.height = gdk_pixbuf_get_width(image);
	}
	return true;
}
GtkWidget* renderContainer(Container& container)
{
	printf("Rendering container %u\n", container.id);
	if (container.assetId)
	{
		auto widget = gtk_image_new_from_pixbuf(assets[container.assetId].data);
		if (!widget)
			return nullptr;
		gtk_widget_show(widget);
		return widget;
	}
	GtkFixed* layout = GTK_FIXED(gtk_fixed_new());
	if (!layout)
		return nullptr;
	for (unsigned i=1; i<numContainers; ++i)
	{
		if (containers[i].parentId != container.id)
			continue;
		auto child = renderContainer(containers[i]);
		if (!child)
			return nullptr;
		gtk_fixed_put(layout, child, containers[i].area.x, containers[i].area.y);
	}
	gtk_widget_show(GTK_WIDGET(layout));
	return GTK_WIDGET(layout);
}
void renderContainers(GtkWidget* window)
{
	GtkFixed* layout = GTK_FIXED(gtk_fixed_new());
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET(layout));
	for (unsigned i=1; i<numContainers; ++i)
	{
		if (containers[i].parentId)
			continue;
		auto child = renderContainer(containers[i]);
		if (!child)
			return;
		gtk_fixed_put(layout, child, containers[i].area.x, containers[i].area.y);
	}
	gtk_widget_show(GTK_WIDGET(layout));
}
}
int main(int argc, char** argv)
{
	/* GtkWidget is the storage type for widgets */
	GtkWidget *window;

	/* This is called in all GTK applications. Arguments are parsed
	 * from the command line and are returned to the application. */
	gtk_init(&argc, &argv);

	/* create a new window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);

	GTKProject::renderContainers(GTK_WIDGET(window));

	gtk_widget_show_all(window);
	/* When the window is given the "delete-event" signal (this is given
	 * by the window manager, usually by the "close" option, or on the
	 * titlebar), we ask it to call the delete_event () function
	 * as defined above. The data passed to the callback
	 * function is NULL and is ignored in the callback function. */
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

	/* Here we connect the "destroy" event to a signal handler.
	 * This event occurs when we call gtk_widget_destroy() on the window,
	 * or if we return FALSE in the "delete-event" callback. */
	g_signal_connect(window, "destroy", G_CALLBACK(destroy), NULL);

	/* Sets the border width of the window. */
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);

	/* All GTK applications must have a gtk_main(). Control ends here
	 * and waits for an event to occur (like a key press or
	 * mouse event). */
	gtk_main();
	return 0;
}


