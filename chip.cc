#include"chip.h"

Chip::Chip()
{
    balls = Ball();
    dice.clear();
    netlist.clear();
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

    netlist.clear();
    do
	{
		getline(fin, str_temp);
	} while (str_temp.find("$NETS") == std::string::npos);   // if find "$NET"
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
            ss >> netlist_name;  // get netlist name

            if (ignore_P_G && (
                netlist_name.find("P") != std::string::npos || 
                netlist_name.find("V") != std::string::npos || 
                netlist_name.find("VCC") != std::string::npos || 
                netlist_name.find("VDD") != std::string::npos || 
                netlist_name.find("G") != std::string::npos || 
                netlist_name.find("VSS") != std::string::npos
                ))
            {
                std::cout << "ignore " << str <<std::endl;
            }
            else if (str.find("BGA") != std::string::npos && str.find("DIE") != std::string::npos)   // filter only one kind of pin
            {
                Relationship rela_s;
                std::string sub_str;
                std::string BGA_name;
                std::string DIE_name;
                sub_str.clear();
                BGA_name.clear();
                DIE_name.clear();
                
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
                    
                    // Decide to do combination or calculate center of gravity
                    if (BGAs.size() * DIEs.size() <= 10)
                    {
                        // To do combination
                        for (size_t i = 0; i < BGAs.size(); i++)
                        {
                            rela_s.balls_index.clear();
                            rela_s.balls_index.push_back(balls.get_Ball_Index(BGAs[i]));   // set ball_index
                            if (rela_s.balls_index[0] == -1UL)
                            {
                                std::cout << "error: Not found " << BGAs[i] << std::endl;
                                exit(0);
                            }
                            rela_s.ball_car.x += balls.get_All_Pos().at(rela_s.balls_index[0]).x;
                            rela_s.ball_car.y += balls.get_All_Pos().at(rela_s.balls_index[0]).y;

                            for (size_t j = 0; j < DIEs.size(); j++)
                            {
                                rela_s.dice_pads_index.resize(1);
                                if (BGAs.size() != 1 || DIEs.size() != 1)   // if has multi-BGA or multi_DIE, netlist name append number
                                    rela_s.relation_name = netlist_name + std::to_string(i) + "_" + std::to_string(j);
                                std::stringstream string_cache(DIEs[j].substr(3, DIEs[j].find(".") - 3));
                                string_cache >> rela_s.dice_pads_index[0].first;           // set die_index
                                rela_s.dice_pads_index[0].second = dice[rela_s.dice_pads_index[0].first - 1].get_Pad_Index(DIEs[j]);   // set pad_index
                                if (rela_s.dice_pads_index[0].second == -1UL)
                                {
                                    std::cout << "error: Not found " << DIEs[j] << std::endl;
                                    exit(0);
                                }
                                rela_s.pad_car.x += dice[rela_s.dice_pads_index[0].first - 1].get_Cart_Position().at(rela_s.dice_pads_index[0].second).x;
                                rela_s.pad_car.y += dice[rela_s.dice_pads_index[0].first - 1].get_Cart_Position().at(rela_s.dice_pads_index[0].second).y;
                                rela_s.pad_pol.radius += dice[rela_s.dice_pads_index[0].first - 1].get_Pol_Position().at(rela_s.dice_pads_index[0].second).radius;
                                rela_s.pad_pol.angle += dice[rela_s.dice_pads_index[0].first - 1].get_Pol_Position().at(rela_s.dice_pads_index[0].second).angle;
                                netlist.push_back(rela_s);
                            }
                        }
                    }
                    else
                    {
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
                        netlist.push_back(rela_s);
                    }
                }
                else
                {
                    std::cout << "error: string has no \";\"\n";
                    exit(0);
                }
            }
            else
            {
                std::cout << "warring: string \"" << str << "\" has no \"BGA\" and \"DIE\"\n";
            }
            str.clear();
        }
	}

    // Cartesian convert to Polar
    double x = 0.0, y = 0.0;
    for (size_t i = 0; i < netlist.size(); i++)
    {
        x = netlist[i].pad_car.x;
        y = netlist[i].pad_car.y;

        // calculate radius
        netlist[i].pad_pol.radius = sqrt(x * x + y * y);
        //std::cout << "\tradius = " << pol_position[i].radius;

        // calculate theta
        x /= netlist[i].pad_pol.radius;
        y /= netlist[i].pad_pol.radius;
        if (x >= 0 && y >= 0){   //Quadrant I
            netlist[i].pad_pol.angle = (asin(y) + acos(x)) / 2;
        }
        else if (x < 0 && y >= 0){  //Quadrant II
            netlist[i].pad_pol.angle = acos(x);
        }
        else if (x < 0 && y < 0){  //Quadrant III
            netlist[i].pad_pol.angle = acos(x) - 2 * asin(y);
        }
        else if (x >= 0 && y < 0){  //Quadrant IV
            netlist[i].pad_pol.angle = (2 * M_PI + asin(y));
        }
        //std::cout << "\ttheta = " << pol_position[i].angle << std::endl;
    }

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
                            << " | r: " << std::setw(10)<< polar_position[i].radius 
                            << ", theta: "<< std::setw(10) << polar_position[i].angle * 180 / M_PI << std::endl;
        }
    }
    std::cout << "----------Die content----------\n";
    return;
}

