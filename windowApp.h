#ifndef WINDOWAPP_H
#define WINDOWAPP_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include"kernel.h"
#include"gtk_cairo.h"

class WindowApp
{
private:
    GtkApplication *app;
public:
    WindowApp() { app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE); }
    WindowApp(std::string appName) { app = gtk_application_new (appName.c_str(), G_APPLICATION_FLAGS_NONE); }
    ~WindowApp() { g_object_unref(app); }
    void Run(int argc, char* argv[]);
protected:
};

#endif  // WINDOWAPP_H