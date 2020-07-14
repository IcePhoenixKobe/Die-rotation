#ifndef CHIP_H
#define CHIP_H

// whether output minimize in LP file
extern bool min_output;

#include"kernel.h"

class Ball
{
private:
    std::size_t amount;
    std::vector<std::string> name;
    std::vector<Cartesian> position;
public:
    // default constructor
    Ball();
    // constructor
    Ball(std::size_t);
    // constructor
    Ball(std::vector<std::string> par_name, std::vector<Cartesian> par_pos) : name(par_name), position(par_pos) {
        assert(par_name.size() == par_pos.size());
        amount = par_name.size();
    }
    // copy constructor
    Ball(const Ball& me) : amount(me.amount), name(me.name), position(me.position) {}
    // destructor
    ~Ball() {}

    // Get function
    size_t get_Amount() const { return amount; }
    std::vector<std::string> get_All_Name() const { return name; }
    std::vector<Cartesian> get_All_Position() const { return position; }
    size_t get_Ball_Index(std::string str) const;
    std::string get_Name(size_t index) const { return name[index]; }
    Cartesian get_Position(size_t index) const { return position[index]; }

    // Set function
    void set_Amount(size_t par_amount) { amount = par_amount; }
    void set_All_Name(std::vector<std::string> par_name) { name = par_name; }
    void set_All_Pos(std::vector<Cartesian> pos) { position = pos; }
};

class Die
{
private:
    std::size_t amount;
    Cartesian center;
    std::vector<std::string> name;
    std::vector<std::pair<Cartesian, Polar>> position;
    std::vector<double> rotation;

    void Clear_Pad();
    void Resize_Pad(size_t);
public:
    // default constructor
    Die() : amount(0), center(Cartesian(0.0, 0.0)) { Clear_Pad(); }
    // constructor with pad amount
    Die(std::size_t);
    // constructor with die center
    Die(Cartesian xy) : center(xy) { Clear_Pad(); }
    // constructor with pad amount and die center
    Die(std::size_t, Cartesian);
    // copy constructor
    Die(const Die& me) : amount(me.amount), center(me.center), name(me.name), position(me.position), rotation(me.rotation) {}
    // destructor
    ~Die() {}

    // Get function
    size_t get_Pad_Amount() const { return amount; }
    Cartesian get_Center() const { return center; }
    std::vector<std::string> get_Pads_Name() const { return name; }
    std::vector<std::pair<Cartesian, Polar>> get_Pads_Position() const { return position; }
    std::vector<double> get_Pads_Rotation() const { return rotation; }
    // Get individual data
    size_t get_Pad_Index(std::string str) const;
    std::string get_Pad_Name(size_t index) const { return name[index]; }
    Cartesian get_Cart_Position(size_t index) const { return position[index].first; }
    Polar get_Pol_Position(size_t index) const { return position[index].second; }
    double get_Pad_Rotation(size_t index) const { return rotation[index]; }

    // Set function
    void set_Amount(size_t par_amount) { amount = par_amount; }
    void set_Center(Cartesian xy) { center = xy; }
    void set_Pads(std::vector<std::string>, std::vector<Cartesian>, std::vector<double>);

    void Original_Position();
};

class Chip
{
private:
    Ball balls;
    std::vector<Die> dice;
    std::vector<InnerRelationship> inner_netlist;
    std::vector<OuterRelationship> outer_netlist;
public:
    infomation drc;

    Chip();
    ~Chip() {}

    // chip information
    void basic_infomation();
    void balls_Content();
    void dice_Content();
    void netlist_Content();
    /*====================*/

    // Get function
    // get ball's information
    size_t get_Ball_Amount() const { return balls.get_Amount(); }
    std::vector<std::string> get_Balls_Name() const { return balls.get_All_Name(); }
    std::vector<Cartesian> get_Balls_Position() const { return balls.get_All_Position(); }
    size_t get_Ball_Index(std::string str) const { return balls.get_Ball_Index(str); }
    std::string get_Ball_Name(size_t index) const { return balls.get_Name(index); }
    Cartesian get_Ball_Position(size_t index) const { return balls.get_Position(index); }
    // get dice's information
    std::vector<Die> get_Dice() const { return dice; }
    size_t get_Dice_Amount() const { return dice.size(); }
    size_t get_Die_Index(std::string) const;
    size_t get_Die_Pad_Amount(size_t index) const { return dice[index].get_Pad_Amount(); }
    Cartesian get_Die_Center(size_t index) const { return dice[index].get_Center(); }
    size_t get_Die_Pad_Index(size_t index, std::string str) const { return dice[index].get_Pad_Index(str); }
    Cartesian get_Die_Pad_Position(size_t die_index, size_t pad_index) const { return dice[die_index].get_Cart_Position(pad_index); }
    double get_Die_Pad_Rotation(size_t die_index, std::string str) const { return dice[die_index].get_Pad_Rotation(dice[die_index].get_Pad_Index(str)); }
    Die get_Die(size_t index) const { return dice[index]; }
    // get netlist information
    size_t get_Netlist_Index(std::string) const;
    std::vector<InnerRelationship> get_All_I_Netlist() const { return inner_netlist; }
    std::vector<OuterRelationship> get_All_O_Netlist() const { return outer_netlist; }
    /*====================*/

    // Set function
    // set the data of ball
    void set_Balls_Amount(size_t par_amount) { balls.set_Amount(par_amount); }
    void set_Balls_Name(std::vector<std::string> par_name) { balls.set_All_Name(par_name); }
    void set_Balls_Position(std::vector<Cartesian> pos) { balls.set_All_Pos(pos); }
    // set the data of dice
    void set_Dice_Amount(size_t par_amount) {
        dice.clear();
        for (size_t i = 0; i < par_amount; i++) {
            dice.push_back(Die());
        }
    }
    void set_Die_Amount(size_t index, size_t par_amount) {
        assert(index < dice.size());
        dice[index].set_Amount(par_amount);
    }
    void set_Die_Center(size_t index, Cartesian par_pos) {
        assert(index < dice.size());
        dice[index].set_Center(par_pos);
    }
    void set_Die_Amount_Center(size_t index, size_t par_amount, Cartesian par_center) {
        assert(index < dice.size());
        dice[index] = Die(par_amount, par_center);
    }
    void set_Die_Pads(size_t index, std::vector<std::string> par_names, std::vector<Cartesian> par_poss, std::vector<double> par_rotas) { dice[index].set_Pads(par_names, par_poss, par_rotas); }
    // set the data of netlist
    void set_All_I_Netlist(std::vector<InnerRelationship> par_inner_rela) { inner_netlist = par_inner_rela; }
    void set_All_O_Netlist(std::vector<OuterRelationship> par_outer_rela) { outer_netlist = par_outer_rela; }
    /*====================*/
    
    // file output
    void output_LP_File(std::ofstream&);
    void output_M_File(std::ofstream&, char*);

    void Original_Dice_Pads();
};

#endif  // CHIP_H
