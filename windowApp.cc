#include"windowApp.h"

// physical data
map<string, DrawRectangle> padMap;
map<string, DrawRectangle> fingerMap;
map<string, DrawRectangle> fingerAvailableMap;
map<string, DrawCircle> ballMap;
map<string, DrawLine> padToFingerMap;
vector<DrawLine> fingerToBallVec;
vector<DrawLine> padToBallVec;
map<string, DrawLine> legalPadToFingerMap;
map<string, DrawRectangle> legalFingerMap;

map<string, map<string, PhysicalObject>> g_entireObjMap;
map<string, DrawContainer> g_temp;

// transform parameters
static double scale = 0.15;
static Point translate(990.0, 540.0);
static double speed = 10.0;
// TEST gtk key press
static bool test = false;
static bool show = true;
// static variables and functions
static cairo_surface_t *surface = NULL;

/**/
static void activate(GtkApplication *app, gpointer user_data);

/**/
static gboolean onDrawEvent(GtkWidget *widget, cairo_t *cr2, gpointer user_data);

/**/
static gboolean keyPressCallback(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

/**/
static gboolean edgeButton_onClick(GtkWidget *widget, gpointer user_data);

/**/
static gboolean checkButton_toggled(GtkToggleButton *toggleButton, gpointer user_data);

void WindowApp::Run(int argc, char *argv[])
{
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    dataTransfer();
    g_application_run(G_APPLICATION(app), 1, argv);
    g_object_set (gtk_settings_get_default(), "gtk-enable-animations", TRUE, NULL);
}

static void activate(GtkApplication *app, gpointer user_data){
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWidget *button;
    GtkWidget *button2;
    GtkWidget *edgeButton;
    GtkWidget *listBox;
    GtkWidget *layout;
    GtkWidget *revealer;
    
    revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_CROSSFADE);
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 10000);
    gtk_widget_set_size_request(revealer, 100, 100);

    button = gtk_button_new_with_label("Next Stage");
    edgeButton = gtk_button_new_with_label("Hide Edge");
    button2 = gtk_button_new_with_label("reveal test");
    gtk_container_add(GTK_CONTAINER(revealer), button2);
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), TRUE);

    // add new window to app
    window = gtk_application_window_new(app);
    // set window title
    gtk_window_set_title(GTK_WINDOW(window), "die rotation");
    // set window default size
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    
    layout = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), layout);
    
    //g_signal_connect(G_OBJECT(button), "draw", G_CALLBACK(onDrawEvent), NULL);
    // create a drawing area
    drawing_area = gtk_drawing_area_new();
    // singal "draw" connect to callback function drawAll
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(onDrawEvent), NULL);
    // singal "key_press_event" connect to callback function keyPressCallBack
    g_signal_connect(G_OBJECT(drawing_area), "key_press_event", G_CALLBACK(keyPressCallback), revealer);
    gtk_widget_set_can_focus(drawing_area, TRUE);
    gtk_widget_set_events (drawing_area, gtk_widget_get_events (drawing_area)
                                     | GDK_KEY_PRESS_MASK);
    //
    gtk_widget_set_size_request(drawing_area, 1920, 1080);
    g_signal_connect(edgeButton, "clicked", G_CALLBACK(edgeButton_onClick), drawing_area);
    //
    listBox = gtk_list_box_new();
    //
    int i = 0;
    for(auto map_it = g_entireObjMap.begin(); map_it != g_entireObjMap.end(); ++map_it){
        GtkWidget *checkButton;
        checkButton = gtk_check_button_new_with_label(map_it->first.c_str());
        gtk_widget_set_name(checkButton, map_it->first.c_str());
        g_signal_connect(checkButton, "toggled", G_CALLBACK(checkButton_toggled), drawing_area);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkButton), TRUE);
        gtk_list_box_prepend(GTK_LIST_BOX(listBox), checkButton);
        ++i;
    }
    //
    gtk_layout_put(GTK_LAYOUT(layout), drawing_area, 100, 0);
    gtk_layout_put(GTK_LAYOUT(layout), button, 0, 0);
    gtk_layout_put(GTK_LAYOUT(layout), edgeButton, 0, 50);
    gtk_layout_put(GTK_LAYOUT(layout), revealer, 0, 100);
    gtk_layout_put(GTK_LAYOUT(layout), listBox, 0, 200);
    //gtk_container_add(GTK_CONTAINER(window), button);
    
    // show all widget in window
    gtk_widget_show_all(window);
}

