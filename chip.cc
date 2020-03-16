#include"chip.h"

Chip::Chip()
{
    die = Die();
    ball = Ball();
    netlist.clear();
}

void Chip::parse_Die(std::vector<std::vector<std::string>> str_str_array)
{
    std::vector<std::string> *all_pad_name;
    all_pad_name = new std::vector<std::string>;

    std::vector<Cartesian> *all_pad_position;
    all_pad_position = new std::vector<Cartesian>;

    // int index = 0, center_x = 0, center_y = 0;

    die = Die(static_cast<int>(str_str_array.size()), 0, 0);
    all_pad_name->resize(die.get_Amount());
    all_pad_position->resize(die.get_Amount());
    
    for (size_t i = 0; i < str_str_array.size(); i++) {
        assert(static_cast<int>(str_str_array[i].size()) == 6);
        if (die.get_Amount() != 0) {
            all_pad_name->at(i) = str_str_array[i][0];
            all_pad_position->at(i).x = stod(str_str_array[i][1]);
            all_pad_position->at(i).y = stod(str_str_array[i][2]);
        }
    }
    die.set_Pad_Name(*all_pad_name);
    die.set_Cart_Pos(*all_pad_position);
    return;
}

void Chip::parse_Ball(std::vector<std::vector<std::string>> str_str_array)
{
    std::vector<std::string> *all_ball_name;
    all_ball_name = new std::vector<std::string>;

    std::vector<Cartesian> *all_ball_position;
    all_ball_position = new std::vector<Cartesian>;

    ball.set_Amount(static_cast<int>(str_str_array.size()));
    all_ball_name->resize(ball.get_Amount());
    all_ball_position->resize(ball.get_Amount());

    for (size_t i = 0; i < str_str_array.size(); i++) {
        assert(static_cast<int>(str_str_array[i].size()) == 6);
        if (ball.get_Amount() != 0) {
            all_ball_name->at(i) = str_str_array[i][3];
            all_ball_position->at(i).x = stod(str_str_array[i][4]);
            all_ball_position->at(i).y = stod(str_str_array[i][5]);
        }
    }
    ball.set_All_Name(*all_ball_name);
    ball.set_All_Pos(*all_ball_position);
    return;
}

