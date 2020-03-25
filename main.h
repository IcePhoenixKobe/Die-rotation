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

// for parser.cpp
// #include <stdlib.h>
// #include <map>
// #include <set>

// whether output minimize in LP file
extern bool min_output;
// whether ignore the power(P/V/VCC/VDD) and ground(G/VSS) in netlist file
extern bool ignore_P_G;

// Cartesian coordinate system
typedef struct Cartesian
{
    double x;
    double y;
    Cartesian() : x(0), y(0) {}
    Cartesian(double par_x, double par_y) : x(par_x), y(par_y) {}
} Cartesian;

// Polar coordinate system
typedef struct Polar
{
    double radius;
    double angle;
    Polar() : radius(0), angle(0) {}
    Polar(double ra, double an) : radius(ra), angle(an) {}
} Polar;

typedef struct Relationship {
	std::string relation_name;
    std::vector<std::pair<size_t, size_t>> dice_pads_index;
    std::vector<size_t> balls_index;
    Cartesian pad_car;
    Cartesian ball_car;
    Polar pad_pol;
    Relationship() : relation_name("unknown")
    {
        dice_pads_index.clear();
        balls_index.clear();
        pad_car = Cartesian(0.0, 0.0);
        ball_car = Cartesian(0.0, 0.0);
        pad_pol = Polar(0.0, 0.0);
    }
    Relationship(std::string name, std::vector<std::pair<size_t, size_t>> par_dice_pads_index, std::vector<size_t> par_balls_index, Cartesian par_pad, Cartesian par_ball) : relation_name(name)
    {
        dice_pads_index = par_dice_pads_index;
        balls_index = par_balls_index;
        pad_car = par_pad;
        ball_car = par_ball;
    }
}Relationship;

void check_argument(int, char*[]);

#endif
