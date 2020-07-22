#include"main.h"

using namespace std;

static const double CG_radius = 35;
static const double pad_width = 43;
static const double pad_height = 43;

// extern variables from windowsApp.cpp file
extern map<string, map<string, PhysicalObject>> g_entireObjMap;

pair<Cartesian, Cartesian> Relationship::get_CWH()
{
    double maxX = numeric_limits<double>::min(), minX = numeric_limits<double>::max(), 
                    maxY = numeric_limits<double>::min(), minY = numeric_limits<double>::max();
    for (size_t index = 0; index < pins1.size(); index++) {
        Cartesian pin_location(0.0, 0.0);
        if (pins1_number == 0) pin_location = Cartesian(chip->get_Ball_Location(pins1[index]));
        else pin_location = Cartesian(chip->get_Die_Pad_Location(pins1[index]));
        if(maxX < pin_location.x) maxX = pin_location.x;  // update maxX
        if(minX > pin_location.x) minX = pin_location.x;  // update minX
        if(maxY < pin_location.y) maxY = pin_location.y;  // update maxY
        if(minY > pin_location.y) minY = pin_location.y;  // update minY
    }
    for (size_t index = 0; index < pins2.size(); index++) {
        Cartesian pin_location(chip->get_Die_Pad_Location(pins2[index]));
        if(maxX < pin_location.x) maxX = pin_location.x;  // update maxX
        if(minX > pin_location.x) minX = pin_location.x;  // update minX
        if(maxY < pin_location.y) maxY = pin_location.y;  // update maxY
        if(minY > pin_location.y) minY = pin_location.y;  // update minY
    }
    return make_pair<Cartesian, Cartesian>(Cartesian((maxX + minX) / 2.0, (maxY + minY) / 2.0), Cartesian(maxX - minX, maxY - minY));
}

