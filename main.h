#ifndef MAIN_H
#define MAIN_H

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
    Outer_Relationship() : relation_name("unknown")
    {
        dice_pads_index.clear();
        balls_index.clear();
        pad_car = Cartesian(0.0, 0.0);
        ball_car = Cartesian(0.0, 0.0);
        pad_pol = Polar(0.0, 0.0);
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

// whether output minimize in LP file
extern bool min_output;
// whether ignore the power(P/V/VCC/VDD) and ground(G/VSS) in netlist file
extern bool ignore_P_G;
// The index of argument that point next one argument is .lp file name
extern int LP_out;
// The index of argument that point next one argument is .M file name
extern int M_out;
// GUI
extern int GUI;
// For kernel.cc
extern Cartesian GOD_Center;
extern double GOD_Rotation;
extern Cartesian GOD_GOD_Center;
extern double GOD_GOD_Rotation;

void check_argument(int, char*[]);
Cartesian CG(std::vector<Cartesian>);

bool ignore_Power_Ground(std::string);

#endif  // MAIN_H