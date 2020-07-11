#ifndef KERNEL_H
#define KERNEL_H

#include<iostream>
//#define NDEBUG
#include<assert.h>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<utility>
#include<math.h>
#include<string.h>
#include<vector>
#include<cmath>
#include<map>
#include<set>

// Cartesian coordinate system
typedef struct Cartesian
{
    double x;
    double y;
    Cartesian() : x(0.0), y(0.0) {}
    Cartesian(double par_x, double par_y) : x(par_x), y(par_y) {}

    double distance() { return sqrt(pow(x, 2.0) + pow(y, 2.0)); }
}Cartesian;

// Polar coordinate system
typedef struct Polar
{
    double radius;
    double angle;
    Polar() : radius(0.0), angle(0.0) {}
    Polar(double ra, double an) : radius(ra), angle(an) {}
}Polar;

// Relationship of outer connection
typedef struct Outer_Relationship {
	std::string relation_name;
    std::vector<std::pair<size_t, size_t>> dice_pads_index;
    std::vector<size_t> balls_index;
    Cartesian pad_car;
    Cartesian ball_car;
    Polar pad_pol;
    Outer_Relationship() : relation_name("unknown"), pad_car(Cartesian(0.0, 0.0)), ball_car(Cartesian(0.0, 0.0)), pad_pol(Polar(0.0, 0.0))
    {
        dice_pads_index.clear();
        balls_index.clear();
    }/*
    Outer_Relationship(std::string name, std::vector<std::pair<size_t, size_t>> par_dice_pads_index, std::vector<size_t> par_balls_index, Cartesian par_pad, Cartesian par_ball) : relation_name(name)
    {
        dice_pads_index = par_dice_pads_index;
        balls_index = par_balls_index;
        pad_car = par_pad;
        ball_car = par_ball;
    }*/
}OuterRelationship;

// Relationship of inner connection
typedef struct Inner_Relationship {
	std::string relation_name;
    std::pair<size_t, std::vector<size_t>> dice_pads1_index;
    std::pair<size_t, std::vector<size_t>> dice_pads2_index;
    Cartesian pad1_car;
    Cartesian pad2_car;
    Polar pad1_pol;
    Polar pad2_pol;
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
    }/*
    Inner_Relationship(std::string name, std::pair<size_t, std::vector<size_t>> par_pads1_index, std::pair<size_t, std::vector<size_t>> par_pads2_index, Cartesian par_pad1, Cartesian par_pad2) : relation_name(name)
    {
        dice_pads1_index = par_pads1_index;
        dice_pads2_index = par_pads2_index;
        pad1_car = par_pad1;
        pad2_car = par_pad2;
    }*/
}InnerRelationship;

void check_argument(int, char*[]);
Cartesian CG(std::vector<Cartesian>);

bool ignore_Power_Ground(std::string);

/* use to classify padGroup 
 * currently we have four directions for Group
 * */
enum class Group{
    UP, DOWN, LEFT, RIGHT
};

typedef struct Item {
	std::string name;
	double x;
	double y;
}item;

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
void initialize(const std::map<std::string, std::map<std::string, item>> &items, 
        const std::map<std::string, std::string> &padBallPair,
        const std::map<std::string, std::string> &padArtificialFingerPair,
        std::map<Group, PadGroup> &padGroupMap);

// For kernel.cc
extern Cartesian GOD_Center;
extern double GOD_Rotation;
extern Cartesian GOD_GOD_Center;
extern double GOD_GOD_Rotation;

#endif  // KERNEL_H