#ifndef CHIP_H
#define CHIP_H

#include"ball.h"
#include"die.h"

class Chip
{
private:
    Ball balls;
    std::vector<Die> dice;
    std::vector<InnerRelationship> inner_netlist;
    std::vector<OuterRelationship> outer_netlist;

    void parse_PIN(std::ifstream&);
    void parse_Netlist(std::ifstream&);
public:
    Chip();
    ~Chip() {}

    // chip information

    void balls_Content();
    void dice_Content();
    void netlist_Content();
    /*====================*/

    // file parser
    // parse PIN, netlist(, and shuffle_netlist)
    int parser(int, char**);

    /*====================*/

    // get ball's information
    size_t get_Ball_Amount() const { return balls.get_Amount(); }
    std::vector<std::string> get_Balls_Name() const { return balls.get_All_Name(); }
    std::string get_Ball_Name(size_t index) const { return balls.get_Name(index); }
    std::vector<Cartesian> get_Balls_Pos() const { return balls.get_All_Pos(); }
    Cartesian get_Ball_Pos(size_t index) const { return balls.get_Position(index); }

    // get dice's information
    int get_Die_Pad_Amount(int i) const { return dice[i].get_Pad_Amount(); }
    Cartesian get_Die_Center(int i) const { return dice[i].get_Center(); }
    std::vector<Cartesian> get_Die_Cart_Pos(int i) const { return dice[i].get_Pads_Cart_Position(); }
    std::vector<Polar> get_Die_Pol_Pos(int i) const { return dice[i].get_Pads_Pol_Position(); }
    std::vector<Die> get_Dice() const { return dice; }
    Die get_Die(size_t index) const { return dice[index]; }

    // get netlist information
    size_t get_Netlist_Index(std::string) const;
    std::vector<InnerRelationship> get_All_I_Netlist() const { return inner_netlist; }
    std::vector<OuterRelationship> get_All_O_Netlist() const { return outer_netlist; }

    // set the data of ball
    void set_Ball_Amount(int num) {balls.set_Amount(num); }
    void set_Ball_Pos(std::vector<Cartesian> pos) { balls.set_All_Pos(pos); }

    // set the data of dice
    void set_Die_Amount(int i, int num) { dice[i].set_Amount(num); }
    void set_Die_Center(int i, Cartesian single_pos) { dice[i].set_Center(single_pos); }
    void set_Die_Cart_Pos(int i, std::vector<Cartesian> cart_pos) { dice[i].set_Cart_Pos(cart_pos); }
    void set_Die_Pol_Pos(int i, std::vector<Polar> pol_pos) { dice[i].set_Pol_Pos(pol_pos); }
    
    // file output
    void output_LP_File(std::ofstream&);
    void output_M_File(std::ofstream&, char*);
};

#endif  // CHIP_H
