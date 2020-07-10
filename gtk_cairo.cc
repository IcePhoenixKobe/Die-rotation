#include"gtk_cairo.h"

/*
    Point class defination
*/
Point operator+(const Point& p1, const Point& p2){
    return Point(p1.x + p2.x, p1.y + p2.y);
}

Point operator-(const Point& p1, const Point& p2){
    return Point(p1.x - p2.x, p1.y - p2.y);
}

Point operator/(const Point& p1, double scaler){
    if(scaler != 0.0)
        return Point(p1.x / scaler, p1.y / scaler);
    else
        return p1;
}

/*
    DrawObject class defination
*/
// assignment
DrawObject& DrawObject::operator=(const DrawObject& assign_from_me){
    //cout << "DrawObject assignment\n";
    // self-assignment check
    if(this == &assign_from_me)
        return *this;
    
    // assign member of class from 'assign_from_me'
    this->center = assign_from_me.center;
    this->color = assign_from_me.color;
    this->drawMethod = assign_from_me.drawMethod;
    this->transformMat = assign_from_me.transformMat;
    
    return *this;
}

/* Other function */
void DrawObject::translate(double x, double y){
    cairo_matrix_translate(&transformMat, x, y);
}

void DrawObject::rotate(double radians){
    cairo_matrix_translate(&transformMat, center.x, center.y);
    cairo_matrix_rotate(&transformMat, radians);
    cairo_matrix_translate(&transformMat, -center.x, -center.y);
}

void DrawObject::scale(double sx, double sy){
    cairo_matrix_translate(&transformMat, center.x, center.y);
    cairo_matrix_scale(&transformMat, sx, sy);
    cairo_matrix_translate(&transformMat, -center.x, -center.y);
}

/*
    DrawLine class defination
*/
DrawLine& DrawLine::operator=(const DrawLine& assign_from_me){
    //cout << "DrawLine assignment\n";
    // self-assignment check
    if(this == &assign_from_me)
        return *this;

    // assign member of class from 'assign_from_me'
    this->DrawObject::operator= (assign_from_me);
    this->end = assign_from_me.end;

    return *this;
}

void DrawLine::Draw(cairo_t *cr){
    if(!isVisible)
        return;
    cairo_save(cr);
    cairo_transform(cr, &transformMat);
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    cairo_move_to(cr, center.x, center.y);
    cairo_line_to(cr, end.x, end.y);
    cairo_stroke(cr);
    cairo_restore(cr);
}

/*
    DrawRectangle class defination
*/
DrawRectangle& DrawRectangle::operator=(const DrawRectangle& assign_from_me){
    //cout << " DrawRectangle assignment\n";
    // self-assignment check
    if(this == &assign_from_me){
        return *this;
    }
    // assign member of class from 'assign_from_me'
    this->DrawObject::operator= (assign_from_me);
    this->width = assign_from_me.width;
    this->height = assign_from_me.height;

    return *this;
}

void DrawRectangle::Draw(cairo_t *cr){
    if(!isVisible)
        return;
    Point topLeft(center.x - width / 2.0, center.y - height / 2.0);
    cairo_save(cr);
    cairo_transform(cr, &transformMat);
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    cairo_rectangle(cr, topLeft.x, topLeft.y, width, height);
    drawMethod(cr);
    cairo_restore(cr);
}

/*
    DrawCircle class defination
*/
DrawCircle& DrawCircle::operator=(const DrawCircle& assign_from_me){
    //cout << " DrawCircle assignment\n";
    // self-assignment check
    if(this == &assign_from_me){
        return *this;
    }
    // assign member of class from 'assign_from_me'
    this->DrawObject::operator= (assign_from_me);
    this->radius = assign_from_me.radius;

    return *this;
}

void DrawCircle::Draw(cairo_t *cr){
    if(!isVisible)
        return;
    cairo_save(cr);
    cairo_transform(cr, &transformMat);
    cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
    cairo_arc(cr, center.x, center.y, radius, 0, 2 * 3.14159);
    drawMethod(cr);
    cairo_restore(cr);
}

/*
    DrawContainer class defination
*/
DrawContainer::DrawContainer(const DrawContainer& copy_from_me) : DrawRectangle(copy_from_me){
    //cout << "DrawContainer copy\n";
    // delete all the 'DrawObject*' object in member 'drawComponents'
    //for(vector<DrawObject*>::iterator vec_it = this->drawComponents.begin(); vec_it != this->drawComponents.end(); ++vec_it){
      //  delete *vec_it;
   // }

    // clear the vector 'drawComponents'
    this->drawComponents.clear();

    // copy the 'drawComponents' from 'copy_from_me'
    for(vector<DrawObject*>::const_iterator vec_it = copy_from_me.drawComponents.begin(); vec_it != copy_from_me.drawComponents.end(); ++vec_it){
        this->drawComponents.push_back((*vec_it)->Clone());
    } 
}

DrawContainer& DrawContainer::operator=(const DrawContainer& assign_from_me){
    //cout << "DrawContainer assignment\n";
    if(this == &assign_from_me)
        return *this;
    
    this->DrawRectangle::operator= (assign_from_me);
    // clear the vector 'drawComponents'
    for(vector<DrawObject*>::iterator vec_it = this->drawComponents.begin(); vec_it != this->drawComponents.end(); ++vec_it){
        delete *vec_it;
    }
    this->drawComponents.clear();

    // copy the 'drawComponents' from 'assign_from_me'
    for(vector<DrawObject*>::const_iterator vec_it = assign_from_me.drawComponents.begin(); vec_it != assign_from_me.drawComponents.end(); ++vec_it){
        this->drawComponents.push_back((*vec_it)->Clone());
    }

    return *this;
}

void DrawContainer::Draw(cairo_t *cr){
    if(!isVisible)
        return;
    cairo_save(cr);
    cairo_transform(cr, &transformMat);
    for(vector<DrawObject*>::iterator vec_it = drawComponents.begin(); vec_it != drawComponents.end(); ++vec_it){
        (*vec_it)->Draw(cr);
    }
    cairo_restore(cr);
}

/*
    PhysicalObject class defination
*/
PhysicalObject& PhysicalObject::operator=(const PhysicalObject& assign_from_me){
    if(this == &assign_from_me)
        return *this;

    
    this->name = assign_from_me.name;
    this->center = assign_from_me.center;
    this->drawContainer = assign_from_me.drawContainer;

    return *this;
}

void PhysicalObject::addRelation(const pair<string, string>& relation){
    string relationType = relation.first;
    string relationName = relation.second;
    
    relationList[relationType].insert(relationName);
}

/*
    static function
*/
static double transform(double source, double scale, double translate){
    return (source + translate) * scale;
}