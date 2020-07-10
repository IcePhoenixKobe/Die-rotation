#ifndef DIE_H
#define DIE_H
#include"main.h"

class Die
{
private:
    std::size_t pad_num;
    Cartesian center;
    std::vector<std::string> pad_name;
    std::vector<Cartesian> cart_position;
    std::vector<Polar> pol_position;

    void Clear_Pad();
    void Resize_Pad(int);
public:
    Die();
    Die(std::size_t);
    Die(std::size_t, double, double);
    ~Die() {}

    void convert_cart_to_polar();

    size_t get_Pad_Amount() const { return pad_num; }
    Cartesian get_Center() const { return center; }
    std::vector<std::string> get_Pads_Name() const { return pad_name; }
    std::vector<Cartesian> get_Pads_Cart_Position() const { return cart_position; }
    std::vector<Polar> get_Pads_Pol_Position() const { return pol_position; }
    size_t get_Pad_Index(std::string str) const;
    std::string get_Pad_Name(size_t index) const { return pad_name[index]; }
    Cartesian get_Cart_Position(size_t index) const { return cart_position[index]; }
    Polar get_Pol_Position(size_t index) const { return pol_position[index]; }

    void set_Amount(int num) { pad_num = num; }
    void set_Center(Cartesian xy) { center = xy; }
    void set_Pad_Name(std::vector<std::string> die_pad_name) { pad_name = die_pad_name; }
    void set_Cart_Pos(std::vector<Cartesian> cart_pos) { cart_position = cart_pos; }
    void set_Pol_Pos(std::vector<Polar> pol_pos) { pol_position = pol_pos; }
};

#endif  // DIE_H