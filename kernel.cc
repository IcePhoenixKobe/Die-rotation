#include"main.h"

using namespace std;

static const int pad_width = 40;
static const int pad_height = 60;

// extern variables from windowsApp.cpp file
extern map<string, map<string, PhysicalObject>> g_entireObjMap;

// set global parameter with input argument
void check_argument(int argc, char* argv[])
{
    if (argc < 4) {
        cout << "argument at least has drc, pin, and netlist file\n";
        exit(-1);
    }
    else {
        for (int i = 4; i < argc; i++)
        {
            if (strcmp(argv[i], "-m") == 0)
                min_output = false;
            if (strcmp(argv[i], "-PG") == 0)
                ignore_P_G = true;
            if (strcmp(argv[i], "-LP") == 0)
                LP_out = i;
            if (strcmp(argv[i], "-M") == 0)
                M_out = i;
            if (strcmp(argv[i], "-G") == 0)
                GUI = i;
        }
    }
    return;
}

// convert cartesian coordinate system to polar coordinate system
Polar convert_cart_to_polar(Cartesian cart)
{
    Polar return_polar(0.0, 0.0);
    double x = 0.0, y = 0.0;

    // calculate radius
    return_polar.radius = cart.distance();

    // calculate theta
    x = cart.x / return_polar.radius;
    y = cart.y / return_polar.radius;
    if (x >= 0 && y >= 0){   //Quadrant I
        return_polar.angle = (asin(y) + acos(x)) / 2;
    }
    else if (x < 0 && y >= 0){  //Quadrant II
        return_polar.angle = acos(x);
    }
    else if (x < 0 && y < 0){  //Quadrant III
        return_polar.angle = acos(x) - 2 * asin(y);
    }
    else if (x >= 0 && y < 0){  //Quadrant IV
        return_polar.angle = (2 * M_PI + asin(y));
    }
    return return_polar;
}

// calculate center of gravity by cartesian coordinate system
Cartesian CG(vector<Cartesian> pos)
{
    if (pos.size() == 1) return pos[0];
    Cartesian cg(0.0, 0.0);
    for (size_t i = 0; i < pos.size(); i++)
    {
        cg.x += pos[i].x;
        cg.y += pos[i].y;
    }
    cg.x /= static_cast<double>(pos.size());
    cg.y /= static_cast<double>(pos.size());
    return cg;
}

// point rotation
Cartesian shift_rotation(Cartesian point, Cartesian ori_center, Cartesian new_center, double rotation)
{
    return Cartesian(
            (
                (point.x - ori_center.x) * cos(rotation) 
                - (point.y - ori_center.y) * sin(rotation)
            ) + new_center.x
        , (
                (point.x - ori_center.x) * sin(rotation) 
                + (point.y - ori_center.y) * cos(rotation)
            ) + new_center.y
    );
}

bool ignore_Power_Ground(string str)
{
    if (ignore_P_G && (
						str.find("GND") != string::npos || 
                        str.find("NET_G") != string::npos || 
                        str.find("NET_P") != string::npos || 
                        str.find("NET_V") != string::npos || 
						str.find("VCC") != string::npos || 
						str.find("VDD") != string::npos || 
						str.find("VPP") != string::npos || 
						str.find("VSS") != string::npos
                    )
        )
	{
		cout << "ignore " << str << endl;
		return true;
	}
    else
		return false;
}

void dataTransfer(){
    
    map<string, map<string, item>> items;   // record object name and location
    map<Group, PadGroup> padGroupMap;
    items.clear();
    padGroupMap.clear();
    
    cout << "----------------\n";
    initialize(items, padGroupMap);
}

