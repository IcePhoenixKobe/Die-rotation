#ifndef MAIN_H
#define MAIN_H

#include<iostream>
//#define NDEBUG
#include<assert.h>
#include<fstream>
#include<math.h>
#include<string>
#include<vector>
#include<cmath>

// for parser.cpp
#include <utility>
#include <sstream>
#include <stdlib.h>
#include <iomanip>
#include <map>
#include <set>

// Cartesian coordinate system
typedef struct Cartesian
{
    double x;
    double y;
} Cartesian;

// Polar coordinate system
typedef struct Polar
{
    double radius;
    double angle;
} Polar;

// for parser.cpp
struct Item {
	std::string name;
	double x;
	double y;
};

// for parser.cpp
typedef struct Relationship {
	std::string relation_name;
	int ball_index;
	int die_pad_index;
}Relationship;

int parser(int, char**);   // for parser.cpp

#endif
