#include"chip.h"

Chip::Chip()
{
    balls = Ball();
    dice.clear();
    outer_netlist.clear();
}

void Chip::parse_PIN(std::ifstream& fin)
{
	int sub_str_pos = -1;
	bool isBall = false;
	bool isDie = false;
	size_t die_number = 0, dice_amount = 0;
	std::string str;
	std::string pin_name;
	std::vector<std::string> balls_name;
	std::vector<Cartesian> balls_cartesian;
	std::vector<Cartesian> dice_center;
	std::vector<std::vector<std::string>> dice_pads_name;       // all pads name
	std::vector<std::vector<Cartesian>> dice_pads_cartesian;    // all pads location
    
    str.clear();
    pin_name.clear();
    balls_name.clear();
    balls_cartesian.clear();
    dice_center.clear();
    dice_pads_name.clear();
    dice_pads_cartesian.clear();

    while (getline(fin, str))
	{
		if (str.find("pin number:") != std::string::npos)
        {
			str.erase(0, 16);
            if (str.find("BGA") != std::string::npos)
                isBall = true;
            else if (str.find("DIE") != std::string::npos)
                isDie = true;
            else
            {
                isBall = false;
                isDie = false;
            }

            if (isBall || isDie)
            {
                std::stringstream ss;
                ss << str;
                ss >> pin_name;
                
                if (isDie)
                {
                    std::stringstream string_cache(pin_name.substr(3, pin_name.find(".") - 3));
                    string_cache >> die_number;
                }
            }
        }
        
		sub_str_pos = str.find("location-xy:");
		if (sub_str_pos != -1 && (isBall || isDie)) // if found location
		{
            Cartesian pin_cartesian;
            pin_cartesian.x = 0;
            pin_cartesian.y = 0;

			str.erase(0, 16);
			std::stringstream ss; //clean white space
			ss << str;
			ss >> str;
			str.erase(0, 1);    // clean left quote
			pin_cartesian.x = stod(str);
			ss >> str;
			str.erase(str.size() - 1, 1);//clean right quote
			pin_cartesian.y = stod(str);
			if (isBall && !isDie)
			{
				balls_name.push_back(pin_name);
                balls_cartesian.push_back(pin_cartesian);
			}
			else if (isDie && !isBall)
			{
                if (die_number <= dice_amount)
                {
                    dice_pads_name[die_number - 1].push_back(pin_name);
                    dice_pads_cartesian[die_number - 1].push_back(pin_cartesian);
                }
                else
                {
                    while (dice_amount < die_number)
                    {
                        dice_center.push_back(Cartesian(0, 0));
                        std::vector<std::string> pads_name;
                        std::vector<Cartesian> pads_cartesian;
                        if (dice_amount == die_number - 1)
                        {
                            pads_name.push_back(pin_name);
                            pads_cartesian.push_back(pin_cartesian);
                        }
                        dice_pads_name.push_back(pads_name);
                        dice_pads_cartesian.push_back(pads_cartesian);
                        dice_amount++;
                    }
                }
			}
            else
            {
                std::cout << "error: isBall and isDie at the same times.\n";
                exit(0);
            }
            isBall = false;
            isDie = false;
        }
	}
    
    // parse done and do set
    balls = Ball(balls_name.size());
    balls.set_All_Name(balls_name);
    balls.set_All_Pos(balls_cartesian);
    dice.resize(dice_amount);
    for (size_t i = 0; i < dice_amount; i++)
    {
        dice[i] = Die(dice_pads_name[i].size(), dice_center[i].x, dice_center[i].y);
        dice[i].set_Pad_Name(dice_pads_name[i]);
        dice[i].set_Cart_Pos(dice_pads_cartesian[i]);
        dice[i].convert_cart_to_polar();
    }

	fin.close();
    std::cout << "parse PIN done\n\n";
    return;
}