void initialize(map<string, map<string, item>> &items, 
        map<Group, PadGroup> &padGroupMap)
{
    item temp_item;
    map<string, item> pads, balls;
    
    temp_item = item();
    pads.clear();
    balls.clear();
/*
    // Draw Line
    {
        // Draw Inner Netlist Line
        vector<relationship> inner_nets = chip->get_All_I_Netlist();
        for (size_t i = 0; i < inner_nets.size(); i++) {
            // draw single line
            if (inner_nets[i].dice_pads1_index.second.size() == 1 && inner_nets[i].dice_pads2_index.second.size() == 1)
            {
                Point pad1_center(inner_nets[i].pad1_car.x, -(inner_nets[i].pad1_car.y));
                Point pad2_center(inner_nets[i].pad2_car.x, -(inner_nets[i].pad2_car.y));
                DrawContainer NetlistContainer(
                        (pad1_center + pad2_center) / 2.0,      // the middle point between pad1_center and pad2_center
                        fabs(pad1_center.x - pad2_center.x),    // the x distance between pad1_center and pad2_center
                        fabs(pad1_center.y - pad2_center.y)     // the y distance between pad1_center and pad2_center
                    );
                DrawLine padToPadLine(
                        pad1_center,              // start point is pad1_center
                        pad2_center,             // end point is pad2_center
                        Color(1.0, 0.0, 1.0)    // ???color
                    );
                
                NetlistContainer.push_back(&padToPadLine);
            
                // constructor the 'PhysicalObject' to pad and store it
                g_entireObjMap["InnerNetlistMap"][inner_nets[i].name] = PhysicalObject(
                        inner_nets[i].name,        // the name of the netlist
                        NetlistContainer.getCenterPoint(),  // the center of the 
                        NetlistContainer                                    // correspond DrawContainer
                        );
            }
            // draw multi line
            else {
                // the border location of the pads
                double maxX = -1.0 * chip->drc.packageSize.x, minX = chip->drc.packageSize.x, 
                                maxY = -1.0 * chip->drc.packageSize.y, minY = chip->drc.packageSize.y;
                {
                    for (size_t j = 0; j < inner_nets[i].dice_pads1_index.second.size(); j++) {
                        // get the center point of the pad
                        Point padCenter(
                            chip->get_Die_Pad_Location(inner_nets[i].dice_pads1_index.first - 1, inner_nets[i].dice_pads1_index.second[j]).x, 
                            chip->get_Die_Pad_Location(inner_nets[i].dice_pads1_index.first - 1, inner_nets[i].dice_pads1_index.second[j]).y);
                        // update maxX
                        if(maxX < padCenter.x) maxX = padCenter.x;
                        // update minX
                        if(minX > padCenter.x) minX = padCenter.x;
                        // update maxY
                        if(maxY < padCenter.y) maxY = padCenter.y;
                        // update minY
                        if(minY > padCenter.y) minY = padCenter.y;
                    }
                    for (size_t j = 0; j < inner_nets[i].dice_pads2_index.second.size(); j++) {
                        // get the center point of the pad
                        Point padCenter(
                            chip->get_Die_Pad_Location(inner_nets[i].dice_pads2_index.first, inner_nets[i].dice_pads2_index.second[j]).x, 
                            chip->get_Die_Pad_Location(inner_nets[i].dice_pads2_index.first, inner_nets[i].dice_pads2_index.second[j]).y);
                        // update maxX
                        if(maxX < padCenter.x) maxX = padCenter.x;
                        // update minX
                        if(minX > padCenter.x) minX = padCenter.x;
                        // update maxY
                        if(maxY < padCenter.y) maxY = padCenter.y;
                        // update minY
                        if(minY > padCenter.y) minY = padCenter.y;
                    }
                }
                
                DrawContainer NetlistContainer(
                        Point((maxX + minX) / 2.0, (maxY + minY) / 2.0),    // the middle point of pads
                        fabs(maxX - minX),    // the x distance of pads
                        fabs(maxY - minY)     // the y distance of pads
                    );
                if (inner_nets[i].dice_pads1_index.second.size() != 1)
                {
                    for (size_t j = 0; j < inner_nets[i].dice_pads1_index.second.size(); j++) {
                        DrawLine padToCGLine(
                            Point(    // start point is one of pad of pads1
                            chip->get_Die_Pad_Location(inner_nets[i].dice_pads1_index.first - 1, inner_nets[i].dice_pads1_index.second[j]).x, 
                            -(chip->get_Die_Pad_Location(inner_nets[i].dice_pads1_index.first - 1, inner_nets[i].dice_pads1_index.second[j]).y)), 
                            Point(inner_nets[i].pad1_car.x, -(inner_nets[i].pad1_car.y)),  // end point is pads1 CG
                            Color(1.0, 0.2, 0.2)    // ???color
                        );
                        NetlistContainer.push_back(&padToCGLine);
                    }
                }
                DrawLine CGToCGLine(
                        Point(inner_nets[i].pad1_car.x, -(inner_nets[i].pad1_car.y)), // start point is pads1 CG
                        Point(inner_nets[i].pad2_car.x, -(inner_nets[i].pad2_car.y)),  // end point is pads2 CG
                        Color(1.0, 0.2, 0.2)    // ???color
                    );
                NetlistContainer.push_back(&CGToCGLine);
                if (inner_nets[i].dice_pads2_index.second.size() != 1)
                {
                    for (size_t j = 0; j < inner_nets[i].dice_pads2_index.second.size(); j++) {
                        DrawLine padToCGLine(
                            Point(    // start point is one of pad of pads2
                            chip->get_Die_Pad_Location(inner_nets[i].dice_pads2_index.first - 1, inner_nets[i].dice_pads2_index.second[j]).x, 
                            -(chip->get_Die_Pad_Location(inner_nets[i].dice_pads2_index.first - 1, inner_nets[i].dice_pads2_index.second[j]).y)), 
                            Point(inner_nets[i].pad2_car.x, -(inner_nets[i].pad2_car.y)),  // end point is pad1s CG
                            Color(1.0, 0.2, 0.2)    // ???color
                        );
                        NetlistContainer.push_back(&padToCGLine);
                    }
                }

                // constructor the 'PhysicalObject' to pad and store it
                g_entireObjMap["InnerNetlistMap"][inner_nets[i].name] = PhysicalObject(
                        inner_nets[i].name,        // the name of the netlist
                        NetlistContainer.getCenterPoint(),  // the center of the 
                        NetlistContainer                                    // correspond DrawContainer
                        );
            }
        }
        // Draw Outer Netlist Line
        vector<OuterRelationship> outer_nets = chip->get_All_O_Netlist();
        for (size_t i = 0; i < outer_nets.size(); i++) {
            // get amount of pads
            size_t pads_amount = 0;
            for (size_t j = 0; j < outer_nets[i].dice_pads_index.size(); j++)
                pads_amount += outer_nets[i].dice_pads_index[j].size();
            
            // draw single line
            if (outer_nets[i].balls_index.size() == 1 && pads_amount == 1)
            {
                Point ball_center(outer_nets[i].ball_car.x, -(outer_nets[i].ball_car.y));
                Point pad_center(outer_nets[i].pad_car.x, -(outer_nets[i].pad_car.y));
                DrawContainer NetlistContainer(
                        (ball_center + pad_center) / 2.0,      // the middle point between pad1_center and pad2_center
                        fabs(ball_center.x - pad_center.x),    // the x distance between pad1_center and pad2_center
                        fabs(ball_center.y - pad_center.y)     // the y distance between pad1_center and pad2_center
                    );
                DrawLine padToPadLine(
                        ball_center,              // start point is pad1_center
                        pad_center,             // end point is pad2_center
                        Color(0.0, 1.0, 1.0)    // ???color
                    );
                
                NetlistContainer.push_back(&padToPadLine);
            
                // constructor the 'PhysicalObject' to pad and store it
                g_entireObjMap["OuterNetlistMap"][outer_nets[i].name] = PhysicalObject(
                        outer_nets[i].name,        // the name of the netlist
                        NetlistContainer.getCenterPoint(),  // the center of the 
                        NetlistContainer                                    // correspond DrawContainer
                        );
            }
            // draw multi line
            else {
                // the border location of the balls and pads
                double maxX = -1.0 * chip->drc.packageSize.x, minX = chip->drc.packageSize.x, 
                                maxY = -1.0 * chip->drc.packageSize.y, minY = chip->drc.packageSize.y;
                {
                    for (size_t j = 0; j < outer_nets[i].balls_index.size(); j++) {
                        // get the center point of the pad
                        Point padCenter(
                            chip->get_Ball_Location(outer_nets[i].balls_index[j]).x, 
                            chip->get_Ball_Location(outer_nets[i].balls_index[j]).y);
                        // update maxX
                        if(maxX < padCenter.x) maxX = padCenter.x;
                        // update minX
                        if(minX > padCenter.x) minX = padCenter.x;
                        // update maxY
                        if(maxY < padCenter.y) maxY = padCenter.y;
                        // update minY
                        if(minY > padCenter.y) minY = padCenter.y;
                    }
                    for (size_t j = 0; j < outer_nets[i].dice_pads_index.size(); j++) {
                        for (size_t k = 0; k < outer_nets[i].dice_pads_index[j].size(); k++) {
                            // get the center point of the pad
                            Point padCenter(
                                chip->get_Die_Pad_Location(j, outer_nets[i].dice_pads_index[j][k]).x, 
                                chip->get_Die_Pad_Location(j, outer_nets[i].dice_pads_index[j][k]).y);
                            // update maxX
                            if(maxX < padCenter.x) maxX = padCenter.x;
                            // update minX
                            if(minX > padCenter.x) minX = padCenter.x;
                            // update maxY
                            if(maxY < padCenter.y) maxY = padCenter.y;
                            // update minY
                            if(minY > padCenter.y) minY = padCenter.y;
                        }
                    }
                }
                
                DrawContainer NetlistContainer(
                        Point((maxX + minX) / 2.0, (maxY + minY) / 2.0),    // the middle point of pads
                        fabs(maxX - minX),    // the x distance of pads
                        fabs(maxY - minY)     // the y distance of pads
                    );
                if (pads_amount > 1)
                {
                    for (size_t j = 0; j < outer_nets[i].dice_pads_index.size(); j++) {
                        for (size_t k = 0; k < outer_nets[i].dice_pads_index[j].size(); k++) {
                            DrawLine padToCGLine(
                                Point(    // start point is one of pad of pads1
                                chip->get_Die_Pad_Location(j, outer_nets[i].dice_pads_index[j][k]).x, 
                                -(chip->get_Die_Pad_Location(j, outer_nets[i].dice_pads_index[j][k]).y)), 
                                Point(outer_nets[i].pad_car.x, -(outer_nets[i].pad_car.y)),  // end point is pads1 CG
                                Color(1.0, 0.5, 0.5)    // ???color
                            );
                            NetlistContainer.push_back(&padToCGLine);
                        }
                    }
                }
                DrawLine CGToCGLine(
                        Point(outer_nets[i].pad_car.x, -(outer_nets[i].pad_car.y)), // start point is pads CG
                        Point(outer_nets[i].ball_car.x, -(outer_nets[i].ball_car.y)),  // end point is ball CG
                        Color(1.0, 0.5, 0.5)    // ???color
                    );
                NetlistContainer.push_back(&CGToCGLine);
                if (outer_nets[i].balls_index.size() > 1)
                {
                    for (size_t j = 0; j < outer_nets[i].balls_index.size(); j++) {
                        DrawLine padToCGLine(
                            Point(    // start point is one of pad of pads2
                            chip->get_Ball_Location(outer_nets[i].balls_index[j]).x, 
                            -(chip->get_Ball_Location(outer_nets[i].balls_index[j]).y)), 
                            Point(outer_nets[i].ball_car.x, -(outer_nets[i].ball_car.y)),  // end point is pad1s CG
                            Color(1.0, 0.5, 0.5)    // ???color
                        );
                        NetlistContainer.push_back(&padToCGLine);
                    }
                }

                // constructor the 'PhysicalObject' to pad and store it
                g_entireObjMap["OuterNetlistMap"][outer_nets[i].name] = PhysicalObject(
                        outer_nets[i].name,        // the name of the netlist
                        NetlistContainer.getCenterPoint(),  // the center of the 
                        NetlistContainer                                    // correspond DrawContainer
                    );
            }
        }
    }
    
    //chip->Original_Dice_Pads();

    // 1. insert all pads
    for (size_t i = 0; i < chip->get_Dice_Amount(); i++)
    {
        pads.clear();
        Die die = chip->get_Die(i);
        for (size_t j = 0; j < die.get_Pads_Amount(); j++)
        {
            temp_item.name = die.get_Pad_Name(j);
            temp_item.xy = Cartesian(die.get_Pad_Cart_Location(j).x, -die.get_Pad_Cart_Location(j).y);
            pads[temp_item.name] = temp_item;
        }
        stringstream ss;
        ss << i + 1;
        string item_name("die");
        item_name += ss.str();
	    items[item_name.c_str()] = pads;
    }

    // 2. insert all BGA balls
    for (size_t t = 0; t < chip->get_Balls_Amount(); t++)
    {
        temp_item.name = chip->get_Ball_Name(t);
        temp_item.xy = Cartesian(chip->get_Ball_Location(t).x, -chip->get_Ball_Location(t).y);
        balls[temp_item.name] = temp_item;
    }
	items["ball"] = balls;

    // Draw Dice
    for (size_t die_index = 0; die_index < chip->get_Dice_Amount(); die_index++)
    {
        // the border location of the die
        double maxX = -1.0 * chip->drc.packageSize.x, minX = chip->drc.packageSize.x, 
                        maxY = -1.0 * chip->drc.packageSize.y, minY = chip->drc.packageSize.y;
        
        stringstream ss;
        ss << die_index + 1;
        string item_name("die");
        item_name += ss.str();
        const map<string, item> pads = items.at(item_name);
        cout << "size of " << item_name << ": " << pads.size() << endl;
        
        // find the border location of the pads in "pad + die_index"
        for(map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); ++map_it){
            // get the center point of the pad
            Point padCenter(map_it->second.xy.x, map_it->second.xy.y);
            // update maxX
            if(maxX < padCenter.x) maxX = padCenter.x;
            // update minX
            if(minX > padCenter.x) minX = padCenter.x;
            // update maxY
            if(maxY < padCenter.y) maxY = padCenter.y;
            // update minY
            if(minY > padCenter.y) minY = padCenter.y;
        }

        // print out the border location of the die
        cout << "die " << die_index + 1 << ": " << endl
                  << "  minX: " << minX << endl
                  << "  maxX: " << maxX << endl
                  << "  minY: " << minY << endl
                  << "  maxY: " << maxY << endl;
        // construct the 'PhysicalObject Die' with previous found border (maxX, minX, maxY, minY)
        double DieWidth = maxX - minX + 2 * chip->drc.spacing;
        double DieHeight = maxY - minY + 2 * chip->drc.spacing;
        DrawContainer DieContainer(
                Point(chip->get_Die_Center(die_index).x, -chip->get_Die_Center(die_index).y),   // the center of the die
                DieWidth, 
                DieHeight
            );
        DrawRectangle DieRect(
                Point(chip->get_Die_Center(die_index).x, -chip->get_Die_Center(die_index).y),   // the center of the die
                Color(1.0, 1.0, 0.0),   // yellow
                DieWidth,   // the width of the die
                DieHeight,  // the height of the die
                cairo_stroke
            );
        
        DieContainer.push_back(&DieRect);

        // according to the location of the pad, put it to the suitable 'PadGroup'
        for(map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); ++map_it)
        {
            // get the center point of the pad
            Point padCenter(map_it->second.xy.x, map_it->second.xy.y);
            // the vertical and horizontal 'Group'
            Group groupV, groupH;
            // the ratio of pad width/height to maximum width/height
            double nearV, nearH;

            // calculate the ratio
            nearV = (maxY - padCenter.y) / (maxY - minY);
            nearH = (maxX - padCenter.x) / (maxX - minX);
            
            // if 'nearV' is less than or equal to 0.5, the y location of pad is near to 'Group::DOWN'
            // otherwise it is near to 'Group::UP'
            if(nearV <= 0.5){
                groupV = Group::DOWN;
            }
            else{
                groupV = Group::UP;
                nearV = 1.0 - nearV;
            }

            // if 'nearH' is less than or equal to 0.5, the x location of pad is near to 'Group::RIGHT'
            // otherwise it is near to 'Group::LEFT'
            if(nearH <= 0.5){
                groupH = Group::RIGHT;
            }
            else{
                groupH = Group::LEFT;
                nearH = 1.0 - nearH;
            }
            
            // if 'nearV' is greater than 'nearH', the pad is belong to 'groupH', otherwise it is belong to 'groupV'
            if(nearV > nearH) groupV = groupH;
            
            // create 'PhysicalObject' Object to each pad and store it into 'g_entireObjMap' with key "padMap"
            DrawContainer padContainer(
                    padCenter, 
                    pad_width, 
                    pad_height
                    );
            DrawRectangle padRect(
                    padCenter,              // the center of the pad
                    Color(1.0, 1.0, 0.0),   // yellow
                    pad_width,              // the width of the pad
                    pad_height,             // the height of the pad
                    cairo_stroke            // use stroke drawMethod
                    );

            
            // if Group is 'LEFT' rotate counterclockwise degree 90
            if(groupV == Group::LEFT)
                padContainer.rotate(-1.57);
            // if Group is 'RIGHT' rotate clockwise degree 90
            else if(groupV == Group::RIGHT)
                padContainer.rotate(1.57);
            
            // original rotation
            if (chip->get_Die_Index(map_it->first) == 0 && ori_rotas.size() > 0) padContainer.rotate(-(chip->get_Die_Pad_Rotation(die_index, map_it->first)-(ori_rotas[0] + GOD_Rotation)));
            else if (chip->get_Die_Index(map_it->first) == 1 && ori_rotas.size() > 1) padContainer.rotate(-(chip->get_Die_Pad_Rotation(die_index, map_it->first)-(ori_rotas[1] + GOD_GOD_Rotation)));
            else padContainer.rotate(-chip->get_Die_Pad_Rotation(die_index, map_it->first));

            // put 'padRect' into 'padContainer'
            padContainer.push_back(&padRect);
            DieContainer.push_back(&padContainer);
        }
        
        if (die_index == 0) {
            if (ori_rotas.size() > 0) DieContainer.rotate(-(ori_rotas[0] + GOD_Rotation));
        }
        else if (die_index == 1) {
            if (ori_rotas.size() > 1) DieContainer.rotate(-(ori_rotas[1] +  GOD_GOD_Rotation));
        }
        
        // constructor the 'PhysicalObject' to pad and store it
        g_entireObjMap["DiceMap"][item_name] = PhysicalObject(
                item_name,    // the name of the die
                DieContainer.getCenterPoint(),  // the center of the die
                DieContainer                                    // correspond DrawContainer
                );
    }

    // Draw BGA Balls
    // create 'PhysicalObject' object to each ball and store it into 'g_entireObjMap' with key 'ballMap'
    const map<string, item> all_balls = items.at("ball");
    for(map<string, item>::const_iterator map_it = all_balls.begin(); map_it != all_balls.end(); ++map_it)
    {
        // the name of the ball
        string ballName = map_it->first;
        // the center of the ball
        Point ballCenter(map_it->second.xy.x, map_it->second.xy.y);
        
        // the 'DrawContainer' of the ball
        DrawContainer ballContainer(
                ballCenter, 
                chip->drc.ballDiameter, 
                chip->drc.ballDiameter
            );
        // the draw pattern of the ball
        DrawCircle ballCircle(
                ballCenter,                 // the center of the ball
                Color(0.0, 0.0, 1.0),   // blue
                chip->drc.ballDiameter / 2.0,   // the radius of the ball
                cairo_stroke            // use stroke drawMethod
            );
        
        // put 'ballCircle' into 'ballContainer'
        ballContainer.push_back(&ballCircle);
        // scale the circle
        ballCircle.scale(0.6, 0.6);
        // change the DrawMethod with cairo_fill
        ballCircle.setDrawMethod(cairo_fill);
        // put 'ballCircle' into 'ballContainer'
        ballContainer.push_back(&ballCircle);

        // construct the 'PhysicalObject' with ball and store it
        g_entireObjMap["BallMap"][ballName] = PhysicalObject(
                ballName,                       // the name of the ball
                ballContainer.getCenterPoint(), // the center of the ball
                ballContainer                   // the DrawContainer of the ball
            );
    }
    
    // Draw Substrate
    // the 'DrawContainer' of the substrate
    DrawContainer SubstrateContainer(
            Point(), 
            chip->drc.packageSize.x, 
            chip->drc.packageSize.y
        );
    // the draw pattern of the ball
    DrawRectangle SubstrateRect(
            Point(),                 // the center of the substrate
            Color(0.0, 0.0, 1.0),   // blue
            chip->drc.packageSize.x,   // the width of the substrate
            chip->drc.packageSize.y,   // the height of the substrate
            cairo_stroke            // use stroke drawMethod
        );
    SubstrateContainer.push_back(&SubstrateRect);
    
    // construct the 'PhysicalObject' with substrate and store it
    g_entireObjMap["SubstrateMap"]["substrate"] = PhysicalObject(
            "substrate",                       // the name of the substrate
            SubstrateContainer.getCenterPoint(), // the center of the substrate
            SubstrateContainer                   // the DrawContainer of the substrate
        );
*/
    //double artificialDistance = 0.0;
    //double totalPadToFingerDistance = 0.0;
    //double totalFingerToBallDistance = 0.0;

    /*for(map<string, PhysicalObject>::iterator map_it = g_entireObjMap["padMap"].begin(); map_it != g_entireObjMap["padMap"].end(); ++map_it){
        const PhysicalObject &artificialFinger = g_entireObjMap["artificialFingerMap"][padArtificialFingerPair.at(map_it->first)];
        const PhysicalObject &ball = g_entireObjMap["ballMap"][padBallPair.at(map_it->first)];
        double padToFingerDistance = distance(map_it->second, artificialFinger);
        double fingerToBallDistance = distance(artificialFinger, ball);
        totalPadToFingerDistance += padToFingerDistance;
        totalFingerToBallDistance += fingerToBallDistance;
        cout << padToFingerDistance << " " << fingerToBallDistance << endl;
    }
    cout << "totalPadToFingerDistance: " << totalPadToFingerDistance << endl;
    cout << "totalFingerToBallDistance: " << totalFingerToBallDistance << endl;*/
}