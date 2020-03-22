#include"chip.h"

Chip::Chip()
{
    ball = Ball();
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
    std::string pin_name;       // pin name
    std::vector<std::string> balls_name;
    std::vector<Cartesian> balls_cartesian;
    std::vector<Cartesian> dice_center;
    std::vector<std::vector<std::string>> dice_pads_name;
    std::vector<std::vector<Cartesian>> dice_pads_cartesian;
    
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
    ball = Ball(balls_name.size());
    ball.set_All_Name(balls_name);
    ball.set_All_Pos(balls_cartesian);
    dice.resize(dice_amount);
    for (size_t i = 0; i < dice_amount; i++)
    {
        dice[i] = Die(dice_pads_name[i].size(), dice_center[i].x, dice_center[i].y);
        dice[i].set_Pad_Name(dice_pads_name[i]);
        dice[i].set_Cart_Pos(dice_pads_cartesian[i]);
        dice[i].convert_cart_to_polar();
    }

	fin.close();
    std::cout << "parse PIN done\n";
    ball_Content();
    dice_Content();
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
    
    while (getline(fin, str_temp))
	{
        if (str_temp.find(",") != std::string::npos)
        {
            str_temp.erase(str_temp.size() - 1, 1);
            str += str_temp;
        }
		else
        {
            str+=str_temp;
            if (str.find("BGA") != std::string::npos && str.find("DIE") != std::string::npos)   // filter only one kind of pin
            {
                Relationship rela_s;
                std::string sub_str;
                std::string BGA_name;
                std::string DIE_name;
                sub_str.clear();
                BGA_name.clear();
                DIE_name.clear();
                
                std::stringstream ss;
                ss.clear();
                ss << str;
                ss >> sub_str;  // get netlist name
                rela_s.relation_name = sub_str;                 // set relation_name
                ss >> sub_str;  // get ";"
                if (sub_str == ";")
                {
                    std::vector<std::string> BGAs;
                    std::vector<std::string> DIEs;
                    BGAs.clear();
                    DIEs.clear();

                    while (ss >> sub_str)
                    {
                        if (sub_str.find("BGA") != std::string::npos)
                            BGAs.push_back(sub_str);
                        else if (sub_str.find("DIE") != std::string::npos)
                            DIEs.push_back(sub_str);
                        else
                            std::cout << "Warning: netlist has no BGA or DIE\n";
                    }
                    for (size_t i = 0; i < BGAs.size(); i++)
                    {
                        rela_s.ball_index = ball.get_Ball_Index(BGAs[i]);   // set ball_index
                        if (rela_s.ball_index == -1UL)
                        {
                            std::cout << "error: Not found " << BGAs[i] << std::endl;
                            exit(0);
                        }

                        for (size_t j = 0; j < DIEs.size(); j++)
                        {
                            std::stringstream string_cache(DIEs[j].substr(3, DIEs[j].find(".") - 3));
                            string_cache >> rela_s.die_index;           // set die_index
                            rela_s.pad_index = dice[rela_s.die_index - 1].get_Pad_Index(DIEs[j]);   // set pad_index
                            if (rela_s.pad_index == -1UL)
                            {
                                std::cout << "error: Not found " << DIEs[j] << std::endl;
                                exit(0);
                            }
                            netlist.push_back(rela_s);
                        }
                    }
                }
                else
                {
                    std::cout << "error: string has no \";\"\n";
                    exit(0);
                }
            }
            str.clear();
        }
	}
    std::cout << "parse netlist done: Amount of netlist = " << netlist.size() <<std::endl;
    fin.close();
    netlist_Content();
    return;
}

void Chip::parse_Shuffle(std::ifstream& fin)
{
    std::string str;
    str.clear();

    do
    {
        getline(fin, str);
    } while (str.find("$NETS") == std::string::npos);   // if find "$NET"

    while (getline(fin, str))
    {
        if (str.find(";") != std::string::npos)
        {
            std::cout << "test" << str << std::endl;
            std::string sub_str;
            std::stringstream ss;
            sub_str.clear();
            ss.clear();

            ss << str;
            ss >> sub_str;
            int net_index = get_Netlist_Index(sub_str);
            if (net_index != -1)
            {
                ss >> sub_str;  // ignore ";"
                ss >> sub_str;
                if (sub_str.find("BGA") != std::string::npos)
                {
                    size_t ball_index = ball.get_Ball_Index(sub_str);
                    if (ball_index != -1UL)
                    {
                        netlist[net_index].ball_index = ball_index;
                    }
                }
            }
        }
    }
}