void Chip::parse_Netlist(std::ifstream& fin)
{
    std::string str, str_temp;
    str.clear();
    str_temp.clear();

    outer_netlist.clear();
    // Try to find "$NET"
    do getline(fin, str_temp); while (str_temp.find("$NETS") == std::string::npos && !fin.eof());

    if (!fin.eof()) // found $NET
    {
        // start to parse netlist
        while (getline(fin, str_temp) && str_temp.find("$END") == std::string::npos)
        {
            if (str_temp.find(",") != std::string::npos)    // concatenate multi-line
            {
                str_temp.erase(str_temp.size() - 1, 1);
                str += str_temp;
            }
            else
            {
                std::string netlist_name;
                std::stringstream ss;

                netlist_name.clear();
                ss.clear();

                str+=str_temp;
                ss << str;
                ss >> netlist_name;  // get outer_netlist name

                if (ignore_Power_Ground(netlist_name))
                {}
                else if (str.find("BGA") != std::string::npos && str.find("DIE") != std::string::npos)   // filter only one kind of pin
                {
                    OuterRelationship rela_s;
                    std::string sub_str;
                    sub_str.clear();
                    
                    rela_s.relation_name = netlist_name;                 // set relation_name
                    ss >> sub_str;  // get ";"
                    if (sub_str == ";")
                    {
                        std::vector<std::string> BGAs;
                        std::vector<std::string> DIEs;
                        BGAs.clear();
                        DIEs.clear();

                        // Categories balls and dice
                        while (ss >> sub_str)
                        {
                            if (sub_str.find("BGA") != std::string::npos)
                                BGAs.push_back(sub_str);
                            else if (sub_str.find("DIE") != std::string::npos)
                                DIEs.push_back(sub_str);
                            else
                                std::cout << "Warning: netlist has no BGA or DIE\n";
                        }
                        
                        // To calculate the center of gravity
                        Cartesian center_of_gravity(0, 0);
                        std::vector<size_t> balls_index;
                        balls_index.clear();
                        for (size_t i = 0; i < BGAs.size(); i++)
                        {
                            size_t index_temp = balls.get_Ball_Index(BGAs[i]);   // set ball_index
                            if (index_temp == -1UL)
                            {
                                std::cout << "error: Not found " << BGAs[i] << std::endl;
                                exit(0);
                            }
                            else
                            {
                                center_of_gravity.x += balls.get_All_Pos().at(index_temp).x;
                                center_of_gravity.y += balls.get_All_Pos().at(index_temp).y;
                                balls_index.push_back(index_temp);
                            }
                        }
                        center_of_gravity.x /= static_cast<double>(BGAs.size());
                        center_of_gravity.y /= static_cast<double>(BGAs.size());
                        rela_s.balls_index = balls_index;
                        rela_s.ball_car = center_of_gravity;
                        
                        std::vector<std::pair<size_t, size_t>> dice_pads_index;
                        dice_pads_index.clear();
                        center_of_gravity = Cartesian(0, 0);
                        for (size_t i = 0; i < DIEs.size(); i++)
                        {
                            std::pair<size_t, size_t> index_temp(-1UL, -1UL);
                            std::stringstream string_cache(DIEs[i].substr(3, DIEs[i].find(".") - 3));
                            string_cache >> index_temp.first;           // set die_index
                            index_temp.second = dice[index_temp.first - 1].get_Pad_Index(DIEs[i]);   // set pad_index
                            if (index_temp.second == -1UL)
                            {
                                std::cout << "error: Not found " << DIEs[i] << std::endl;
                                exit(0);
                            }
                            else
                            {
                                center_of_gravity.x += dice[index_temp.first - 1].get_Cart_Position().at(index_temp.second).x;
                                center_of_gravity.y += dice[index_temp.first - 1].get_Cart_Position().at(index_temp.second).y;
                                dice_pads_index.push_back(index_temp);
                            }
                            center_of_gravity.x /= static_cast<double>(DIEs.size());
                            center_of_gravity.y /= static_cast<double>(DIEs.size());
                            rela_s.dice_pads_index = dice_pads_index;
                            rela_s.pad_car = center_of_gravity;
                        }
                        outer_netlist.push_back(rela_s);
                    }
                    else
                    {
                        std::cout << "error: string has no \";\"\n";
                        exit(0);
                    }
                }
                else
                {
                    std::cout << "warring: string \"" << str.substr(0, str.size() - 1) << "\" has no BGA and DIE\n";
                }
                str.clear();
            }
        }
    }
    else
    {
        // start to parse multi_netlist
        std::cout << "There is no \"$NET\" in netlist file\n";
        std::cout << "Use Net Name to parse netlist file\n";
        fin.clear();
        fin.seekg(std::ios::beg);   // Seek to begin of fin

        bool isNet = false;
        std::string end_net = "  No connections remaining";
        std::string netlist_name;
        OuterRelationship rela_s_O;
        InnerRelationship rela_s_I;
        std::vector<std::string> BGAs;
        std::vector<std::string> DIEs;
        netlist_name.clear();
        BGAs.clear();
        DIEs.clear();

        while (getline(fin, str))
        {
            if (str.find("Net Name:") != std::string::npos)
            {
                std::stringstream ss;
                ss.clear();
                
                str.erase(0, 23);
                ss << str;
                netlist_name.clear();
                ss >> netlist_name;  // get netlist name
                
                if (!ignore_Power_Ground(netlist_name))  // ignore power and ground if ignore_P_G is true
                {
                    rela_s_O = OuterRelationship();
                    rela_s_O.relation_name = netlist_name;                 // set relation_name
                    rela_s_I = InnerRelationship();
                    rela_s_I.relation_name = netlist_name;                 // set relation_name
                    //std::cout << rela_s_O.relation_name << "\t\n";
                    while(getline(fin, str) && str.find("---") == std::string::npos);
                    isNet = true;
                }
            }
            
            std::vector<int> die_number;    // Store the die number that appeared
            die_number.clear();

		    if (isNet)
		    {
                BGAs.clear();
                DIEs.clear();
                while(getline(fin,str) && ( str.find("BGA")  != std::string::npos || 
											str.find("DIE")  != std::string::npos ))
                {
                    std::stringstream ss;
                    ss.clear();
                    ss << str;
                    ss >> str;  // get pin name

                    if (str.find("BGA") != std::string::npos)
                        BGAs.push_back(str);
                    else
                    {
                        bool not_appeared = true;
                        size_t length = str.find('.');
                        int pin_number = atoi(str.substr(3, length - 3).c_str());
                        //std::cout << "str: " << str << "\tlength: " << length << "\tpin number: " << pin_number << std::endl;
                        // Check if pin_number has appeared
                        for (size_t i = 0; i < die_number.size() && not_appeared; i++)  if (die_number[i] == pin_number) not_appeared = false;
                        // If pin_number has not appeared, add pin_number into die_number
                        if (not_appeared) die_number.push_back(pin_number);

                        DIEs.push_back(str);
                    }
                }
                getline(fin,str);
                if (str.find("No connections remaining") == std::string::npos)
                    std::cout << "Warning: netlist ‘" << netlist_name << "‘ has no BGA or DIE, and may have some pin miss catch.\n";
                
                // Decide to do combination or calculate center of gravity
                if (BGAs.size() != 0 && DIEs.size() != 0)   // BGA balls to die pads
                {
                    // Calculate the center of gravity of BGA balls 
                    Cartesian center_of_gravity(0, 0);
                    std::vector<size_t> balls_index;
                    balls_index.clear();
                    for (size_t i = 0; i < BGAs.size(); i++)
                    {
                        size_t index_temp = balls.get_Ball_Index(BGAs[i]);   // set ball_index
                        if (index_temp == -1UL)
                        {
                            std::cout << "error: Not found " << BGAs[i] << std::endl;
                            exit(0);
                        }
                        else
                        {
                            center_of_gravity.x += balls.get_All_Pos().at(index_temp).x;
                            center_of_gravity.y += balls.get_All_Pos().at(index_temp).y;
                            balls_index.push_back(index_temp);
                        }
                    }
                    center_of_gravity.x /= static_cast<double>(BGAs.size());
                    center_of_gravity.y /= static_cast<double>(BGAs.size());
                    rela_s_O.balls_index = balls_index;
                    rela_s_O.ball_car = center_of_gravity;
                    
                    // Calculate the center of gravity of die pads 
                    std::vector<std::pair<size_t, size_t>> dice_pads_index;
                    dice_pads_index.clear();
                    center_of_gravity = Cartesian(0, 0);
                    for (size_t i = 0; i < DIEs.size(); i++)
                    {
                        std::pair<size_t, size_t> index_temp(-1UL, -1UL);
                        std::stringstream string_cache(DIEs[i].substr(3, DIEs[i].find(".") - 3));
                        string_cache >> index_temp.first;           // set die_index
                        index_temp.second = dice[index_temp.first - 1].get_Pad_Index(DIEs[i]);   // set pad_index
                        if (index_temp.second == -1UL)
                        {
                            std::cout << "error: Not found " << DIEs[i] << std::endl;
                            exit(0);
                        }
                        else
                        {
                            center_of_gravity.x += dice[index_temp.first - 1].get_Cart_Position().at(index_temp.second).x;
                            center_of_gravity.y += dice[index_temp.first - 1].get_Cart_Position().at(index_temp.second).y;
                            dice_pads_index.push_back(index_temp);
                        }
                    }
                    center_of_gravity.x /= static_cast<double>(DIEs.size());
                    center_of_gravity.y /= static_cast<double>(DIEs.size());
                    rela_s_O.dice_pads_index = dice_pads_index;
                    rela_s_O.pad_car = center_of_gravity;
                    
                    // insert relationship into outer netlist
                    outer_netlist.push_back(rela_s_O);
                }
                else if (DIEs.size() >= 2)  // die pads to die pads
                {
                    //std::cout << "kind of dice number: " << die_number.size() << std::endl;
                    if (die_number.size() == 2) // Can not handle three or more dice's relationships
                    {
                        Cartesian cg1(0, 0), cg2(0, 0);
                        std::vector<size_t> pads1_index, pads2_index;
                        std::pair<size_t, std::vector<size_t>> pads1, pads2;
                        pads1_index.clear();
                        pads2_index.clear();
                        pads1.first = 0;
                        pads2.first = 0;
                        pads1.second.clear();
                        pads2.second.clear();
                        for (size_t i = 0; i < DIEs.size(); i++)
                        {
                            size_t die_index = -1UL, pad_index = -1UL;
                            std::stringstream string_cache(DIEs[i].substr(3, DIEs[i].find(".") - 3));
                            string_cache >> die_index;
                            pad_index = dice[die_index - 1].get_Pad_Index(DIEs[i]);
                            if (pads1.first == 0 || pads1.first == die_index)
                            {
                                pads1.first = die_index;
                                cg1.x += dice[die_index - 1].get_Cart_Position().at(pad_index).x;
                                cg1.y += dice[die_index - 1].get_Cart_Position().at(pad_index).y;
                                pads1_index.push_back(pad_index);
                            }
                            else if (pads2.first == 0 || pads2.first == die_index)
                            {
                                pads2.first = die_index;
                                cg2.x += dice[die_index - 1].get_Cart_Position().at(pad_index).x;
                                cg2.y += dice[die_index - 1].get_Cart_Position().at(pad_index).y;
                                pads2_index.push_back(pad_index);
                            }
                            else
                            {
                                std::cout << "error: more than two kind of dice";
                                exit(0);
                            }
                        }
                        cg1.x /= static_cast<double>(pads1_index.size());
                        cg1.y /= static_cast<double>(pads1_index.size());
                        cg2.x /= static_cast<double>(pads1_index.size());
                        cg2.y /= static_cast<double>(pads1_index.size());
                        rela_s_I.dice_pads1_index.first = pads1.first;
                        rela_s_I.dice_pads1_index.second = pads1_index;
                        rela_s_I.pad1_car = cg1;
                        rela_s_I.dice_pads2_index.first = pads2.first;
                        rela_s_I.dice_pads2_index.second = pads2_index;
                        rela_s_I.pad2_car = cg2;

                        // insert relationship into netlist
                        inner_netlist.push_back(rela_s_I);
                    }
                }
                isNet = false;
            }
        }
    }

    // Cartesian convert to Polar (Outer Relationship)
    double x = 0.0, y = 0.0;
    for (size_t i = 0; i < outer_netlist.size(); i++)
    {
        x = outer_netlist[i].pad_car.x;
        y = outer_netlist[i].pad_car.y;

        // calculate radius
        outer_netlist[i].pad_pol.radius = sqrt(x * x + y * y);
        //std::cout << "\tradius = " << pol_position[i].radius;

        // calculate theta
        x /= outer_netlist[i].pad_pol.radius;
        y /= outer_netlist[i].pad_pol.radius;
        if (x >= 0 && y >= 0){   //Quadrant I
            outer_netlist[i].pad_pol.angle = (asin(y) + acos(x)) / 2;
        }
        else if (x < 0 && y >= 0){  //Quadrant II
            outer_netlist[i].pad_pol.angle = acos(x);
        }
        else if (x < 0 && y < 0){  //Quadrant III
            outer_netlist[i].pad_pol.angle = acos(x) - 2 * asin(y);
        }
        else if (x >= 0 && y < 0){  //Quadrant IV
            outer_netlist[i].pad_pol.angle = (2 * M_PI + asin(y));
        }
        //std::cout << "\ttheta = " << pol_position[i].angle << std::endl;
    }

    // Cartesian convert to Polar (Inner Relationship)
    double x1 = 0.0, y1 = 0.0, x2 = 0.0, y2 = 0.0;
    for (size_t i = 0; i < inner_netlist.size(); i++)
    {
        x1 = inner_netlist[i].pad1_car.x;
        y1 = inner_netlist[i].pad1_car.y;
        x2 = inner_netlist[i].pad2_car.x;
        y2 = inner_netlist[i].pad2_car.y;

        // calculate radius
        inner_netlist[i].pad1_pol.radius = sqrt(x1 * x1 + y1 * y1);
        inner_netlist[i].pad2_pol.radius = sqrt(x2 * x2 + y2 * y2);

        // calculate theta1
        x1 /= inner_netlist[i].pad1_pol.radius;
        y1 /= inner_netlist[i].pad1_pol.radius;
        // for pad1
        if (x1 >= 0 && y1 >= 0){   //Quadrant I
            inner_netlist[i].pad1_pol.angle = (asin(y1) + acos(x1)) / 2;
        }
        else if (x1 < 0 && y1 >= 0){  //Quadrant II
            inner_netlist[i].pad1_pol.angle = acos(x1);
        }
        else if (x1 < 0 && y1 < 0){  //Quadrant III
            inner_netlist[i].pad1_pol.angle = acos(x1) - 2 * asin(y1);
        }
        else if (x1 >= 0 && y1 < 0){  //Quadrant IV
            inner_netlist[i].pad1_pol.angle = (2 * M_PI + asin(y1));
        }
        // calculate theta2
        x2 /= inner_netlist[i].pad2_pol.radius;
        y2 /= inner_netlist[i].pad2_pol.radius;
        if (x2 >= 0 && y2 >= 0){   //Quadrant I
            inner_netlist[i].pad2_pol.angle = (asin(y2) + acos(x2)) / 2;
        }
        else if (x2 < 0 && y2 >= 0){  //Quadrant II
            inner_netlist[i].pad2_pol.angle = acos(x2);
        }
        else if (x2 < 0 && y2 < 0){  //Quadrant III
            inner_netlist[i].pad2_pol.angle = acos(x2) - 2 * asin(y2);
        }
        else if (x2 >= 0 && y2 < 0){  //Quadrant IV
            inner_netlist[i].pad2_pol.angle = (2 * M_PI + asin(y2));
        }
    }

    /*std::vector<Cartesian> balls_car;
    balls_car.clear();
    for (size_t i = 0; i < outer_netlist.size(); i++)
        balls_car.push_back(outer_netlist[i].ball_car);*/

    fin.close();
    std::cout << "parse netlist done\n";
    return;
}