void Chip::die_Content()
{
    std::cout << "\n----------Die content----------\n"
     << "center: " << die.get_Center().x << " " << die.get_Center().y
     << "\namount: " << die.get_Amount() << std::endl;
    std::vector<std::string> name;
    name = die.get_Pad_Name();
    std::vector<Cartesian> cart_position;
    cart_position = die.get_Cart_Position();
    std::vector<Polar> polar_position;
    polar_position = die.get_Pol_Position();
    assert(cart_position.size() == polar_position.size());
    for (size_t i = 0; i < cart_position.size() && i < polar_position.size(); i++) {
        std::cout << std::fixed << std::setprecision(3) << std::setw(4) << i + 1 << " " << std::setw(8) << name[i]
                        << " x: " << std::setw(10) << cart_position[i].x 
                        << ", y: " << std::setw(10)<< cart_position[i].y
                        << " | r: " << std::setw(10)<< polar_position[i].radius 
                        << ", theta: "<< std::setw(10) << polar_position[i].angle * 180 / M_PI << std::endl;
    }
    std::cout << "----------Die content----------\n";
    return;
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

void Chip::netlist_Content()
{
    std::cout << "\n----------Netlist content----------\n"
     << "amount: " << netlist.size() << std::endl;
    for (size_t i = 0; i < netlist.size(); i++)
        std::cout << std::setw(4) << i + 1 << " " << std::setw(10) << netlist[i].relation_name << ": "
         << ball.get_Ball_Name(netlist[i].ball_index) << "-->" << die.get_Pad_Name(netlist[i].die_pad_index) << std::endl;
    std::cout << "----------Netlist content----------\n";
    return;
}

int Chip::parser(int argc, char** argv)
{
    int sub_str_pos = -1;
	bool isBall = false;
	bool isDie = false;
    std::string str;
	std::string index_suffle;
    std::string pin_name;       // pin name
    std::ifstream PIN_fin;		// PIN
	std::ifstream netlist_fin;	// netlist
	std::ifstream shuffle_fin;	// shuffle netlist
	std::ofstream fout;			// relationship
    std::vector<std::string> balls_name;
    std::vector<Cartesian> balls_cartesian;
    Cartesian die_center;
    std::vector<std::string> die_pads_name;
    std::vector<Cartesian> die_pads_cartesian;

    str.clear();
    index_suffle.clear();
    pin_name.clear();
    PIN_fin.clear();
    netlist_fin.clear();
    shuffle_fin.clear();
    fout.clear();
    balls_name.clear();
    balls_cartesian.clear();
    die_center.x = 0;
    die_center.y = 0;
    die_pads_name.clear();
    die_pads_cartesian.clear();

    PIN_fin.open(argv[1]);
	netlist_fin.open(argv[2]);

	if (!PIN_fin) {
		std::cout << "file \"" << argv[1] << "\" open error.";
		return -1;
	}
	if (!netlist_fin) {
		std::cout << "file \"" << argv[2] << "\" open error.";
		return -1;
	}

    while (getline(PIN_fin, str))
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
				die_pads_name.push_back(pin_name);
                die_pads_cartesian.push_back(pin_cartesian);
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
    if (balls_name.size() == balls_cartesian.size())
    {
        ball = Ball(balls_name.size());
        ball.set_All_Name(balls_name);
        ball.set_All_Pos(balls_cartesian);
    }
    else
    {
        std::cout << "error: the Balls number of names and xys are different.\n";
        exit(0);
    }
    if (die_pads_name.size() == die_pads_cartesian.size())
    {
        die = Die(die_pads_name.size(), die_center.x, die_center.y);
        die.set_Pad_Name(die_pads_name);
        die.set_Cart_Pos(die_pads_cartesian);
        die.convert_cart_to_polar();
    }
    else
    {
        std::cout << "error: the Die Pads number of names and xys are different.\n";
        exit(0);
    }
	PIN_fin.close();
    //ball_Content();
    //die_Content();

    netlist.clear();
    do
	{
		getline(netlist_fin, str);
	} while (str.find("$NETS") == std::string::npos);   // if find "$NET"
    // start to parse netlist
    
    while (getline(netlist_fin, str))
	{
		if (str.find("BGA") != std::string::npos && str.find("DIE") != std::string::npos)
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
            rela_s.relation_name = sub_str;
            ss >> sub_str;  // get ";"
            if (sub_str == ";")
            {
                ss >> sub_str;
                if (sub_str.find("BGA") != std::string::npos)
                {
                    rela_s.ball_index = ball.get_Ball_Index(sub_str);
                    if (rela_s.ball_index == -1)
                    {
                        std::cout << "error: Not found " << sub_str << std::endl;
                        exit(0);
                    }
                }
                else
                {
                    std::cout << "error: netlist member has no BGA item\n";
                    exit(0);
                }

                while (ss >> sub_str)
                {
                    if (sub_str.find("DIE") != std::string::npos)
                    {
                        rela_s.die_pad_index = die.get_Pad_Index(sub_str);
                         if (rela_s.die_pad_index == -1)
                        {
                            std::cout << "error: Not found " << sub_str << std::endl;
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
	}

    // argument has shuffle netlist file
	if (argc >= 5) {
        index_suffle = argv[4];//get index
		if (index_suffle == "-s") {
			shuffle_fin.open(argv[5]);
			if (!shuffle_fin) {
				std::cout << "file \"" << argv[5] << "\" open error.\n";
				return -1;
            }
			else {
                do
                {
                    getline(shuffle_fin, str);
                } while (str.find("$NETS") == std::string::npos);   // if find "$NET"

                while (getline(shuffle_fin, str))
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
                                int ball_index = ball.get_Ball_Index(sub_str);
                                if (ball_index != -1)
                                {
                                    netlist[net_index].ball_index = ball_index;
                                }
                            }
                        }
                    }
                }
            }
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
    std::vector<Polar> pad;
    pad = die.get_Pol_Position();
    fout << "Bounds\n";
    fout << "  0 <= x <= " << 2 * M_PI << "\n";
    for (size_t i = 0; i < netlist.size(); i++) {
        fout << " \\net" << i << "\n"
                 << "  2r" << i << " = " << 2 * pad[netlist[i].die_pad_index].radius << "\n" 
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
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 x + " << pad[netlist[i].die_pad_index].angle << " )\n" 
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  sin" << i << ": s" << i << "p = POLY ( "  << ( ball_pos[netlist[i].ball_index].x + die.get_Center().x ) << " n" << i << "s )\n"
                 << "  cos" << i << ": c" << i << "p = POLY ( " << ( ball_pos[netlist[i].ball_index].y + die.get_Center().y ) << " n" << i << "c )\n";
        fout << " \\minimize equation\n";
        fout << "  miner" << i << "x: m" << i << "x = POLY ( -" << pad[netlist[i].die_pad_index].radius 
                 << " n" << i << "c + " << ball_pos[netlist[i].ball_index].x << " )\n"
                 << "  miner" << i << "y: m" << i << "y = POLY ( -" << pad[netlist[i].die_pad_index].radius 
                 << " n" << i << "s + " << ball_pos[netlist[i].ball_index].y << " )\n";
        fout << " \\absolute value\n";
        fout << "  abs" << i << "x: a" << i << "x = ABS ( m" << i << "x )\n" 
                 << "  abs" << i << "y: a" << i << "y = ABS ( m" << i << "y )\n";
    }

    // End
    fout << "End";
    return;
}
