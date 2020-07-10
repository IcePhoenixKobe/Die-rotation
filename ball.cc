#include"ball.h"

Ball::Ball()
{
    amount = 0;
    name.clear();
    position.clear();
}

Ball::Ball(std::size_t ball_num)
{
    amount = ball_num;
    name.clear();
    position.clear();
    name.resize(amount);
    position.resize(amount);
    for (int i = 0; i < static_cast<int>(position.size()); i++) {
        name[i] = "unknown";
        position[i].x = 0.0;
        position[i].y = 0.0;
    }
}

size_t Ball::get_Index(std::string str) const
{
    for (size_t t = 0; t < name.size(); t++)
        if (name[t] == str)
            return t;
    return -1;
}