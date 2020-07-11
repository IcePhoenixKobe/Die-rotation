#include"ball.h"

using namespace std;

/* Default Constructor */
Ball::Ball()
{
    amount = 0;
    name.clear();
    position.clear();
}

/* Constructor */
Ball::Ball(size_t ball_num)
{
    amount = ball_num;
    name.clear();
    position.clear();
    name.resize(amount);
    position.resize(amount);
    for (size_t i = 0; i < position.size(); i++) {
        name[i] = "unknown";
        position[i].x = 0.0;
        position[i].y = 0.0;
    }
}

/* Use BGA ball name to get index of vector */
size_t Ball::get_Ball_Index(string str) const
{
    for (size_t t = 0; t < name.size(); t++)
        if (name[t] == str)
            return t;
    return -1;
}