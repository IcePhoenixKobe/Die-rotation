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

// Relationship of outer connection
typedef struct Outer_Relationship {
	std::string relation_name;
    std::vector<size_t> balls_index;
    std::vector<std::vector<size_t>> dice_pads_index;
    Cartesian ball_car;
    Cartesian pad_car;
    Polar pad_pol;
    // constructor
    Outer_Relationship() : relation_name("unknown"), ball_car(Cartesian(0.0, 0.0)), pad_car(Cartesian(0.0, 0.0)), pad_pol(Polar(0.0, 0.0))
    {
        dice_pads_index.clear();
        balls_index.clear();
    }
    Outer_Relationship(std::string name, std::vector<std::vector<size_t>> par_dice_pads_index, std::vector<size_t> par_balls_index, Cartesian par_pad, Cartesian par_ball) : relation_name(name)
    {
        dice_pads_index = par_dice_pads_index;
        balls_index = par_balls_index;
        ball_car = par_ball;
        pad_car = par_pad;
    }
} OuterRelationship;

// Relationship of inner connection
typedef struct Inner_Relationship {
	std::string relation_name;
    std::pair<unsigned short, std::vector<size_t>> dice_pads1_index;
    std::pair<unsigned short, std::vector<size_t>> dice_pads2_index;
    Cartesian pad1_car;
    Cartesian pad2_car;
    Polar pad1_pol;
    Polar pad2_pol;
    // constructor
    Inner_Relationship() : relation_name("unknown")
    {
        dice_pads1_index.first = 0;
        dice_pads2_index.first = 0;
        dice_pads1_index.second.clear();
        dice_pads2_index.second.clear();
        pad1_car = Cartesian(0.0, 0.0);
        pad2_car = Cartesian(0.0, 0.0);
        pad1_pol = Polar(0.0, 0.0);
        pad2_pol = Polar(0.0, 0.0);
    }
    Inner_Relationship(std::string name, std::pair<unsigned short, std::vector<size_t>> par_pads1_index, std::pair<unsigned short, std::vector<size_t>> par_pads2_index, Cartesian par_pad1, Cartesian par_pad2) : relation_name(name)
    {
        dice_pads1_index = par_pads1_index;
        dice_pads2_index = par_pads2_index;
        pad1_car = par_pad1;
        pad2_car = par_pad2;
    }
} InnerRelationship;

// set global parameter with input argument
void check_argument(int, char*[]); 
bool ignore_Power_Ground(std::string);
// Convert cartesian coordinate system to polar coordinate system
Polar convert_cart_to_polar(Cartesian);
// calculate center of gravity by cartesian coordinate system
Cartesian CG(std::vector<Cartesian>);

/* use to classify padGroup 
 * currently we have four directions for Group
 * */
enum class Group{
    UP, DOWN, LEFT, RIGHT
};

typedef struct Item {
	std::string name;
	Cartesian xy;

    Item() : name("unknownItem"), xy(Cartesian(0.0, 0.0)) {}
    Item(std::string str, Cartesian par_xy) : name(str), xy(par_xy) {}
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
/* according to the information in 'balls', 'pads' and 'padBallPair', construct the 'PadGroupMap' and 
 * correspond DrawObject such that pad DrawRectangle, ball DrawCircle, pad_to_ball DrawLine
 * */
void initialize(std::map<std::string, std::map<std::string, item>>&, 
        std::map<Group, PadGroup>&);

// For kernel.cc
extern Cartesian GOD_Center;
extern double GOD_Rotation;
extern Cartesian GOD_GOD_Center;
extern double GOD_GOD_Rotation;
// original rotation
extern std::vector<double> ori_rotas;

#endif  // KERNEL_H