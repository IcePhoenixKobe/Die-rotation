#ifndef CHIP_H
#define CHIP_H

#include"main.h"
#include"ball.h"
#include"die.h"

class Chip
{
private:
    Die die;
    Ball ball;
    std::vector<Relationship> netlist;

    void parse_Die(std::vector<std::vector<std::string>>);
    void parse_Ball(std::vector<std::vector<std::string>>);
    void parse_PIN(std::ifstream&);
public:
    Chip();
    ~Chip() {}

    // chip information

    void die_Content();
    void ball_Content();
    void netlist_Content();
    /*====================*/

    // file parser
    // parse PIN, netlist(, and shuffle_netlist)
    int parser(int, char**);

    /*====================*/

    // get ball's paramenter
    int get_Ball_Amount() const { return ball.get_Amount(); }
    std::vector<Cartesian> get_Ball_Pos() const { return ball.get_All_Pos(); }

    // get die's paramenter
    int get_Die_Amount() const { return die.get_Amount(); }
    Cartesian get_Die_Center() const { return die.get_Center(); }
    std::vector<Cartesian> get_Die_Cart_Pos() const { return die.get_Cart_Position(); }
    std::vector<Polar> get_Die_Pol_Pos() const { return die.get_Pol_Position(); }

    // get netlist information
    int get_Netlist_Index(std::string) const;

    // set bll's paramenter
    void set_Ball_Amount(int num) {ball.set_Amount(num); }
    void set_Ball_Pos(std::vector<Cartesian> pos) { ball.set_All_Pos(pos); }

    // set die's parameter
    void set_Die_Amount(int num) { die.set_Amount(num); }
    void set_Die_Center(Cartesian single_pos) { die.set_Center(single_pos); }
    void set_Die_Cart_Pos(std::vector<Cartesian> cart_pos) { die.set_Cart_Pos(cart_pos); }
    void set_Die_Pol_Pos(std::vector<Polar> pol_pos) { die.set_Pol_Pos(pol_pos); }
    
    // file output
    void output_LP_File(std::ofstream&);
};

#endif