/**/
static gboolean keyPressCallback(GtkWidget *widget, GdkEventKey *event, gpointer user_data){
    //g_print("%s\n", gtk_widget_get_name(widget));
    GtkWidget* revealer = GTK_WIDGET(user_data);

    if(event->keyval == GDK_KEY_n){
       test = !test;
       gtk_revealer_set_reveal_child(GTK_REVEALER(revealer), !gtk_revealer_get_reveal_child(GTK_REVEALER(revealer)));
    }
    else if(event->keyval == GDK_KEY_Left){
        translate.x -= speed;
    }
    else if(event->keyval == GDK_KEY_Right){
        translate.x += speed;
    }
    else if(event->keyval == GDK_KEY_Up){
        translate.y -= speed;
    }
    else if(event->keyval == GDK_KEY_Down){
        translate.y += speed;
    }
    else if(event->keyval == GDK_KEY_KP_Subtract){
        scale -= (abs(scale) + 1.0) * 0.01;    
    }
    else if(event->keyval == GDK_KEY_KP_Add){
        scale += (abs(scale) + 1.0) * 0.01;    
    }
    else if(event->keyval == GDK_KEY_KP_Decimal){
        g_print("output_png\n");
        cairo_surface_write_to_png(surface, "output.png");
    }
    gtk_widget_queue_draw(widget);
    return TRUE;
}

/**/
static gboolean onDrawEvent(GtkWidget *widget, cairo_t *cr2, gpointer user_data){
    /*
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40.0);
    cairo_move_to(cr, 10.0, 50.0);
    cairo_show_text(cr, "Disziplin ist Macht.");
    */
    if(surface)
        cairo_surface_destroy(surface);

    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
            CAIRO_CONTENT_COLOR,
            gtk_widget_get_allocated_width(widget),
            gtk_widget_get_allocated_height(widget));

    cairo_t *cr;
    cr = cairo_create(surface);
    cairo_translate(cr, translate.x, translate.y);
    cairo_scale(cr, scale, scale);
    cairo_set_line_width(cr, 2.0 / scale);
    
    //g_print("Draw: %s\n", gtk_widget_get_name(widget));
    //DrawRectangle IC(Point(), Color(1.0, 1.0, 0.0), 5000.0, 5000.0, cairo_stroke);
    //IC.Draw(cr);
    
    //for(map<string, DrawCircle>::iterator map_it = ballMap.begin(); map_it != ballMap.end(); ++map_it)
      //  map_it->second.Draw(cr);

    //for(map<string, DrawRectangle>::iterator map_it = padMap.begin(); map_it != padMap.end(); ++map_it)
      //  map_it->second.Draw(cr);

    for(map<string, map<string, PhysicalObject>>::iterator map_it = g_entireObjMap.begin(); map_it != g_entireObjMap.end(); ++map_it){
        if(map_it->first != "padToBallMap" || show)
            if((map_it->first != "fingerMap" && map_it->first != "padToFingerMap") || !test)
                if((map_it->first != "legalFingerMap" && map_it->first != "legalPadToFingerMap") || test)
            for(map<string, PhysicalObject>::iterator map_it2 = map_it->second.begin(); map_it2 != map_it->second.end(); ++map_it2){
                map_it2->second.drawContainer.Draw(cr);
            }
        
    }
        
    //for(map<string, DrawRectangle>::iterator map_it = fingerAvailableMap.begin(); map_it != fingerAvailableMap.end(); ++map_it)
       // map_it->second.Draw(cr);
    if(!test){
//        for(map<string, DrawRectangle>::iterator map_it = fingerMap.begin(); map_it != fingerMap.end(); ++map_it)
  //          map_it->second.Draw(cr);

//        for(map<string, DrawLine>::iterator map_it = padToFingerMap.begin(); map_it != padToFingerMap.end(); ++map_it)
  //          map_it->second.Draw(cr);
    }
    else{
    //    for(map<string, DrawRectangle>::iterator map_it = legalFingerMap.begin(); map_it != legalFingerMap.end(); ++map_it)
      //      map_it->second.Draw(cr);

        //for(map<string, DrawLine>::iterator map_it = legalPadToFingerMap.begin(); map_it != legalPadToFingerMap.end(); ++map_it)
          //  map_it->second.Draw(cr);
    }

    for(size_t i = 0; i < fingerToBallVec.size(); ++i){
        fingerToBallVec[i].Draw(cr);
    }


    // testing c++ polymorphism
    /*
    vector<DrawObject*> drawContainer;
    
    drawContainer.push_back(new DrawRectangle(Point(100, 100), Color(), 4000.0, 4000.0, cairo_fill));
    drawContainer.push_back(new DrawCircle(Point(4000, 100), Color(), 300, cairo_fill));
    for(int i = 0; i < drawContainer.size(); ++i)
        drawContainer[i]->Draw(cr);
    for(int i = 0; i < drawContainer.size(); ++i)
        delete drawContainer[i];
    */

    // testing 'drawContainer'
    /*
    DrawContainer drawCon;
    for(map<string, DrawRectangle>::iterator map_it = padMap.begin(); map_it != padMap.end(); ++map_it){
        drawCon.push_back(&(map_it->second));
    }
    for(map<string, DrawCircle>::iterator map_it = ballMap.begin(); map_it != ballMap.end(); ++map_it){
        drawCon.push_back(&(map_it->second));
    }
    DrawContainer all;
    all.push_back(&drawCon);
    cairo_matrix_scale(&drawCon.transformMat, 0.5, 0.5);
    DrawContainer drawCon2(drawCon);
    cairo_matrix_t matrix;
    cairo_matrix_init_identity(&matrix);
    */