void Chip::balls_Content()
{
    std::cout << "\n----------Ball content----------\n"
     << "amount: " << balls.get_Amount() << std::endl;
    std::vector<std::string> name;
    name = balls.get_All_Name();
    std::vector<Cartesian> position;
    position = balls.get_All_Pos();
    for (size_t i = 0; i < position.size(); i++)
        std::cout << std::fixed << std::setprecision(3) << std::setw(4) << i + 1 << " " << std::setw(8) << name[i]
         << " x: " << std::setw(10) << position[i].x << ", y: " << std::setw(10) << position[i].y << std::endl;
    std::cout << "----------Ball content----------\n";
    return;
}

void Chip::dice_Content()
{
    std::cout << "\n----------Die content----------";
    for (size_t index = 0; index < dice.size(); index++)
    {
        std::cout << "\nDie " << index + 1 << ":"
         << "\ncenter: " << dice[index].get_Center().x << " " << dice[index].get_Center().y
         << "\npad amount: " << dice[index].get_Pad_Amount() << std::endl;
        std::vector<std::string> name;
        name = dice[index].get_Pad_Name();
        std::vector<Cartesian> cart_position;
        cart_position = dice[index].get_Cart_Position();
        std::vector<Polar> polar_position;
        polar_position = dice[index].get_Pol_Position();
        assert(cart_position.size() == polar_position.size());
        for (size_t i = 0; i < cart_position.size() && i < polar_position.size(); i++) {
            std::cout << std::fixed << std::setprecision(3) << std::setw(4) << i + 1 << " " << std::setw(8) << name[i]
                            << " x: " << std::setw(10) << cart_position[i].x 
                            << ", y: " << std::setw(10)<< cart_position[i].y
                            << " | r: " << std::setw(9)<< polar_position[i].radius 
                            << ", theta: "<< std::setw(7) << polar_position[i].angle * 180 / M_PI << std::endl;
        }
    }
    std::cout << "----------Die content----------\n";
    return;
}

