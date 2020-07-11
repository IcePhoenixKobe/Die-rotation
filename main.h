#ifndef MAIN_H
#define MAIN_H

#include"kernel.h"
#include"chip.h"
#include"windowApp.h"

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

extern Chip *chip;

#endif  // MAIN_H