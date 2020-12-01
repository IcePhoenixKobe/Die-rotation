#ifndef GTK_CAIRO_H
#define GTK_CAIRO_H

#include"kernel.h"
#include<gtk/gtk.h>

using namespace std;

class Point{
    private:
    public:
        double x, y;
        Point() : x(0.0), y(0.0) {}
        Point(double x, double y) : x(x), y(y) {}
        double distance(Point point) { return sqrt(pow(this->x - point.x, 2.0) + pow(this->y - point.y, 2.0)); }
        friend Point operator+(const Point&, const Point&);
        friend Point operator-(const Point&, const Point&);
        friend Point operator/(const Point&, double);
};

class Color{
    private:
    public:
        double r, g, b, a;
        Color() : r(1.0), g(1.0), b(1.0), a(1.0) {}
        Color(double r, double g, double b, double a = 1.0) : r(r), g(g), b(b), a(a) {}
};

/* the base class for all drawing class */
class DrawObject{
    protected:
        Point center;
        Color color;
        void (*drawMethod)(cairo_t*);   // cairo_fill or cairo_stroke depends on user
        cairo_matrix_t transformMat;    // transformation matrix for this 'DrawObject' (including translate and scale functionality)
        bool isVisible = true;
    public:
        // default constructor
        DrawObject() : center(), color(), drawMethod(cairo_fill)
            { cairo_matrix_init_identity(&transformMat); }
        // parameters constructor
        DrawObject(Point center, Color color, void (*drawMethod)(cairo_t*) = cairo_fill) : center(center), color(color), drawMethod(drawMethod)
            { cairo_matrix_init_identity(&transformMat); }
        // copy constructor
        DrawObject(const DrawObject& copy_from_me) : center(copy_from_me.center), color(copy_from_me.color), drawMethod(copy_from_me.drawMethod), transformMat(copy_from_me.transformMat) {}
        // assignment
        DrawObject& operator=(const DrawObject&);
        // virtual destructor
        virtual ~DrawObject() {}

        // Set function
        // set the member 'center'
        void setCenterPoint(Point center) { this->center = center; }
        // set the member 'color'
        void setColor(Color color) { this->color = color; }
        // set the member 'drawMethod'
        void setDrawMethod(void(*drawMethod)(cairo_t*)) { this->drawMethod = drawMethod; }
        // set the member 'isVisible'
        void setIsVisible(bool isVisable) { this->isVisible = isVisible; }
        
        // Get function
        // get the member 'center'
        Point getCenterPoint() { return center; }
        // get the member 'color'
        Color getColor() { return color; }
        // get the member 'drawMethod'
        void (*getDrawMethod())(cairo_t*) { return drawMethod; }
        // get the member 'isVisible'
        bool getIsVisible() const { return isVisible; }
        // draw itself (pure virtual function, all inheritance class need implement this function)
        
        // Virtual function
        virtual void Draw(cairo_t *) {}
        // deep copy, simulate the virtual copy constructor
        virtual DrawObject* Clone() const { return new DrawObject(*this); }
        
        // Other function
        // translate the 'transformMat' with x and y
        void translate(double x, double y);
        // rotate the 'transformMat'with radians (origin is the 'center')
        void rotate(double radians);
        // scale the 'transforMat' with sx and sy (origin is the 'center') 
        void scale(double sx, double sy);
        //template <class T> friend double distance(const T&, const T&);
        template <class T1, class T2> friend double distance(const T1&, const T2&);
};

/* draw the line from 'source' to 'end' point */
class DrawLine : public DrawObject{
    protected:
        Point end;  // the end point of the line
    public:
        // default constructor
        DrawLine() : DrawObject(), end(1.0, 1.0) {}
        // parameters constructor
        DrawLine(Point source, Point end, Color color) : DrawObject(source, color), end(end) {}
        // copy construtor
        DrawLine(const DrawLine& copy_from_me) : DrawObject(static_cast<DrawObject>(copy_from_me)), end(copy_from_me.end) {}
        // assignment
        DrawLine& operator=(const DrawLine&);
        // virtual destructor
        virtual ~DrawLine() {}

        // Set function
        // set the member 'source' (source == center)
        void setSourcePoint(Point source) { setCenterPoint(source); }
        // set the member 'end'
        void setEndPoint(Point end) { this->end = end; }

        // Get function
        // get the member 'source' (source == center)
        Point getSourcePoint() { return center; }
        // get the member 'end'
        Point getEndPoint() { return end; }

        // Virtual function
        // draw itself (draw a line which start from 'source' to 'end' point)
        virtual void Draw(cairo_t *) override;
        // deep copy, simulate the virtual copy constructor
        virtual DrawLine* Clone() const override { return new DrawLine(*this); }
};

/* draw the rectangle 
 *
 * topLeft---------------------witdh
 *                              |
 *                              |
 *                              |
 *                              |
 *                              |
 *                            height         
 * */
class DrawRectangle : public DrawObject{
    protected:
        double width, height;           // the width and height of the rectangle
    public:
        // default constructor
        DrawRectangle() : DrawObject(), width(1.0), height(1.0) {}
        // parameters constructor
        DrawRectangle(Point center, Color color, double width, double height, void (*drawMethod)(cairo_t *)) : DrawObject(center, color, drawMethod), width(width), height(height) {}
        // copy constructor
        DrawRectangle(const DrawRectangle& copy_from_me) : DrawObject(copy_from_me),  width(copy_from_me.width), height(copy_from_me.height) {}
        // assignment
        DrawRectangle& operator=(const DrawRectangle&);
        // virtual constructor
        virtual ~DrawRectangle() {}