void Chip::netlist_Content()
{
    std::cout << "\n----------Netlist content----------\n"
     << "Outer Netlist: \n"
     << "amount: " << outer_netlist.size() << std::endl;
    for (size_t i = 0; i < outer_netlist.size(); i++)
    {
        std::cout << std::setw(4) << i + 1 << " " << std::setw(10) << outer_netlist[i].relation_name << ": ";
        for (size_t j = 0; j < outer_netlist[i].balls_index.size(); j++)
            std::cout << "\n\t" << balls.get_Ball_Name(outer_netlist[i].balls_index[j]);
        std::cout << "\n\t-->\n";
        for (size_t j = 0; j < outer_netlist[i].dice_pads_index.size(); j++)
            std::cout << "\t" << dice[outer_netlist[i].dice_pads_index[j].first - 1].get_Pad_Name(outer_netlist[i].dice_pads_index[j].second) << std::endl;
    }
    std::cout << "\nInner Netlist: \n"
     << "amount: " << inner_netlist.size() << std::endl;
    for (size_t i = 0; i < inner_netlist.size(); i++)
    {
        std::cout << std::setw(4) << i + 1 << " " << std::setw(10) << inner_netlist[i].relation_name << ": ";
        for (size_t j = 0; j < inner_netlist[i].dice_pads1_index.second.size(); j++)
            std::cout << "\n\t" << dice[inner_netlist[i].dice_pads1_index.first - 1].get_Pad_Name(inner_netlist[i].dice_pads1_index.second[j]);
        std::cout << "\n\t-->\n";
        for (size_t j = 0; j < inner_netlist[i].dice_pads2_index.second.size(); j++)
            std::cout << "\t" << dice[inner_netlist[i].dice_pads2_index.first - 1].get_Pad_Name(inner_netlist[i].dice_pads2_index.second[j]) << std::endl;
    }
    std::cout << "----------Netlist content----------\n";
    return;
}

