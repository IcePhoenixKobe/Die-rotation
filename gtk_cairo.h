#ifndef GTK_CAIRO_H
#define GTK_CAIRO_H

#include<gtkmm.h>
using namespace Gtk;

class WindowApp
{
private:
    GtkApplication *app;
public:
    WindowApp() { app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE); }
    WindowApp(std::string appName) { app = gtk_application_new (appName.c_str(), G_APPLICATION_FLAGS_NONE); }
    ~WindowApp() { g_object_unref(app); }
protected:
    
};

#endif