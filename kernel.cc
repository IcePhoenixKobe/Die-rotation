#include"main.h"

using namespace std;

static const int space = 40;
static const int pad_width = 40;
static const int pad_height = 60;
static const int finger_width = 80;
static const int finger_height = 250;
static const double ball_radius = 200.0;

// extern variables from gtk_GUI.cpp file
extern map<string, map<string, PhysicalObject>> g_entireObjMap;

void check_argument(int argc, char* argv[])
{
    for (int i = 3; i < argc; i++)
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
    return;
}

Cartesian CG(vector<Cartesian> pos)
{
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

bool ignore_Power_Ground(string str)
{
    if (ignore_P_G && (
                        str.find("P") != string::npos || 
						str.find("V") != string::npos || 
						str.find("VCC") != string::npos || 
						str.find("VDD") != string::npos || 
						str.find("G") != string::npos || 
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
    /* first - read the info, netlist, pin and artificial finger file (name, position and relationship) */
    item temp_item;
    map<string, item> pads, balls;
    map<string, map<string, item>> items;            // record object name and position
    map<string, string> padBallPair, padArtificialFingerPair; // object relationship
	//vector<vector<float>> fingerRowPos;             // each Group's position of row of the fingers

    pads.clear();
    balls.clear();
    items.clear();
    padBallPair.clear();
    padArtificialFingerPair.clear();

    // 1. insert all pads
    for (size_t t = 0; t < chip->get_Dice().size(); t++)
    {
        pads.clear();
        Die die = chip->get_Die(t);
        for (size_t tt = 0; tt < die.get_Pad_Amount(); tt++)
        {
            temp_item.name = die.get_Pad_Name(tt);
            temp_item.x = die.get_Cart_Position(tt).x;
            temp_item.y = -die.get_Cart_Position(tt).y;
            pads[temp_item.name] = temp_item;
        }
	    if (t == 0) items["pad"] = pads;
        else if (t == 1) items["pad1"] = pads;
    }

    // 2. insert all BGA balls
    for (size_t t = 0; t < chip->get_Ball_Amount(); t++)
    {
        temp_item.name = chip->get_Ball_Name(t);
        temp_item.x = chip->get_Ball_Pos(t).x;
        temp_item.y = -chip->get_Ball_Pos(t).y;
        balls[temp_item.name] = temp_item;
    }
	items["ball"] = balls;

    // 3. insert all netlists
    vector<Die> dice = chip->get_Dice();
    vector<InnerRelationship> inner_nets = chip->get_All_I_Netlist();
    vector<OuterRelationship> outer_nets = chip->get_All_O_Netlist();
    for (size_t t = 0; t < inner_nets.size(); t++)
    {
        // NOPE!!!
    }
    for (size_t t = 0; t < outer_nets.size(); t++)
    {
        for (size_t tt = 0; tt < outer_nets[t].balls_index.size(); tt++)
            for (size_t ttt = 0; ttt < outer_nets[t].dice_pads_index.size(); ttt++)
                padBallPair[
                    dice[outer_nets[t].dice_pads_index[ttt].first - 1].get_Pad_Name(outer_nets[t].dice_pads_index[ttt].second)
                ] = chip->get_Ball_Name(outer_nets[t].balls_index[tt]);
    }

    /*for (auto i = padBallPair.begin(); i != padBallPair.end(); i++)
	{
		cout << fixed << setprecision(3) << "  " << pads[i->first].name << "  " << pads[i->first].x << " " << pads[i->first].y << " " << balls[i->second].name << "  " << balls[i->second].x << " " << balls[i->second].y << endl;
	}
	cout << pads.size() << " " << balls.size() << " " << endl;*/

    //combine pads balls fingers into items
    
	/* second - create pad, ball drawObject and preprocessing data for Minimum-cost flow algorithm */
    map<Group, PadGroup> padGroupMap;
    
    cout << "----------------\n";
    initialize(items, padBallPair, padArtificialFingerPair, padGroupMap);

    /*int count = 0;
    for(map<Group, PadGroup>::iterator map_it = padGroupMap.begin(); map_it != padGroupMap.end(); ++map_it){
       
        PadGroup currentPadGroup = map_it->second;
        map<string, string> padFingerPair;
        vector<int> rowFingerNum;
        int pad_number = (int)currentPadGroup.pad_name_vec.size();

        count++;
    }*/
}

void initialize(const map<string, map<string, item>> &items, 
        const map<string, string> &padBallPair,
        const map<string, string> &padArtificialFingerPair,
        map<Group, PadGroup> &padGroupMap)
{
    const map<string, item> pads = items.at("pad"); 
    const map<string, item> balls = items.at("ball");
    //const map<string, item> fingers = items.at("finger");

    // the border position of the IC 
    double maxX = -2500.0, minX = 2500.0, maxY = -2500.0, minY = 2500.0;
    // find the border position of the pad in 'pads'
    for(map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); ++map_it){
        // get the center point of the pad
        Point padCenter(map_it->second.x, map_it->second.y);
        // update maxX
        if(maxX < padCenter.x)
            maxX = padCenter.x;
        // update minX
        if(minX > padCenter.x)
            minX = padCenter.x;
        // update maxY
        if(maxY < padCenter.y)
            maxY = padCenter.y;
        // update minY
        if(minY > padCenter.y)
            minY = padCenter.y;
    }

    // print out the border position of the IC
    cout << "minX: " << minX << endl;
    cout << "maxX: " << maxX << endl;
    cout << "minY: " << minY << endl;
    cout << "maxY: " << maxY << endl;

    // construct the 'PhysicalObject ic' with previous found border (maxX, minX, maxY, minY)
    double ICWidth = maxX - minX + 2 * space;
    double ICHeight = maxY - minY + 2 * space;
    DrawContainer ICContainer(
            Point(), 
            ICWidth, 
            ICHeight
            );
    DrawRectangle ICRect(
            Point(GOD_Center.x, GOD_Center.y),
            Color(1.0, 1.0, 0.0),
            ICWidth,
            ICHeight,
            cairo_stroke
            );

    ICContainer.push_back(&ICRect);

    // initialize the 'PadGroup' (UP, DOWN, LEFT, RIGHT) and insert them in 'padGroupMap'
    padGroupMap.insert(pair<Group, PadGroup>(Group::UP, PadGroup()));
    padGroupMap.insert(pair<Group, PadGroup>(Group::DOWN, PadGroup()));
    padGroupMap.insert(pair<Group, PadGroup>(Group::LEFT, PadGroup()));
    padGroupMap.insert(pair<Group, PadGroup>(Group::RIGHT, PadGroup()));
    
    // according to the position of the pad, put it to the suitable 'PadGroup'
    for(map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); ++map_it){
        // get the center point of the pad
        Point padCenter(map_it->second.x, map_it->second.y);
        // the vertical and horizontal 'Group'
        Group groupV, groupH;
        // the ratio of pad width/height to maximum width/height
        double nearV, nearH;

        // calculate the ratio
        nearV = (maxY - padCenter.y) / (maxY - minY);
        nearH = (maxX - padCenter.x) / (maxX - minX);
        
        // if 'nearV' is less than or equal to 0.5, the y position of pad is near to 'Group::DOWN'
        // otherwise it is near to 'Group::UP'
        if(nearV <= 0.5){
            groupV = Group::DOWN;
        }
        else{
            groupV = Group::UP;
            nearV = 1.0 - nearV;
        }

        // if 'nearH' is less than or equal to 0.5, the x position of pad is near to 'Group::RIGHT'
        // otherwise it is near to 'Group::LEFT'
        if(nearH <= 0.5){
            groupH = Group::RIGHT;
        }
        else{
            groupH = Group::LEFT;
            nearH = 1.0 - nearH;
        }
        
        // if 'nearV' is greater than 'nearH', the pad is belong to 'groupH'
        // otherwise it is belong to 'groupV'
        if(nearV > nearH)
            groupV = groupH;
        // put the pad's name into its 'PadGroup'
        padGroupMap[groupV].pad_name_vec.push_back(map_it->first);
        
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

        // put 'padRect' into 'padContainer'
        padContainer.push_back(&padRect);

        // constructor the 'PhysicalObject' to pad and store it
        /*g_entireObjMap["padMap"][map_it->first"1"] = PhysicalObject(
                map_it->first"2",                  // the name of the pad
                padContainer.getCenterPoint(),  // the center of the pad
                padContainer                    // correspond DrawContainer
                );*/
        //g_entireObjMap["padMap"][map_it->first].addRelation(pair<string, string>("ballMap", /*padBallPair.at(map_it->first)*/"1"));
    }

    // pad2
    if (items.find("pad1") != items.end())
    {
        // the border position of the IC 
        double maxX = -2500.0, minX = 2500.0, maxY = -2500.0, minY = 2500.0;
        const map<string, item> pads1 = items.at("pad1"); 
        for(map<string, item>::const_iterator map_it = pads1.begin(); map_it != pads1.end(); ++map_it){
            // get the center point of the pad
            Point padCenter(map_it->second.x, map_it->second.y);
            // update maxX
            if(maxX < padCenter.x)
                maxX = padCenter.x;
            // update minX
            if(minX > padCenter.x)
                minX = padCenter.x;
            // update maxY
            if(maxY < padCenter.y)
                maxY = padCenter.y;
            // update minY
            if(minY > padCenter.y)
                minY = padCenter.y;
        }

        // print out the border position of the IC
        cout << "minX: " << minX << endl;
        cout << "maxX: " << maxX << endl;
        cout << "minY: " << minY << endl;
        cout << "maxY: " << maxY << endl;

        // construct the 'PhysicalObject ic' with previous found border (maxX, minX, maxY, minY)
        double ICWidth = maxX - minX + 2 * space;
        double ICHeight = maxY - minY + 2 * space;
        DrawRectangle ICRect(
                Point(GOD_GOD_Center.x, GOD_GOD_Center.y),
                Color(1.0, 1.0, 0.0),
                ICWidth,
                ICHeight,
                cairo_stroke
                );

        ICContainer.push_back(&ICRect);

        g_entireObjMap["IC"]["ic"] = PhysicalObject(
                "ic",
                ICContainer.getCenterPoint(),
                ICContainer
                );

        // initialize the 'PadGroup' (UP, DOWN, LEFT, RIGHT) and insert them in 'padGroupMap'
        padGroupMap.insert(pair<Group, PadGroup>(Group::UP, PadGroup()));
        padGroupMap.insert(pair<Group, PadGroup>(Group::DOWN, PadGroup()));
        padGroupMap.insert(pair<Group, PadGroup>(Group::LEFT, PadGroup()));
        padGroupMap.insert(pair<Group, PadGroup>(Group::RIGHT, PadGroup()));
        
        // according to the position of the pad, put it to the suitable 'PadGroup'
        for(map<string, item>::const_iterator map_it = pads1.begin(); map_it != pads1.end(); ++map_it){
            // get the center point of the pad
            Point padCenter(map_it->second.x, map_it->second.y);
            // the vertical and horizontal 'Group'
            Group groupV, groupH;
            // the ratio of pad width/height to maximum width/height
            double nearV, nearH;

            // calculate the ratio
            nearV = (maxY - padCenter.y) / (maxY - minY);
            nearH = (maxX - padCenter.x) / (maxX - minX);
            
            // if 'nearV' is less than or equal to 0.5, the y position of pad is near to 'Group::DOWN'
            // otherwise it is near to 'Group::UP'
            if(nearV <= 0.5){
                groupV = Group::DOWN;
            }
            else{
                groupV = Group::UP;
                nearV = 1.0 - nearV;
            }

            // if 'nearH' is less than or equal to 0.5, the x position of pad is near to 'Group::RIGHT'
            // otherwise it is near to 'Group::LEFT'
            if(nearH <= 0.5){
                groupH = Group::RIGHT;
            }
            else{
                groupH = Group::LEFT;
                nearH = 1.0 - nearH;
            }
            
            // if 'nearV' is greater than 'nearH', the pad is belong to 'groupH'
            // otherwise it is belong to 'groupV'
            if(nearV > nearH)
                groupV = groupH;
            // put the pad's name into its 'PadGroup'
            padGroupMap[groupV].pad_name_vec.push_back(map_it->first);
            
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

            // put 'padRect' into 'padContainer'
            padContainer.push_back(&padRect);

            // constructor the 'PhysicalObject' to pad and store it
            /*g_entireObjMap["padMap"][map_it->first"1"] = PhysicalObject(
                    map_it->first"2",                  // the name of the pad
                    padContainer.getCenterPoint(),  // the center of the pad
                    padContainer                    // correspond DrawContainer
                    );*/
            //g_entireObjMap["padMap"][map_it->first].addRelation(pair<string, string>("ballMap", /*padBallPair.at(map_it->first)*/"1"));
        }
    }
    

    // calculate the parameter and sort the 'pad_name_vec' in the struct 'PadGroup'
    /*for(map<Group, PadGroup>::iterator map_it = padGroupMap.begin(); map_it != padGroupMap.end(); ++map_it){
        // copy the 'PadGroup' member 'pad_name_vec'
        vector<string> pad_name_vec(map_it->second.pad_name_vec);
        // use to sort the 'pad_name_vec'
        vector<pair<double, string>> padGroupSorted((int)pad_name_vec.size(), pair<double, string>());

        // set the 'group' (useless for now)
        map_it->second.group = map_it->first;

        // if 'Group' is equal to 'UP' or 'DOWN', 'padGroupSorted' is sorted by x position
        // otherwise, it is sorted by y position
        if(map_it->first == Group::UP || map_it->first == Group::DOWN){
            for(int i = 0; i < (int)pad_name_vec.size(); ++i){
                padGroupSorted[i].first = pads.at(pad_name_vec[i]).x;
                padGroupSorted[i].second = pad_name_vec[i];
            }
        }
        else if(map_it->first == Group::LEFT || map_it->first == Group::RIGHT){
            for(int i = 0; i < (int)pad_name_vec.size(); ++i){
                padGroupSorted[i].first = pads.at(pad_name_vec[i]).y;
                padGroupSorted[i].second = pad_name_vec[i];
            }
        }
       
        // sort the 'padGroupSorted' with ascending order 
        sort(padGroupSorted.begin(), padGroupSorted.end());
        map_it->second.leftmost_pos = padGroupSorted.front().first;
        map_it->second.rightmost_pos = padGroupSorted.back().first;

        // update the 'pad_name_vec' with 'padGroupSorted'
        for(int i = 0; i < (int)padGroupSorted.size(); ++i){
            map_it->second.pad_name_vec[i] = padGroupSorted[i].second;
        }

        // calculate the 'avg_height'
        map_it->second.avg_height = 0.0;
        // if 'Group' is equal to 'UP' or 'DOWN', 'avg_height' is increased by y position of the pad
        // otherwise, it is increased by x position of the pad
        if(map_it->first == Group::UP || map_it->first == Group::DOWN){
            for(int i = 0; i < (int)pad_name_vec.size(); ++i){
                map_it->second.avg_height += pads.at(padGroupSorted[i].second).y;
            }
        }
        else if(map_it->first == Group::LEFT || map_it->first == Group::RIGHT){
            for(int i = 0; i < (int)pad_name_vec.size(); ++i){
                map_it->second.avg_height += pads.at(padGroupSorted[i].second).x;
            }
        }
        
        // average the 'avg_height'
        map_it->second.avg_height /= (int)pad_name_vec.size();

        // print out the parameter in struct 'PadGroup' 
        cout << "---" << static_cast<underlying_type<Group>::type>(map_it->first) << "---\n";
        cout << "leftmost_pos: " << map_it->second.leftmost_pos << endl;
        cout << "rightmost_pos: " << map_it->second.rightmost_pos << endl;
        cout << "avg_height: " << map_it->second.avg_height << endl;
    }*/

    // create 'PhysicalObject' object to each ball and store it into 'g_entireObjMap' with key 'ballMap'
    for(map<string, item>::const_iterator map_it = balls.begin(); map_it != balls.end(); ++map_it){
        // the name of the ball
        string ballName = map_it->first;
        // the center of the ball
        Point ballCenter(map_it->second.x, map_it->second.y);
        // the 'DrawContainer' of the ball
        DrawContainer ballContainer(
                ballCenter, 
                2 * ball_radius, 
                2 * ball_radius
                );
        // the draw pattern of the ball
        DrawCircle ballCircle(
                ballCenter,             // the center of the ball
                Color(0.0, 0.0, 1.0),   // blue
                ball_radius,            // the radius of the ball
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
        g_entireObjMap["ballMap"][ballName] = PhysicalObject(
                ballName,                       // the name of the ball
                ballContainer.getCenterPoint(), // the center of the ball
                ballContainer                   // the DrawContainer of the ball
                );
    }

    // create 'PhysicalObject' object to each pair of pad and ball and store it into 'g_entireObjMap' with key 'padToBallMap'
    for(map<string, PhysicalObject>::iterator map_it = g_entireObjMap["padMap"].begin(); map_it != g_entireObjMap["padMap"].end(); ++map_it){
        // the center of the pad
        Point padCenter(map_it->second.getCenter());
        // the center of the ball
        Point ballCenter(
            g_entireObjMap["ballMap"][padBallPair.at(map_it->first)].getCenter());
        // the 'DrawContainer' of the padToBall
        DrawContainer padToBallContainer(
                (padCenter + ballCenter) / 2.0,     // the middle point between padCenter and ballCenter
                fabs(padCenter.x - ballCenter.x),   // the x distance between padCenter and ballCenter
                fabs(padCenter.y - ballCenter.y)   // the y distance between padCenter and ballCenter
                );
        DrawLine padToBallLine(
                padCenter,              // start point is padCenter
                ballCenter,             // end point is ballCenter
                Color(0.5, 0.3, 0.4)    //
                );

        // put 'padToBallLine' into 'padToBallContainer'
        padToBallContainer.push_back(&padToBallLine);

        // construct the 'PhysicalObject' with padToBall and store it
        g_entireObjMap["padToBallMap"][map_it->first] = PhysicalObject(
                map_it->first,                          // the name of the pad
                padToBallContainer.getCenterPoint(),         // the center of the padToBall    
                padToBallContainer                      // the DrawContainer of the padToBall
                );
    }

    // create 'PhysicalObject' object to each artificial finger and store it into 'g_entireObjMap' with key 'artificialFingerMap'
    /*for(map<string, item>::const_iterator map_it = fingers.begin(); map_it != fingers.end(); ++map_it){
        //cout << map_it->first << " " << map_it->second.x << " " << map_it->second.y << endl;
        // the name of the artifical finger
        string artificialFingerName = map_it->first;
        // the center of the artificial finger
        Point artificialFingerCenter(map_it->second.x, map_it->second.y);
        // the 'DrawContainer' of the artificial finger
        DrawContainer artificialFingerContainer(
                artificialFingerCenter, 
                finger_width, 
                finger_height);
        // the draw pattern of the artificial finger
        DrawRectangle artificialFingerRect(
                artificialFingerCenter, // the center of the artificial finger
                Color(1.0, 1.0, 1.0),   // white
                finger_width,           // the width of the artificial finger
                finger_height,          // the height of the artificial finger      
                cairo_fill              // use stroke drawMethod
                );
        
        // put 'artificialFingerRect' into 'artificialFingerContainer'
        artificialFingerContainer.push_back(&artificialFingerRect);
        
        string padName;

        for(map<string, string>::const_iterator map_it = padArtificialFingerPair.begin(); map_it != padArtificialFingerPair.end(); ++map_it){
            if(map_it->second == artificialFingerName){
                padName = map_it->first;
                break;
            }
        }

        Group padGroup;

        for(map<Group, PadGroup>::iterator map_it = padGroupMap.begin(); map_it != padGroupMap.end(); ++map_it){
            vector<string>::iterator vec_it = find(map_it->second.pad_name_vec.begin(), map_it->second.pad_name_vec.end(), padName);
            
            if(vec_it != map_it->second.pad_name_vec.end()){
                padGroup = map_it->second.group;
                break;
            }
        }
        
        if(padGroup == Group::LEFT){
            artificialFingerContainer.rotate(-1.57);
        }
        else if(padGroup == Group::RIGHT){
            artificialFingerContainer.rotate(1.57);
        }

        // construct the 'PhysicalObject' with artificial finger and store it
        g_entireObjMap["artificialFingerMap"][artificialFingerName] = PhysicalObject(
                artificialFingerName,                       // the name of the artificial finger
                artificialFingerContainer.getCenterPoint(), // the center of the atificial finger
                artificialFingerContainer                   // the DrawContainer of the artificial finger
                );
        
    }*/

    //cout << padArtificialFingerPair.size() << endl;
    //cout << fingers.size() << endl;
    // create 'PhysicalObject' object to each pair of pad and artificial finger and store it into 'g_entireObjMap' with key 'padToArtificialFingerMap'
    /*for(map<string, PhysicalObject>::iterator map_it = g_entireObjMap["padMap"].begin(); map_it != g_entireObjMap["padMap"].end(); ++map_it){
        // the center of the pad
        Point padCenter(map_it->second.getCenter());
        // the center of the artificial finger
        cout << map_it->first;
        cout << padArtificialFingerPair.at(map_it->first) << endl;
        Point artificialFingerCenter(g_entireObjMap["artificialFingerMap"][padArtificialFingerPair.at(map_it->first)].getCenter());
        // the 'DrawContainer' of the padToArtificialFinger
        DrawContainer padToArtificialFingerContainer(
                (padCenter + artificialFingerCenter) / 2.0,     // the middle point between padCenter and artificialFingerCenter
                fabs(padCenter.x - artificialFingerCenter.x),   // the x distance between padCenter and artificialFingerCenter
                fabs(padCenter.y - artificialFingerCenter.y)   // the y distance between padCenter and artificialFingerCenter
                );
        DrawLine padToArtificialFingerLine(
                padCenter,              // start point is padCenter
                artificialFingerCenter, // end point is artificialFingerCenter
                Color(0.5, 0.3, 0.4)    //
                );

        // put 'padToArtificialFingerLine' into 'padToArtificialFingerContainer'
        padToArtificialFingerContainer.push_back(&padToArtificialFingerLine);

        // construct the 'PhysicalObject' with padToArtificialFinger and store it
        g_entireObjMap["padToArtificialFingerMap"][map_it->first] = PhysicalObject(
                map_it->first,                                      // the name of the pad
                padToArtificialFingerContainer.getCenterPoint(),    // the center of the padToArtificialFinger    
                padToArtificialFingerContainer                      // the DrawContainer of the padToArtificialFinger
                );
    }*/

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