int Chip::parser(int argc, char** argv)
{
    std::string str;
	std::string index_suffle;
    std::ifstream PIN_fin;		// PIN
	std::ifstream netlist_fin;	// netlist
	std::ifstream shuffle_fin;	// shuffle netlist
	std::ofstream fout;			// relationship

    str.clear();
    index_suffle.clear();
    PIN_fin.clear();
    netlist_fin.clear();
    shuffle_fin.clear();
    fout.clear();

    PIN_fin.open(argv[1]);
	if (!PIN_fin.is_open()) {
		std::cout << "file \"" << argv[1] << "\" open error.";
		return -1;
	}
    parse_PIN(PIN_fin);
    
	netlist_fin.open(argv[2]);
	if (!netlist_fin) {
		std::cout << "file \"" << argv[2] << "\" open error.";
		return -1;
	}
    parse_Netlist(netlist_fin);
    /*
    Cartesian position_sum;
    for (size_t i = 0; i < outer_netlist.size(); i++)
    {
        position_sum.x += outer_netlist[i].ball_car.x + outer_netlist[i].pad_car.x;
        position_sum.y += outer_netlist[i].ball_car.y + outer_netlist[i].pad_car.y;
    }
    dice[0].set_Center(Cartesian(position_sum.x / static_cast<double>(outer_netlist.size()), position_sum.y / static_cast<double>(outer_netlist.size())));
    */
    /*
    std::cout << "number of outer_netlist: " << outer_netlist.size() << std::endl;
    for (size_t index = 0; index < outer_netlist.size(); index++)
    {
        std::cout << outer_netlist[index].relation_name << " : " << outer_netlist[index].ball_index << " --> " << outer_netlist[index].die_pad_index << std::endl;
    }
    */
    balls_Content();
    dice_Content();
    netlist_Content();
    
    return 0;
}

