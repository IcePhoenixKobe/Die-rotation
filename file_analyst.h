#include"main.h"

// parse drc file
void parse_Drc(std::ifstream&);
// parse pin file
void parse_PIN(std::ifstream&);
// parse netlist(and shuffle_netlist) file
void parse_Netlist(std::ifstream&);
// file parser, parse drc, pin, netlist file
int parser(int, char**);