#ifndef KERNEL_H
#define KERNEL_H

#include"chip.h"

/* use to classify padGroup 
 * currently we have four directions for Group
 * */
enum class Group{
    UP, DOWN, LEFT, RIGHT
};

typedef struct Item {
	std::string name;
	double x;
	double y;
}item;

/* PadGroup */
struct PadGroup{
    Group group;
    // record the name of pad which is belong to 'group' and sort by 'group'.
    // e.g., if 'group' is equal to Up, then they are sorted according to the X position of pad from left to right.
    std::vector<std::string> pad_name_vec;
    // the position of leftmost and rightmost pad
    double leftmost_pos, rightmost_pos;
    // average height in 'pad_name_vec'
    double avg_height;
};

/* according to the information in 'balls', 'pads' and 'padBallPair', construct the 'PadGroupMap' and 
 * correspond DrawObject such that pad DrawRectangle, ball DrawCircle, pad_to_ball DrawLine
 * */
void initialize(const Chip&, const std::map<std::string, std::map<std::string, item>> &items, 
        const std::map<std::string, std::string> &padBallPair,
        const std::map<std::string, std::string> &padArtificialFingerPair,
        std::map<Group, PadGroup> &padGroupMap);

#endif  // KERNEL_H