void Chip::netlist_Content()
{
    std::cout << "\n----------Netlist content----------\n"
     << "amount: " << netlist.size() << std::endl;
    for (size_t i = 0; i < netlist.size(); i++)
    {
        std::cout << std::setw(4) << i + 1 << " " << std::setw(10) << netlist[i].relation_name << ": ";
        for (size_t j = 0; j < netlist[i].balls_index.size(); j++)
            std::cout << "\n\t" << balls.get_Ball_Name(netlist[i].balls_index[j]);
        std::cout << "\n\t-->\n";
        for (size_t j = 0; j < netlist[i].dice_pads_index.size(); j++)
            std::cout << "\t" << dice[netlist[i].dice_pads_index[j].first - 1].get_Pad_Name(netlist[i].dice_pads_index[j].second) << std::endl;
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
    for (size_t i = 0; i < netlist.size(); i++)
    {
        position_sum.x += netlist[i].ball_car.x + netlist[i].pad_car.x;
        position_sum.y += netlist[i].ball_car.y + netlist[i].pad_car.y;
    }
    dice[0].set_Center(Cartesian(position_sum.x / static_cast<double>(netlist.size()), position_sum.y / static_cast<double>(netlist.size())));
*/
    /*
    std::cout << "number of netlist: " << netlist.size() << std::endl;
    for (size_t index = 0; index < netlist.size(); index++)
    {
        std::cout << netlist[index].relation_name << " : " << netlist[index].ball_index << " --> " << netlist[index].die_pad_index << std::endl;
    }
    */
    balls_Content();
    dice_Content();
    netlist_Content();
    
    return 0;
}

size_t Chip::get_Netlist_Index(std::string str) const
{
    for (size_t t = 0; t <= netlist.size(); t++)
        if (netlist[t].relation_name == str)
            return t;
    return -1UL;
}

void Chip::output_LP_File(std::ofstream& fout)
{
    /*
    // Minimize
    if (min_output)
    {
        fout << "Minimize\n";
        for (size_t i = 0; i < netlist.size(); i++) {
            if (i == 0) fout << "  a0x + a0y";
            else {
                fout << " + a" << i << "x + a" << i << "y";
            }
        }
        fout << "\n";
    }
    
    // Subject To
    fout << "Subject To\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        if (i == 0) fout << "  goal: [ r0_2 * s0p ] - [ r0_2 * c0p ]";
        else {
            fout << " + [ r" << i << "_2 * s" << i << "p ] - [ r" << i << "_2 * c" << i << "p ]";
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
    for (size_t i = 0; i < netlist.size(); i++) {
        fout << " \\net" << i << "\n"
                 << "  r" << i << "_2 = " << 2 * netlist[i].pad_pol.radius << "\n"
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
    for (size_t i = 0; i < netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 alpha + " << netlist[i].pad_pol.angle << " )\n" 
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  sin" << i << ": s" << i << "p = POLY ( "  << ( netlist[i].ball_car.x + dice[netlist[i].dice_pads_index[0].first - 1].get_Center().x ) << " n" << i << "s )\n"
                 << "  cos" << i << ": c" << i << "p = POLY ( " << ( netlist[i].ball_car.y + dice[netlist[i].dice_pads_index[0].first - 1].get_Center().y ) << " n" << i << "c )\n";
        if (min_output)
        {
            fout << " \\minimize equation\n";
            fout << "  miner" << i << "x: m" << i << "x = POLY ( -" << netlist[i].pad_pol.radius 
                 << " n" << i << "c + " << netlist[i].ball_car.x << " )\n"
                 << "  miner" << i << "y: m" << i << "y = POLY ( -" << netlist[i].pad_pol.radius
                 << " n" << i << "s + " << netlist[i].ball_car.y << " )\n";
            fout << " \\absolute value\n"
                     << "  abs" << i << "x: a" << i << "x = ABS ( m" << i << "x )\n" 
                     << "  abs" << i << "y: a" << i << "y = ABS ( m" << i << "y )\n";
        }
    }
    */

   /* other LP file */
   // Subject To
    fout << "Subject To\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        double radius_2 = 2.0 * netlist[i].pad_pol.radius;
        if (i == 0) fout << "  goal: " << radius_2 << " s0p - " << radius_2 << " c0p";
        else {
            fout << " + " << radius_2 << " s" << i << "p - " << radius_2 << " c" << i << "p";
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
    for (size_t i = 0; i < netlist.size(); i++) {
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
    
    // End
    fout << "End";
    return;
}
