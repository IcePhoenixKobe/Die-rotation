#ifndef KERNEL_H
#define KERNEL_H

#include<iostream>
//#define NDEBUG
#include<assert.h>
#include<string.h>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<utility>
#include<math.h>
#include <limits>
#include<vector>
#include<cmath>
#include<map>
#include<set>

// Cartesian coordinate system
typedef struct Cartesian {
    double x;
    double y;
    // constructor
    Cartesian() : x(0.0), y(0.0) {}
    Cartesian(double par_x, double par_y) : x(par_x), y(par_y) {}
    // function
    double distance() { return sqrt(pow(x, 2.0) + pow(y, 2.0)); }
} Cartesian;

// Polar coordinate system
typedef struct Polar {
    double radius;
    double angle;
    //constructor
    Polar() : radius(0.0), angle(0.0) {}
    Polar(double par_radius, double par_angle) : radius(par_radius), angle(par_angle) {}
    // function
    double get_Radian() { return angle * M_PI / 180.0; }
} Polar;

// basic information of chip
typedef struct Info {
    // package
    Cartesian packageSize;
    size_t numLayer;
    // BGA balls
    size_t ballDimensionX;
    size_t ballDimensionY;
    double ballDiameter;
    double ballPitch;
    // path
    double viaDiameter;
    double wireWidth;
    double spacing;
} infomation;

// Relationship of pins1 and pins2
typedef struct Relationship {
    // die number of die pads or 0(BGA balls)
    size_t pins1_number;
    // die number of die pads
    size_t pins2_number;
    std::vector<std::string> pins1;
    std::vector<std::string> pins2;
    // constructor
    Relationship() : pins1_number(-1UL), pins2_number(-1UL) { pins1.clear(); pins2.clear(); }
    Relationship(size_t par_number1, size_t par_number2, std::vector<std::string> par_pins1, std::vector<std::string> par_pins2) 
        : pins1_number(par_number1), pins2_number(par_number2), pins1(par_pins1), pins2(par_pins2) {}
    // function
    std::pair<Cartesian, Cartesian> get_CWH();
} relationship;

// set global parameter with input argument
void check_argument(int, char*[]);
// judging whether ignore current netlist
bool ignore_Power_Ground(std::string);
// Convert cartesian coordinate system to polar coordinate system (point, center)
Polar convert_cart_to_polar(Cartesian, Cartesian);
// calculate center of gravity by cartesian coordinate system
Cartesian CG(std::vector<Cartesian>);
// point rotation
Cartesian shift_rotation(Cartesian, Cartesian, Cartesian, double);

/* use to classify padGroup, currently we have four directions for Group */
enum class Group{
    UP, DOWN, LEFT, RIGHT
};

typedef struct Item {
	std::string name;
	Cartesian xy;
    double rotation;    // angle
    // constructor
    Item() : name("unknownItem"), xy(Cartesian(0.0, 0.0)), rotation(0.0) {}
    Item(std::string str, Cartesian par_xy, double par_rotation) : name(str), xy(par_xy), rotation(par_rotation) {}
    // function
    double getRadian() { return rotation * M_PI / 180.0; }
} item;

/* PadGroup */
struct PadGroup{
    Group group;
    // record the name of pad which is belong to 'group' and sort by 'group'.
    // e.g., if 'group' is equal to Up, then they are sorted according to the X position of pad from left to right.
    std::vector<std::string> pad_name_vec;
    // the position of leftmost and rightmost pad
    double leftmost_pos, rightmost_pos;
    // average height in 'pad_name_vec'
    double avg_height;
};

/* Transfer chip data to windowApp g_entireObjMap */
void dataTransfer();
/* according to the information in 'balls', 'dice' and 'netlist' and correspond DrawObject
 * such that dicee DrawRectangle, ball DrawCircle, netlist DrawLine*/
void initialize(std::map<std::string, item>&,
                             std::vector<std::pair<Cartesian, Cartesian>>&,
                             std::vector<std::map<std::string, item>>&,
                             std::map<std::string, std::map<std::string, relationship>>&);

// For kernel.cc
extern Cartesian GOD_Center;
extern double GOD_Rotation;
extern Cartesian GOD_GOD_Center;
extern double GOD_GOD_Rotation;
// original rotation
//extern std::vector<double> ori_rotas;

#endif  // KERNEL_H