//    cout << matrix.x0 << " " << matrix.y0 << endl;

   // cairo_matrix_translate(&matrix, 0.0, 0.0);
  //  cairo_matrix_scale(&matrix, 1.5, 2);

  //  cout << matrix.x0 << " " << matrix.y0 << endl;


    /*
    cairo_transform(cr, &matrix);
    all.Draw(cr);
    // test Clone function
    DrawCircle test(Point(), Color(1.0, 1.0, 1.0, 0.5), 100.0, cairo_fill);
    DrawObject* basePtr = &test, *basePtr2;
    basePtr2 = basePtr->Clone();
    basePtr2->setCenterPoint(Point(300.0, 400.0));
    basePtr2->setColor(Color(1.0, 0.0, 0.0));
    basePtr->Draw(cr);
    basePtr2->Draw(cr);
    delete basePtr2;
    */
    cairo_destroy(cr);
    cairo_set_source_surface(cr2, surface, 0, 0);
    cairo_paint(cr2);
    if(gtk_widget_has_focus(widget) == TRUE){
        //g_print("has focus\n");
        DrawRectangle focus(Point(0, 0), Color(1.0, 1.0, 0.3), 100, 100, cairo_stroke);
        focus.Draw(cr2);
    }

    return FALSE;
}


static gboolean edgeButton_onClick(GtkWidget *widget, gpointer user_data){
    show = !show; 
    if(show)
        gtk_button_set_label(GTK_BUTTON(widget), "hide");
    else
        gtk_button_set_label(GTK_BUTTON(widget), "show");
    
    gtk_widget_queue_draw(GTK_WIDGET(user_data));

    return TRUE;
}


static gboolean checkButton_toggled(GtkToggleButton *toggleButton, gpointer user_data){
    g_print("%s\n", gtk_widget_get_name(GTK_WIDGET(toggleButton)));
    string checkButtonName = gtk_widget_get_name(GTK_WIDGET(toggleButton));

    for(auto map_it = g_entireObjMap.begin(); map_it != g_entireObjMap.end(); ++map_it){
        if(checkButtonName == map_it->first){
            gboolean state = gtk_toggle_button_get_active(toggleButton);
            for(auto map_it2 = map_it->second.begin(); map_it2 != map_it->second.end(); ++map_it2){
                if(state == TRUE){
                    map_it2->second.drawContainer.setIsVisible(true);
                }
                else if(state == FALSE){
                    map_it2->second.drawContainer.setIsVisible(false);
                }
            }
        }
    }
    
    gtk_widget_queue_draw(GTK_WIDGET(user_data));
    return TRUE;
}