size_t Chip::get_Netlist_Index(std::string str) const
{
    for (size_t t = 0; t <= outer_netlist.size(); t++)
        if (outer_netlist[t].relation_name == str)
            return t;
    return -1UL;
}

void Chip::output_LP_File(std::ofstream& fout)
{
    std::cout << "Notice: can not use multi_io!!!\n";
    // Minimize
    if (min_output)
    {
        fout << "Minimize\n";
        for (size_t i = 0; i < outer_netlist.size(); i++) {
            if (i == 0) fout << "  a0x + a0y";
            else {
                fout << " + a" << i << "x + a" << i << "y";
            }
        }
        fout << "\n";
    }

   // Subject To
    fout << "Subject To\n";
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        double radius = outer_netlist[i].pad_pol.radius;
        if (i == 0) fout << "  goal: " << radius << " s0p - " << radius << " c0p";
        else {
            fout << " + " << radius << " s" << i << "p - " << radius << " c" << i << "p";
        }
    }
    fout << " = 0\n";

    // Bounds
    std::vector<std::vector<Polar>> dice_pads;
    dice_pads.resize(dice.size());
    for (size_t i = 0; i < dice.size() && i < dice_pads.size(); i++)
        dice_pads[i] = dice[i].get_Pol_Position();
    fout << "Bounds\n";
    fout << "  0 <= alpha <= " << 2 * M_PI << "\n";
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        fout << " \\net" << i << "\n"
                 << "  n" << i << "d free\n" 
                 << "  n" << i << "s free\n" 
                 << "  n" << i << "c free\n" 
                 << "  s" << i << "p free\n" 
                 << "  c" << i << "p free\n" ;
        if (min_output)
            fout << "  m" << i << "x free\n" << "  m" << i << "y free\n"
                     << "  a" << i << "x free\n" << "  a" << i << "y free\n";
    }

    // General Constraint
    std::vector<Cartesian> ball_pos;
    ball_pos = balls.get_All_Pos();
    fout << "General Constraint\n";
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 alpha + " << outer_netlist[i].pad_pol.angle << " )\n" 
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  sin" << i << ": s" << i << "p = POLY ( "  << ( outer_netlist[i].ball_car.x - dice[outer_netlist[i].dice_pads_index[0].first - 1].get_Center().x ) << " n" << i << "s )\n"
                 << "  cos" << i << ": c" << i << "p = POLY ( " << ( outer_netlist[i].ball_car.y - dice[outer_netlist[i].dice_pads_index[0].first - 1].get_Center().y ) << " n" << i << "c )\n";
        if (min_output)
        {
            fout << " \\minimize equation\n";
            fout << "  miner" << i << "x: m" << i << "x = POLY ( -" << outer_netlist[i].pad_pol.radius 
                 << " n" << i << "c + " << outer_netlist[i].ball_car.x - dice[outer_netlist[i].dice_pads_index[0].first - 1].get_Center().x << " )\n"
                 << "  miner" << i << "y: m" << i << "y = POLY ( -" << outer_netlist[i].pad_pol.radius
                 << " n" << i << "s + " << outer_netlist[i].ball_car.y - dice[outer_netlist[i].dice_pads_index[0].first - 1].get_Center().y << " )\n";
            fout << " \\absolute value\n"
                     << "  abs" << i << "x: a" << i << "x = ABS ( m" << i << "x )\n" 
                     << "  abs" << i << "y: a" << i << "y = ABS ( m" << i << "y )\n";
        }
    }

    /*// Minimize
    if (min_output)
    {
        fout << "Minimize\n";
        for (size_t i = 0; i < outer_netlist.size(); i++) {
            if (i == 0) fout << "  ";
            else fout << " + ";
            fout << "a" << i << "x + a" << i << "y";
        }
        fout << "\n";
    }

   // Subject To
    fout << "Subject To\n";
    fout << "  alpha <= " << 2 * M_PI << "\n";

    // Bounds
    std::vector<std::vector<Polar>> dice_pads;
    dice_pads.resize(dice.size());
    for (size_t i = 0; i < dice.size() && i < dice_pads.size(); i++)
        dice_pads[i] = dice[i].get_Pol_Position();
    fout << "Bounds\n";
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        fout << " \\net" << i << "\n"
                 << "  n" << i << "d free\n"
                 << "  n" << i << "s free\n"
                 << "  n" << i << "c free\n"
                 << "  n" << i << "x free\n"
                 << "  n" << i << "y free\n"
                 << "  a" << i << "x free\n"
                 << "  a" << i << "y free\n";
    }

    // General Constraint
    std::vector<Cartesian> ball_pos;
    ball_pos = balls.get_All_Pos();
    fout << "General Constraint\n";
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 alpha + " << outer_netlist[i].pad_pol.angle << " )\n"
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  net" << i << "x: n" << i << "x = POLY ( -" << outer_netlist[i].pad_pol.radius << " n" << i << "c + "
                 << outer_netlist[i].ball_car.x - dice[outer_netlist[i].dice_pads_index[0].first - 1].get_Center().x << " )\n"
                 << "  net" << i << "y: n" << i << "y = POLY ( -" << outer_netlist[i].pad_pol.radius << " n" << i << "s + "
                 << outer_netlist[i].ball_car.y - dice[outer_netlist[i].dice_pads_index[0].first - 1].get_Center().y << " )\n"
                 << "  abs" << i << "x: a" << i << "x = ABS ( n" << i << "x )\n"
                 << "  abs" << i << "x: a" << i << "y = ABS ( n" << i << "y )\n";
    }*/

    // End
    fout << "End";
    return;
}