        // Set function
        // set the member 'width'
        void setWidth(double width) { this->width = width; }
        // set the member 'height'
        void setHeight(double height) { this->height = height; }

        // Get function
        // get the member 'width'
        double getWidth() { return width; }
        // get the member 'height'
        double getHeight() { return height; }

        // Virtual function
        // draw itself
        virtual void Draw(cairo_t *) override;
        // deep copy, simulate the virtual copy constructor
        virtual DrawRectangle* Clone() const override { return new DrawRectangle(*this); }
        // calculate the distance of two drawRectangles
        //template <class T> friend double distance(const T&, const T&);
        template <class T1, class T2> friend double distance(const T1&, const T2&);
};

/* draw the circle */
class DrawCircle : public DrawObject{
    protected:
        double radius;
    public:
        // default constructor
        DrawCircle() : DrawObject(), radius(1.0) {}
        // parameters constructor
        DrawCircle(Point center, Color color, double radius, void (*drawMethod)(cairo_t*)) : DrawObject(center, color, drawMethod), radius(radius) {}
        // copy constructor
        DrawCircle(const DrawCircle& copy_from_me) : DrawObject(copy_from_me), radius(copy_from_me.radius) {}
        // assignment
        DrawCircle& operator=(const DrawCircle&);
        // virtual destructor
        virtual ~DrawCircle() {}

        // Virtual function
        // draw itself
        virtual void Draw(cairo_t *) override;
        // deep copy, simulate the virtual copy constructor
        virtual DrawCircle* Clone() const override { return new DrawCircle(*this); }
        //double distance(DrawRectangle rect);
        //template <class T> friend double distance(const T&, const T&);
        template <class T1, class T2> friend double distance(const T1&, const T2&);
};

/* contain multiple drawObject */
class DrawContainer : public DrawRectangle{
    protected:
        // store all 'DrawObject' objects in this member
        // use STL 'vector' malnipulate
        vector<DrawObject*> drawComponents;
    public:
        // default constructor
        DrawContainer() : DrawRectangle() {}
        // parameters constructor
        DrawContainer(Point center, double width, double height) : DrawRectangle(center, Color(), width, height, cairo_stroke) {}
        // copy constructor
        DrawContainer(const DrawContainer&);
        // assignment
        DrawContainer& operator=(const DrawContainer&);
        // destructor
        virtual ~DrawContainer() { for(vector<DrawObject*>::iterator vec_it = drawComponents.begin(); vec_it != drawComponents.end(); ++vec_it) delete *vec_it; }

        // Other function
        // push a 'DrawObject' object in 'drawComponents' member
        void push_back(DrawObject* drawObj) { drawComponents.push_back(drawObj->Clone()); }
        // Draw itselft (iterative draw all 'DrawObject' object in 'drawComponents')
        virtual void Draw(cairo_t*) override;
        // deep copy, simulate the virtual copy constructor
        virtual DrawContainer* Clone() const override { return new DrawContainer(*this); }
        // overloading subscript operator to index 'DrawObject' in 'drawComponents'
        DrawObject& operator[](int index) { return *drawComponents[index]; }
        //template <class T> friend double distance(const T&, const T&);
        template <class T1, class T2> friend double distance(const T1&, const T2&);
        // friend class that it can directly access private and protected member
        friend class PhysicalObject;
};

/* phyiscal Object such as pad, finger, ball */
class PhysicalObject{
    protected:
        // the name of the 'physicalObject'
        string name;
        // the center of the 'physicalObject' (usally same with the center of the 'drawContainer')
        Point center;
        // the relationship with other 'PhysicalObject' (store the name)
        map<string, set<string>> relationList;
        // pointer to the manager of the 'PhysicalObject'
        map<string, map<string, string>> *g_entireObjMap;
    public:
        DrawContainer drawContainer;
        // default constructor
        PhysicalObject() {}
        // parameters constructor
        PhysicalObject(string name, Point center, DrawContainer drawContainer) : name(name), center(center), drawContainer(drawContainer) {}
        // copy constructor
        PhysicalObject(const PhysicalObject& copy_from_me) : name(copy_from_me.name), center(copy_from_me.center), drawContainer(copy_from_me.drawContainer) {}
        // assignment constrcutor
        PhysicalObject& operator=(const PhysicalObject&);
        // virtual destructor
        virtual ~PhysicalObject() {}

        // Set function
        // set the member 'name'
        void setName(string name) { this->name = name; }
        // set the member 'center'
        void setCenter(Point center) { this->center = center; }

        // Get function
        // get the member 'name'
        string getName() const { return name; }
        // get the member 'center'
        Point getCenter() const { return center; }
        // get set of relationship
        const set<string>& getRelation(const string relationType) const { return relationList.at(relationType); }
        // find a relationship
        // get the first relation name in this relation type
        string getFirstRelation(const string relationType) const { return *(relationList.at(relationType).begin()); }
        
        // Other function
        // add a relationship
        void addRelation(const pair<string, string>&);
        //template <class T> friend double distance(const T&, const T&);
        template <class T1, class T2> friend double distance(const T1&, const T2&);
};

//template <class T> double distance(const T&, const T&);
template <class T1, class T2> double distance(const T1& t1, const T2& t2){
    return sqrt(pow(t1.center.x - t2.center.x, 2.0) + pow(t1.center.y - t2.center.y, 2.0));
}

#endif  // GTK_CAIRO_H