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
#include<string>
#include<vector>
#include<cmath>

// for parser.cpp
// #include <stdlib.h>
// #include <map>
// #include <set>

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
	size_t ball_index;
	size_t die_index;
	size_t pad_index;
    Relationship() : relation_name("unknown"), ball_index(-1), die_index(-1), pad_index(-1) {}
    Relationship(std::string name, size_t bi, size_t di, size_t pi) : 
        relation_name(name), ball_index(bi), die_index(di), pad_index(pi) {}
}Relationship;

int parser(int, char**);   // for parser.cpp

#endif