void Chip::output_M_File(std::ofstream& fout, char* file_name)
{
    /* Get Filename */
    std::string fun_name(file_name);
    fun_name = fun_name.substr(fun_name.rfind('/') + 1);    // get file name(contain .m)
    std::size_t found = fun_name.rfind('.');    // get dot position
    fun_name.erase(fun_name.begin() + found, fun_name.end());   // get file name(no .m)
    fout << "function f = " << fun_name << "(x)\n\tf = ";
    for (size_t i = 0; i < outer_netlist.size(); i++)   // output outer relationship
    {
        if (i != 0) fout << " + ";
        fout << "(" << outer_netlist[i].ball_car.x << "-x(" << outer_netlist[i].dice_pads_index[0].first << ")-"
                 << outer_netlist[i].pad_pol.radius << "*cos(" << outer_netlist[i].pad_pol.angle << "+x(" << 2 * dice.size() + outer_netlist[i].dice_pads_index[0].first << "))).^2 + "
                 << "(" << outer_netlist[i].ball_car.y << "-x(" << dice.size() + outer_netlist[i].dice_pads_index[0].first << ")-"
                 << outer_netlist[i].pad_pol.radius << "*sin(" << outer_netlist[i].pad_pol.angle << "+x(" << 2 * dice.size() + outer_netlist[i].dice_pads_index[0].first << "))).^2";
    }
    for (size_t i = 0; i < inner_netlist.size(); i++)   // output inner relationship
    {
        fout << " + (x(" << inner_netlist[i].dice_pads1_index.first << ")+" << inner_netlist[i].pad1_pol.radius << "*cos(" << inner_netlist[i].pad1_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads1_index.first << "))-x("
                 << inner_netlist[i].dice_pads2_index.first << ")-" << inner_netlist[i].pad2_pol.radius << "*cos(" << inner_netlist[i].pad2_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads2_index.first << "))).^2 + (x("
                 << dice.size() + inner_netlist[i].dice_pads1_index.first << ")+" << inner_netlist[i].pad1_pol.radius << "*cos(" << inner_netlist[i].pad1_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads1_index.first << "))-x("
                 << dice.size() + inner_netlist[i].dice_pads2_index.first << ")-" << inner_netlist[i].pad2_pol.radius << "*cos(" << inner_netlist[i].pad2_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads2_index.first << "))).^2";
    }
    fout << ";";
    return;
}