void Chip::ball_Content()
{
    std::cout << "\n----------Ball content----------\n"
     << "amount: " << ball.get_Amount() << std::endl;
    std::vector<std::string> name;
    name = ball.get_All_Name();
    std::vector<Cartesian> position;
    position = ball.get_All_Pos();
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
        std::cout << std::setw(4) << i + 1 << " " << std::setw(10) << netlist[i].relation_name << ": "
         << ball.get_Ball_Name(netlist[i].ball_index) << "-->" 
         << dice[netlist[i].die_index - 1].get_Pad_Name(netlist[i].pad_index) << std::endl;
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

    // argument has shuffle netlist file
	if (argc >= 5) {
        index_suffle = argv[4];//get index
		if (index_suffle == "-s") {
			shuffle_fin.open(argv[5]);
			if (!shuffle_fin) {
				std::cout << "file \"" << argv[5] << "\" open error.\n";
				return -1;
            }
            parse_Shuffle(shuffle_fin);
        }
    }

    /*
    std::cout << "number of netlist: " << netlist.size() << std::endl;
    for (size_t index = 0; index < netlist.size(); index++)
    {
        std::cout << netlist[index].relation_name << " : " << netlist[index].ball_index << " --> " << netlist[index].die_pad_index << std::endl;
    }
    */
    
    return 0;
}

int Chip::get_Netlist_Index(std::string str) const
{
    for (size_t t = 0; t <= netlist.size(); t++)
        if (netlist[t].relation_name == str)
            return static_cast<int>(t);
    return -1;
}

void Chip::output_LP_File(std::ofstream& fout)
{
    // Minimize
    fout << "Minimize\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        if (i == 0) fout << "  a0x + a0y";
        else {
            fout << " + a" << i << "x + a" << i << "y";
        }
    }
    fout << "\n";

    // Subject To
    fout << "Subject To\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        if (i == 0) fout << "  goal: [ 2r0 * s0p ] - [ 2r0 * c0p ]";
        else {
            fout << " + [ 2r" << i << " * s" << i << "p ] - [ 2r" << i << " * c" << i << "p ]";
        }
    }
    fout << " = 0\n";

    // Bounds
    std::vector<std::vector<Polar>> dice_pads;
    dice_pads.resize(dice.size());
    for (size_t i = 0; i < dice.size() && i < dice_pads.size(); i++)
        dice_pads[i] = dice[i].get_Pol_Position();
    fout << "Bounds\n";
    fout << "  0 <= x <= " << 2 * M_PI << "\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        fout << " \\net" << i << "\n"
                 << "  2r" << i << " = " << 2 * dice_pads[netlist[i].die_index - 1][netlist[i].pad_index].radius << "\n" 
                 << "  n" << i << "d free\n" 
                 << "  n" << i << "s free\n" 
                 << "  n" << i << "c free\n" 
                 << "  s" << i << "p free\n" 
                 << "  c" << i << "p free\n" 
                 << "  m" << i << "x free\n" 
                 << "  m" << i << "y free\n" 
                 << "  a" << i << "x free\n" 
                 << "  a" << i << "y free\n";
    }

    // General Constraint
    std::vector<Cartesian> ball_pos;
    ball_pos = ball.get_All_Pos();
    fout << "General Constraint\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 x + " << dice_pads[netlist[i].die_index - 1][netlist[i].pad_index].angle << " )\n" 
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  sin" << i << ": s" << i << "p = POLY ( "  << ( ball_pos[netlist[i].ball_index].x + dice[netlist[i].die_index - 1].get_Center().x ) << " n" << i << "s )\n"
                 << "  cos" << i << ": c" << i << "p = POLY ( " << ( ball_pos[netlist[i].ball_index].y + dice[netlist[i].die_index - 1].get_Center().y ) << " n" << i << "c )\n";
        fout << " \\minimize equation\n";
        fout << "  miner" << i << "x: m" << i << "x = POLY ( -" << dice_pads[netlist[i].die_index - 1][netlist[i].pad_index].radius 
                 << " n" << i << "c + " << ball_pos[netlist[i].ball_index].x << " )\n"
                 << "  miner" << i << "y: m" << i << "y = POLY ( -" << dice_pads[netlist[i].die_index - 1][netlist[i].pad_index].radius 
                 << " n" << i << "s + " << ball_pos[netlist[i].ball_index].y << " )\n";
        fout << " \\absolute value\n";
        fout << "  abs" << i << "x: a" << i << "x = ABS ( m" << i << "x )\n" 
                 << "  abs" << i << "y: a" << i << "y = ABS ( m" << i << "y )\n";
    }

    // End
    fout << "End";
    return;
}
