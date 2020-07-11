#include"die.h"

using namespace std;

/* Default Constructor */
Die::Die(std::size_t die_pad_number)
{
    pad_num = die_pad_number;
    center.x = 0.0;
    center.y = 0.0;
    Clear_Pad();
    Resize_Pad(pad_num);
}

/* Constructor */
Die::Die(std::size_t die_pad_number, double center_x, double center_y)
{
    pad_num = die_pad_number;
    center.x = center_x;
    center.y = center_y;
    Clear_Pad();
    Resize_Pad(pad_num);
}

/* Clear the cart_position and pol_position */
void Die::Clear_Pad()
{
    pad_name.clear();
    cart_position.clear();
    pol_position.clear();
}

/* Resize the cart_position and pol_position, and set all element of 
 * cart_position and pol_position to 0.0.
 * */
void Die::Resize_Pad(size_t size)
{
    pad_name.resize(size);
    cart_position.resize(size);
    pol_position.resize(size);
    for (size_t i = 0; i < size; i++) {
        pad_name[i] = "unknownPad";
        cart_position[i] = Cartesian(0.0, 0.0);
        pol_position[i] = Polar(0.0, 0.0);
    }
}

/* Convert cartesian coordinate system to polar coordinate system */
void Die::convert_cart_to_polar()
{
    double x = 0.0, y = 0.0;
    assert(cart_position.size() == pol_position.size());
    for (size_t i = 0; i < cart_position.size() && i < pol_position.size(); i++)
    {
        //std::cout << "pad " << i + 1 << ":";

        // calculate radius
        pol_position[i].radius = cart_position[i].distance();
        //std::cout << "\tradius = " << pol_position[i].radius;

        // calculate theta
        x /= pol_position[i].radius;
        y /= pol_position[i].radius;
        if (x >= 0 && y >= 0){   //Quadrant I
            pol_position[i].angle = (asin(y) + acos(x)) / 2;
        }
        else if (x < 0 && y >= 0){  //Quadrant II
            pol_position[i].angle = acos(x);
        }
        else if (x < 0 && y < 0){  //Quadrant III
            pol_position[i].angle = acos(x) - 2 * asin(y);
        }
        else if (x >= 0 && y < 0){  //Quadrant IV
            pol_position[i].angle = (2 * M_PI + asin(y));
        }
        //std::cout << "\ttheta = " << pol_position[i].angle << std::endl;
    }
}

/* Use die pad name to get index of (pad_name) vector */
size_t Die::get_Pad_Index(std::string str) const
{
    for (size_t t = 0; t < pad_name.size(); t++)
        if (pad_name[t] == str)
            return t;
    return -1;
}