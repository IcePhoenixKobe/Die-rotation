#ifndef BALL_H
#define BALL_H
#include"main.h"

class Ball
{
private:
    std::size_t amount;
    std::vector<std::string> name;
    std::vector<Cartesian> position;
public:
    Ball();
    Ball(std::size_t);
    ~Ball() {}

    size_t get_Amount() const { return amount; }
    std::vector<std::string> get_All_Name() const { return name; }
    std::vector<Cartesian> get_All_Pos() const { return position; }
    size_t get_Index(std::string str) const;
    std::string get_Name(size_t index) const { return name[index]; }
    Cartesian get_Position(size_t index) const { return position[index]; }

    void set_Amount(int num) { amount = num; }
    void set_All_Name(std::vector<std::string> names) { name = names; }
    void set_All_Pos(std::vector<Cartesian> pos) { position = pos; }
};

#endif  // BALL_H