// set global parameter with input argument
void check_argument(int argc, char* argv[]) {
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
Polar convert_cart_to_polar(Cartesian cart, Cartesian center) {
    Polar return_polar(0.0, 0.0);
    double x = 0.0, y = 0.0;

    // calculate radius
    cart.x -= center.x;
    cart.y -= center.y;
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
    return_polar.angle = return_polar.angle * 180.0 / M_PI;
    return return_polar;
}

// calculate center of gravity by cartesian coordinate system
Cartesian CG(vector<Cartesian> pos) {
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

// point rotation (rotation is radian)
Cartesian shift_rotation(Cartesian point, Cartesian ori_center, Cartesian new_center, double rotation) {
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

bool ignore_Power_Ground(string str) {
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
        ) {
		cout << "ignore " << str << endl;
		return true;
	}
    else {
		return false;
    }
}

void dataTransfer() {
    map<string, map<string, relationship>> netlist;
    map<string, item> balls;
    vector<map<string, item>> dice;
    vector<pair<Cartesian,Cartesian>> dice_center_WH;
    
    netlist.clear();
    balls.clear();
    dice.clear();
    dice_center_WH.clear();

    // get all BGA balls data
    balls = chip->get_Balls_Item();

    /*Die die1 = chip->get_Die(0);
    map<string, item> die1_pads_change = die1.get_Pads();
    for (map<string, item>::iterator map_it = die1_pads_change.begin(); map_it != die1_pads_change.end(); map_it++) {
        Polar pad_pol = convert_cart_to_polar(map_it->second.xy, chip->get_Die_Center(0));
        map_it->second.xy.x = (pad_pol.radius + 1500) * cos(pad_pol.get_Radian());
        map_it->second.xy.y = (pad_pol.radius + 1500) * sin(pad_pol.get_Radian());
    }
    die1.set_Pads(die1_pads_change);
    chip->set_Die(0, die1);*/

    Die temp_die;
    for (size_t die_index = 0; die_index < chip->get_Dice_Amount(); die_index++) {
        temp_die = chip->get_Die(die_index);

        // Do some change???

        // get die data
        dice_center_WH.push_back(pair<Cartesian, Cartesian>(chip->get_Die_Center(die_index), chip->get_Die_WH(die_index)));
        dice.push_back(temp_die.get_Pads());
    }

    // get internal and external netlist
    netlist["internal"] = chip->get_All_I_Netlist();
    netlist["external"] = chip->get_All_E_Netlist();
    
    cout << "----------------\n";
    initialize(balls, dice_center_WH, dice, netlist);
}

void initialize(map<string, item>& balls,
                             vector<pair<Cartesian, Cartesian>>& dice_CWH,
                             vector<map<string, item>>& dice,
                             map<string, map<string, relationship>>& netlist) {
    // Draw Substrate
    {
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
    }

    // Draw BGA Balls
    for(map<string, item>::const_iterator map_it = balls.begin(); map_it != balls.end(); ++map_it) {
        // the name of the ball
        string ballName = map_it->first;
        // the center of the ball
        Point ballCenter(map_it->second.xy.x, -map_it->second.xy.y);
        
        // the 'DrawContainer' of the ball
        DrawContainer ballContainer(
                ballCenter, 
                chip->drc.ballDiameter, 
                chip->drc.ballDiameter
            );
        // the draw pattern of the ball
        DrawCircle ballCircle(
                ballCenter,                                     // the center of the ball
                Color(0.749, 0.0, 1.0),                 // purple
                chip->drc.ballDiameter / 2.0,   // the radius of the ball
                cairo_stroke                                    // use stroke drawMethod
            );
        
        // put 'ballCircle' into 'ballContainer'
        ballContainer.push_back(&ballCircle);
        // draw smaller solid ball
        /*// scale the circle
        ballCircle.scale(0.6, 0.6);
        // change the DrawMethod with cairo_fill
        ballCircle.setDrawMethod(cairo_fill);
        // put 'ballCircle' into 'ballContainer'
        ballContainer.push_back(&ballCircle);*/

        // construct the 'PhysicalObject' with BGA balls and store it
        g_entireObjMap["BallMap"][ballName] = PhysicalObject(
                ballName,                       // the name of the ball
                ballContainer.getCenterPoint(), // the center of the ball
                ballContainer                   // the DrawContainer of the ball
            );
    }

    // Draw Dice
    for (size_t die_index = 0; die_index < chip->get_Dice_Amount(); die_index++) {
        stringstream ss;
        ss << die_index + 1;
        string item_number("die");
        item_number += ss.str();
        
        // the 'DrawContainer' of the die
        DrawContainer DieContainer(
                Point(dice_CWH[die_index].first.x, -dice_CWH[die_index].first.y),   // the center of the die
                dice_CWH[die_index].second.x,   // die width
                dice_CWH[die_index].second.y    // die height
            );
        DrawRectangle DieRect(
                Point(dice_CWH[die_index].first.x, -dice_CWH[die_index].first.y),   // the center of the die
                Color(0.0, 1.0, 1.0),   // cyan
                dice_CWH[die_index].second.x,   // the width of the die
                dice_CWH[die_index].second.y,  // the height of the die
                cairo_stroke
            );
        
        DieContainer.push_back(&DieRect);

        // Draw Die Pads
        for(map<string, item>::iterator map_it = dice[die_index].begin(); map_it != dice[die_index].end(); ++map_it)
        {
            // get the center point of the die
            Point padCenter(map_it->second.xy.x, -map_it->second.xy.y);
            
            // create 'PhysicalObject' Object to each pad and store it into 'g_entireObjMap' with key "padMap"
            DrawContainer padContainer(
                    padCenter, 
                    pad_width, 
                    pad_height
                    );
            DrawRectangle padRect(
                    padCenter,              // the center of the pad
                    Color(0.0, 1.0, 1.0),   // cyan
                    pad_width,              // the width of the pad
                    pad_height,             // the height of the pad
                    cairo_stroke            // use stroke drawMethod
                    );
            
            // rotate
            padContainer.rotate(-map_it->second.getRadian());

            // put 'padRect' into 'padContainer'
            padContainer.push_back(&padRect);
            DieContainer.push_back(&padContainer);
        }

        // X ray
        /*{
            DrawLine RTToLBLine(
                    Point(dice_CWH[die_index].first.x + dice_CWH[die_index].second.x / 2,
                        -dice_CWH[die_index].first.y - dice_CWH[die_index].second.y / 2),              // start point is right-top
                    Point(dice_CWH[die_index].first.x - dice_CWH[die_index].second.x / 2,
                        -dice_CWH[die_index].first.y + dice_CWH[die_index].second.y / 2),              // start point is left-bottom
                    Color(1.0, 0.0, 0.0)    // red
                );
            DieContainer.push_back(&RTToLBLine);
            DrawLine LTToRBLine(
                    Point(dice_CWH[die_index].first.x - dice_CWH[die_index].second.x / 2,
                        -dice_CWH[die_index].first.y - dice_CWH[die_index].second.y / 2),              // start point is left-top
                    Point(dice_CWH[die_index].first.x + dice_CWH[die_index].second.x / 2,
                        -dice_CWH[die_index].first.y + dice_CWH[die_index].second.y / 2),              // start point is right-bottom
                    Color(1.0, 0.0, 0.0)    // red
                );
            DieContainer.push_back(&LTToRBLine);
        }*/
        
        // move and rotate
        if (die_index == 0) {
            if (GOD_Center.x != 0.0 && GOD_Center.y != 0.0)
                DieContainer.translate(GOD_Center.x, -GOD_Center.y);
            if (GOD_Rotation != 0.0)
                DieContainer.rotate(-(GOD_Rotation * M_PI / 180.0));
            // change die location and rotation for draw netlist
            chip->shift_Rotate_Die_Pads(0, Cartesian(GOD_Center.x, GOD_Center.y), GOD_Rotation);
        }
        else if (die_index == 1) {
            if (GOD_GOD_Center.x != 0.0 && GOD_GOD_Center.y != 0.0)
                DieContainer.translate(GOD_GOD_Center.x, -GOD_GOD_Center.y);
            if (GOD_GOD_Rotation != 0.0)
                DieContainer.rotate(-(GOD_GOD_Rotation) * M_PI / 180.0);
            // change die location and rotation for draw netlist
            chip->shift_Rotate_Die_Pads(1, Cartesian(GOD_GOD_Center.x, GOD_GOD_Center.y), GOD_GOD_Rotation);
        }

        // constructor the 'PhysicalObject' to die and store it
        g_entireObjMap["DiceMap"][item_number] = PhysicalObject(
                item_number,    // the name of the die
                DieContainer.getCenterPoint(),  // the center of the die
                DieContainer                                        // correspond DrawContainer
                );
    }
    
    // Draw Line
    {
        map<string, relationship> nets;
        nets.clear();
        // Draw Inner Netlist Line
        nets = netlist["internal"];
        for (map<string, relationship>::iterator map_it = nets.begin(); map_it != nets.end(); map_it++) {
            // calculate nets size
            pair<Cartesian, Cartesian> CWH = map_it->second.get_CWH();
            // the 'DrawContainer' of the net
            DrawContainer NetContainer(
                        Point(CWH.first.x, CWH.first.y),      // the middle point between pins1 and pins2
                        CWH.second.x,    // the x distance between pins1 and pins2
                        CWH.second.y     // the y distance between pins1 and pins2
                    );
            
            // draw pads1 to CG of pads1
            Cartesian pads1_CG(CG(chip->get_Pads_Location(map_it->second.pins1)));
            Point pads1_CG_center(pads1_CG.x, -pads1_CG.y);
            Cartesian pads2_CG(CG(chip->get_Pads_Location(map_it->second.pins2)));
            Point pads2_CG_center(pads2_CG.x, -pads2_CG.y);
            // multi balls
            if (map_it->second.pins1.size() > 1) {
                for (size_t index = 0; index < map_it->second.pins1.size(); index++) {
                    Point ball_center(chip->get_Ball_Location(map_it->second.pins1[index]).x, -(chip->get_Ball_Location(map_it->second.pins1[index]).y));
                    DrawLine PinToCGLine(
                            ball_center,                        // start point is pins1[index] center
                            pads1_CG_center,             // end point is pins1 CG center
                            Color(1.0, 0.247, 0.0)          // orange red
                        );
                    NetContainer.push_back(&PinToCGLine);
                }
                DrawCircle CGCircle (
                        pads1_CG_center, 
                        Color(1.0, 0.247, 0.0),     // orange red
                        CG_radius,                          // the radius of the CG
                        cairo_fill                              // use file drawMethod
                    );
                NetContainer.push_back(&CGCircle);
            }
            // multi pads
            if (map_it->second.pins2.size() > 1) {
                for (size_t index = 0; index < map_it->second.pins2.size(); index++) {
                    Point pad_center(chip->get_Die_Pad_Location(map_it->second.pins2[index]).x, -(chip->get_Die_Pad_Location(map_it->second.pins2[index]).y));
                    DrawLine PinToCGLine(
                            pads2_CG_center,        // start point is pins2 CG center
                            pad_center,                     // end point is pins2[index] center
                            Color(1.0, 0.247, 0.0)  // orange red
                        );
                    NetContainer.push_back(&PinToCGLine);
                }
                DrawCircle CGCircle (
                        pads2_CG_center, 
                        Color(1.0, 0.247, 0.0),     // orange red
                        CG_radius,                          // the radius of the CG
                        cairo_fill                              // use file drawMethod
                    );
                NetContainer.push_back(&CGCircle);
            }
            // balls CG to pads CG
            DrawLine CGToCGLine(
                        pads1_CG_center,              // start point is pins1 CG center
                        pads2_CG_center,             // end point is pins2 CG center
                        Color(1.0, 0.662745, 0.0)    // orange
                );
            NetContainer.push_back(&CGToCGLine);

            // constructor the 'PhysicalObject' with external netlist and store it
            g_entireObjMap["ExternalNetlistMap"][map_it->first] = PhysicalObject(
                    map_it->first,                                      // the name of the netlist
                    NetContainer.getCenterPoint(),  // the center of the 
                    NetContainer                                        // correspond DrawContainer
                );
        }
        // Draw Outer Netlist Line
        nets = netlist["external"];
        for (map<string, relationship>::iterator map_it = nets.begin(); map_it != nets.end(); map_it++) {
            // calculate nets size
            pair<Cartesian, Cartesian> CWH = map_it->second.get_CWH();
            // the 'DrawContainer' of the net
            DrawContainer NetContainer(
                        Point(CWH.first.x, CWH.first.y),      // the middle point between pins1 and pins2
                        CWH.second.x,    // the x distance between pins1 and pins2
                        CWH.second.y     // the y distance between pins1 and pins2
                    );
            
            // draw pads1 to CG of pads1
            if (map_it->second.pins1_number == 0) {
                Cartesian balls_CG(CG(chip->get_Some_Balls_Location(map_it->second.pins1)));
                Point balls_CG_center(balls_CG.x, -balls_CG.y);
                Cartesian pads_CG(CG(chip->get_Pads_Location(map_it->second.pins2)));
                Point pads_CG_center(pads_CG.x, -pads_CG.y);
                // multi balls
                if (map_it->second.pins1.size() > 1) {
                    for (size_t index = 0; index < map_it->second.pins1.size(); index++) {
                        Point ball_center(chip->get_Ball_Location(map_it->second.pins1[index]).x, -(chip->get_Ball_Location(map_it->second.pins1[index]).y));
                        DrawLine PinToCGLine(
                                ball_center,              // start point is pins1[index] center
                                balls_CG_center,             // end point is pins1 CG center
                                Color(0.0, 0.498, 0.0)    // half green
                            );
                        NetContainer.push_back(&PinToCGLine);
                    }
                    DrawCircle CGCircle (
                            balls_CG_center, 
                            Color(0.0, 0.498, 0.0),     // half green
                            CG_radius,                          // the radius of the CG
                            cairo_fill                              // use file drawMethod
                        );
                    NetContainer.push_back(&CGCircle);
                }
                // multi pads
                if (map_it->second.pins2.size() > 1) {
                    for (size_t index = 0; index < map_it->second.pins2.size(); index++) {
                        Point pad_center(chip->get_Die_Pad_Location(map_it->second.pins2[index]).x, -(chip->get_Die_Pad_Location(map_it->second.pins2[index]).y));
                        DrawLine PinToCGLine(
                                pads_CG_center,              // start point is pins2 CG center
                                pad_center,                     // end point is pins2[index] center
                                Color(0.0, 0.498, 0.0)    // half green
                            );
                        NetContainer.push_back(&PinToCGLine);
                    }
                    DrawCircle CGCircle (
                            pads_CG_center, 
                            Color(0.0, 0.498, 0.0),     // half green
                            CG_radius,                          // the radius of the CG
                            cairo_fill                              // use file drawMethod
                        );
                    NetContainer.push_back(&CGCircle);
                }
                // balls CG to pads CG
                DrawLine CGToCGLine(
                            balls_CG_center,              // start point is pins1 CG center
                            pads_CG_center,             // end point is pins2 CG center
                            Color(0.0, 1.0, 0.0)            // green
                    );
                NetContainer.push_back(&CGToCGLine);

                // constructor the 'PhysicalObject' with external netlist and store it
                g_entireObjMap["ExternalNetlistMap"][map_it->first] = PhysicalObject(
                        map_it->first,                                      // the name of the netlist
                        NetContainer.getCenterPoint(),  // the center of the 
                        NetContainer                                        // correspond DrawContainer
                    );
            }
            else {
                cout << "Warning: Ignore \"" << map_it->first << "\". No BGA balls for external netlist.\n";
            }
        }
    }

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