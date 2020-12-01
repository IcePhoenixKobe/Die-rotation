#ifndef CHIP_H
#define CHIP_H

// whether output minimize in LP file
extern bool min_output;

#include"kernel.h"

class Ball
{
private:
    item ball;
public:
    // default constructor
    Ball() { ball = item(); }
    // constructor
    Ball(item par_ball) : ball(par_ball) {}
    // copy constructor
    Ball(const Ball& me) : ball(me.ball) {}
    // destructor
    ~Ball() {}

    // Get function
    item get_Item() const { return ball; }
    std::string get_Name() const { return ball.name; }
    Cartesian get_Location() const { return ball.xy; }
    double get_Rotation() const { return ball.rotation; }
    // Get BGA ball radian of rotation
    double get_Radian() { return ball.getRadian(); }

    // Set function
    void set_item(item par_item) { ball = par_item; }
    void set_name(std::string par_name) { ball.name = par_name; }
    void set_Location(Cartesian par_pos) { ball.xy = par_pos; }
    void set_Angle(double par_angle) { ball.rotation = par_angle; }
};

class Die
{
private:
    double width;
    double height;
    Cartesian center;
    std::map<std::string, item> pads;
public:
    // default constructor
    Die() : width(0.0), height(0.0), center(Cartesian(0.0, 0.0)) { pads.clear(); }
    // constructor with die center
    Die(Cartesian xy) : width(0.0), height(0.0), center(xy) { pads.clear(); }
    // constructor with die width and height
    Die(double par_width, double par_height) : width(par_width), height(par_height), center(Cartesian(0.0, 0.0)) { pads.clear(); }
    // constructor with die width, height and center
    Die(double par_width, double par_height, Cartesian xy) : width(par_width), height(par_height), center(xy) { pads.clear(); }
    // copy constructor
    Die(const Die& me) : width(me.width), height(me.height), center(me.center), pads(me.pads) {}
    // destructor
    ~Die() {}

    // Get function
    double get_Width() const { return width; }
    double get_Height() const { return height; }
    double get_diagonal() const { return sqrt(pow(width / 2.0, 2.0) + pow(height / 2.0, 2.0)); }
    Cartesian get_Center() const { return center; }
    size_t get_Pads_Amount() const { return pads.size(); }
    std::map<std::string, item> get_Pads() const { return pads; }
    std::vector<std::string> get_Pads_Number() const;
    std::vector<std::string> get_Pads_Name() const;
    std::map<std::string, Cartesian> get_Pads_Location() const;
    std::map<std::string, double> get_Pads_Rotation() const;
    std::map<std::string, double> get_Pads_Radian();
    // Get individual pad data
    std::string get_Name(std::string str) { return pads[str].name; }
    Cartesian get_Pad_Cart_Location(std::string str) { return pads[str].xy; }
    // maybe make some mistake
    // Polar get_Pad_Pol_Location(std::string str) { return convert_cart_to_polar(pads[str].xy); }
    double get_Pad_Rotation(std::string str) { return pads[str].rotation; }
    double get_Pad_Radian(std::string str) { return pads[str].getRadian(); }

    // Set function
    void set_WH(double par_width, double par_height) { width = par_width; height = par_height; }
    void set_Center(Cartesian xy) { center = xy; }
    void set_Pad(std::string pad_name, item par_pad) { pads[pad_name] = par_pad; }
    void set_Pads(std::map<std::string, item> par_pads) { pads = par_pads; }
    void set_Pad_Name(std::string par_number, std::string par_name) { pads[par_number].name = par_name; }
    void set_Pad_Location(std::string par_number, Cartesian par_xy) { pads[par_number].xy = par_xy; }
    void set_Pad_Rotation(std::string par_number, double par_rota) { pads[par_number].rotation = par_rota; }

    // Other function
    void calculate_WH();
    void shift_Rotate(Cartesian, double);
};

class Chip
{
private:
    std::map<std::string, Ball> balls;
    std::vector<Die> dice;
    std::map<std::string, relationship> internal_netlist;
    std::map<std::string, relationship> external_netlist;
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
    std::map<std::string, item> get_Balls_Item() const;
    size_t get_Balls_Amount() const { return balls.size(); }
    std::vector<std::string> get_Balls_Number() const;
    std::map<std::string, std::string> get_Balls_Name() const;
    std::map<std::string, Cartesian> get_Balls_Location() const;
    //
    std::string get_Ball_Name(std::string par_number) const { return balls.at(par_number).get_Name(); }
    Cartesian get_Ball_Location(std::string par_number) const { return balls.at(par_number).get_Location(); }
    // additional get function
    std::vector<Cartesian> get_Some_Balls_Location(std::vector<std::string>) const;
    // get dice's information
    std::vector<Die> get_Dice() const { return dice; }
    size_t get_Dice_Amount() const { return dice.size(); }
    //
    Die get_Die(size_t index) const { return dice[index]; }
    size_t get_Die_Pad_Amount(size_t index) const { return dice[index].get_Pads_Amount(); }
    Cartesian get_Die_Center(size_t index) const { return dice[index].get_Center(); }
    Cartesian get_Die_WH(size_t index) const { return Cartesian(dice[index].get_Width(), dice[index].get_Height()); }
    Cartesian get_Die_Pad_Location(std::string par_number) const;
    double get_Die_Pad_Rotation(std::string par_number) const;
    double get_Die_Pad_Radian(std::string par_number);
    // additional get function
    std::vector<Cartesian> get_Pads_Location(std::vector<std::string>) const;
    // get netlist information
    std::map<std::string, relationship> get_All_I_Netlist() const { return internal_netlist; }
    std::map<std::string, relationship> get_All_E_Netlist() const { return external_netlist; }
    /*====================*/

    // Set function
    // set the data of ball
    void insert_Ball(std::string par_ball_number, Ball par_ball) { balls[par_ball_number] = par_ball; }
    // set the data of dice
    void set_Dice_Amount(size_t par_amount) {
        dice.clear();
        for (size_t i = 0; i < par_amount; i++) {
            dice.push_back(Die());
        }
    }
    void set_Die(size_t index, Die par_die) {
        assert(index < dice.size());
        dice[index] = par_die;
    }
    void set_Die_Width_Height(size_t index, double par_width, double par_height) {
        assert(index < dice.size());
        dice[index].set_WH(par_width, par_height);
    }
    void set_Die_Center(size_t index, Cartesian par_pos) {
        assert(index < dice.size());
        dice[index].set_Center(par_pos);
    }
    void set_Die_Width_Height_Center(size_t index, double par_width, double par_height, Cartesian par_center) {
        assert(index < dice.size());
        dice[index] = Die(par_width, par_height, par_center);
    }
    void insert_Die_Pad(size_t index, std::string par_number, item par_pad) {
        while (index >= dice.size())
            dice.push_back(Die());
        dice[index].set_Pad(par_number, par_pad);
    }
    // set the data of netlist
    void set_All_I_Netlist(std::map<std::string, relationship> par_inner_relations) { internal_netlist = par_inner_relations; }
    void set_All_E_Netlist(std::map<std::string, relationship> par_outer_relations) { external_netlist = par_outer_relations; }
    void insert_Internal_Net(std::string par_number, relationship par_relation) { internal_netlist[par_number] = par_relation; }
    void insert_External_Net(std::string par_number, relationship par_relation) { external_netlist[par_number] = par_relation; }
    /*====================*/
    
    // file output
    void output_LP_File(std::ofstream&);
    void output_M_File(std::ofstream&, char*);
    void output_MC_File(std::ofstream&, char*);
    /*====================*/

    // other function
    void calculate_All_Dice_WH() { for (size_t die_index = 0; die_index < dice.size(); die_index++) dice[die_index].calculate_WH(); }
    void shift_Rotate_Die_Pads(size_t die_index, Cartesian par_center, double par_rotation) { dice[die_index].shift_Rotate(par_center, par_rotation); }
    /*====================*/
};

#endif  // CHIP_H
