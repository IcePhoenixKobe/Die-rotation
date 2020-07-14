#include"file_analyst.h"

using namespace std;

/* parse basic information of chip */
void parse_Drc(std::ifstream& fin)
{
	string str, info_name;
    stringstream ss;

    str.clear();
    ss.clear();

    while (getline(fin, str))
	{
        ss.clear();
        info_name.clear();

        ss << str;
        ss >> info_name;

        if (info_name.find("packagesizeX") != string::npos) {
            ss >> chip->drc.packageSize.x;
        }
        else if (info_name.find("packagesizeY") != string::npos) {
            ss >> chip->drc.packageSize.y;
        }
        else if (info_name.find("ballpitch") != string::npos) {
            ss >> chip->drc.ballPitch;
        }
        else if (info_name.find("balldiameter") != string::npos) {
            ss >> chip->drc.ballDiameter;
        }
        else if (info_name.find("balldimensionX") != string::npos) {
            ss >> chip->drc.ballDimensionX;
        }
        else if (info_name.find("balldimensionY") != string::npos) {
            ss >>  chip->drc.ballDimensionY;
        }
        else if (info_name.find("viadiameter") != string::npos) {
            ss >> chip->drc.viaDiameter;
        }
        else if (info_name.find("wirewidth") != string::npos) {
            ss >> chip->drc.wireWidth;
        }
        else if (info_name.find("spacing") != string::npos) {
            ss >> chip->drc.spacing;
        }
        else if (info_name.find("numlayer") != string::npos) {
            ss >> chip->drc.numLayer;
        }
        else {
            cout << "skip " << info_name << endl;
        }
    }

	fin.close();
    cout << "parse drc done\n";
    return;
}

/* parse all pin of chip */
void parse_PIN(ifstream& fin)
{
	bool isBall = false;
	bool isDie = false;
	size_t die_number = 0, dice_amount = 0;
	string str;
    string temp_str;
	string pin_name;    // temp variable of item name
    Cartesian pin_cartesian(0.0, 0.0);
    stringstream str_stream;    // string stream of each line of file
	vector<string> balls_name;
	vector<Cartesian> balls_cartesian;  // all BGA balls position
	vector<Cartesian> dice_center;  // all dice center
	vector<vector<string>> dice_pads_name;       // all pads name
	vector<vector<Cartesian>> dice_pads_cartesian;    // all pads position
    vector<vector<double>> dice_pads_rotation;
    
    str.clear();
    temp_str.clear();
    pin_name.clear();
    str_stream.clear();
    str_stream.str("");
    balls_name.clear();
    balls_cartesian.clear();
    dice_center.clear();
    dice_pads_name.clear();
    dice_pads_cartesian.clear();
    dice_pads_rotation.clear();

    while (getline(fin, str))
	{
        str_stream.clear();
        str_stream.str(str);
        str_stream >> temp_str; // get first string of line
        //cout << "temp_str: " << temp_str << endl;

        // first: get pin number
		if (temp_str.find("pin") != string::npos)
        {
            str_stream >> temp_str; // get "number:"
            if (temp_str.find("number:") != string::npos)
            {
                str_stream >> temp_str; // get pin number
                if (temp_str.find("BGA") != string::npos)
                    isBall = true;
                else if (temp_str.find("DIE") != string::npos)
                    isDie = true;
                else
                {
                    isBall = false;
                    isDie = false;
                }

                if (isBall || isDie)
                {
                    pin_name = temp_str;
                    if (isDie)
                    {
                        stringstream string_cache(pin_name.substr(3, pin_name.find(".") - 3));
                        string_cache >> die_number;
                    }
                }
            }
        }
        
		// second: get position of die pad or BGA ball
		if (temp_str.find("location-xy:") != string::npos) // if found position of pad or ball
		{
			str_stream >> temp_str;  // x coordinate(e.g. "(0.00")
			temp_str.erase(0, 1);    // clean left quote
			pin_cartesian.x = stod(temp_str);
			str_stream >> temp_str;  // y coordinate(e.g. "0.00)")
			temp_str.erase(temp_str.size() - 1, 1);//clean right quote
			pin_cartesian.y = stod(temp_str);
        }

        // third: get rotation of pad
        if (temp_str.find("padstack") != string::npos && (isDie || isBall))
        {
            str_stream >> temp_str;
            if (temp_str.find("rotation:") != string::npos)
            {
                double rotation = 0.0;
                str_stream >> rotation;
                rotation = rotation * M_PI / 180;

                // set data
                if (isBall && !isDie)   // ball position
                {
                    balls_name.push_back(pin_name);
                    balls_cartesian.push_back(pin_cartesian);
                    isBall = false;
                }
                else if (isDie && !isBall)  // die pad position
                {
                    if (die_number <= dice_amount)  // direct assign
                    {
                        dice_pads_name[die_number - 1].push_back(pin_name);
                        dice_pads_cartesian[die_number - 1].push_back(pin_cartesian);
                        dice_pads_rotation[die_number - 1].push_back(rotation);
                    }
                    else
                    {
                        while (dice_amount < die_number)    // new dice until dice_amount equal to die_number
                        {
                            vector<string> pads_name;
                            vector<Cartesian> pads_cartesian;
                            vector<double> pads_rotation;

                            pads_name.clear();
                            pads_cartesian.clear();
                            pads_rotation.clear();

                            dice_center.push_back(Cartesian(0, 0));
                            if (dice_amount == die_number - 1)  // assign current data into chip
                            {
                                pads_name.push_back(pin_name);
                                pads_cartesian.push_back(pin_cartesian);
                                pads_rotation.push_back(rotation);
                            }
                            dice_pads_name.push_back(pads_name);
                            dice_pads_cartesian.push_back(pads_cartesian);
                            dice_pads_rotation.push_back(pads_rotation);

                            dice_amount++;
                        }
                    }
                }
                else
                {
                    cout << "error: At " << pin_name << ", isBall and isDie at the same times.\n";
                    exit(0);
                }
            }
        }

        // fourth: get center of die
        if (temp_str.find("Symbol:") != string::npos && isDie) // if found center of die
        {
            str_stream >> temp_str;
            str_stream >> temp_str;
            assert(temp_str.find("at") != string::npos);

            str_stream >> dice_center[die_number - 1].x;  // get x coordinate of center
            str_stream >> temp_str;
            assert(temp_str.find(",") != string::npos);
            str_stream >> dice_center[die_number - 1].y;  // get x coordinate of center
            
            isDie = false;
        }
	}
    
    // parse done and do set
    assert(balls_name.size() == balls_cartesian.size());
    chip->set_Balls_Amount(balls_name.size());
    chip->set_Balls_Name(balls_name);
    chip->set_Balls_Position(balls_cartesian);

    chip->set_Dice_Amount(dice_amount);
    assert(dice_center.size() == dice_amount);
    for (size_t i = 0; i < dice_amount; i++)
    {
        assert(dice_pads_name[i].size() == dice_pads_cartesian[i].size());
        assert(dice_pads_cartesian[i].size() == dice_pads_rotation[i].size());

        if (dice_amount == 1) dice_center[i] = Cartesian(0.0, 0.0);
        chip->set_Die_Amount_Center(i, dice_pads_name[i].size(), dice_center[i]);

        // GOD hand
        if (ori_rotas.size() != 0)
        {
            if (i == 0)
                chip->set_Die_Center(0, GOD_Center);
            else if (i == 1)
                chip->set_Die_Center(1, GOD_GOD_Center);
            
            for (size_t j = 0; j < dice_pads_name[i].size(); j++)
            {
                double temp_rotation = dice_pads_rotation[i][j];

                //double temp_rotation = dice_pads_rotation[i][j];
                while(temp_rotation >= M_PI_2) temp_rotation -= M_PI_2;
                while (temp_rotation <= -M_PI_2) temp_rotation += M_PI_2;

                dice_pads_cartesian[i][j] = Cartesian(
                        (
                            (dice_pads_cartesian[i][j].x - dice_center[i].x) * cos(-temp_rotation) 
                            - (dice_pads_cartesian[i][j].y - dice_center[i].y) * sin(-temp_rotation)
                        ) + dice_center[i].x
                    , (
                            (dice_pads_cartesian[i][j].x - dice_center[i].x) * sin(-temp_rotation) 
                            + (dice_pads_cartesian[i][j].y - dice_center[i].y) * cos(-temp_rotation)
                        ) + dice_center[i].y
                );

                if (i == 0) {
                    dice_pads_cartesian[i][j].x += (-dice_center[i].x + GOD_Center.x);
                    dice_pads_cartesian[i][j].y += (-dice_center[i].y + GOD_Center.y);
                    dice_pads_rotation[i][j] += GOD_Rotation;
                    temp_rotation += GOD_Rotation;
                }
                else if (i == 1) {
                    dice_pads_cartesian[i][j].x += (-dice_center[i].x + GOD_GOD_Center.x);
                    dice_pads_cartesian[i][j].y += (-dice_center[i].y + GOD_GOD_Center.y);
                    dice_pads_rotation[i][j] += GOD_GOD_Rotation;
                    temp_rotation += GOD_GOD_Rotation;
                }
                while(temp_rotation >= M_PI_2) temp_rotation -= M_PI_2;
                while (temp_rotation <= -M_PI_2) temp_rotation += M_PI_2;

                dice_pads_cartesian[i][j] = Cartesian(
                        (
                            (dice_pads_cartesian[i][j].x - chip->get_Die_Center(i).x) * cos(temp_rotation) 
                            - (dice_pads_cartesian[i][j].y - chip->get_Die_Center(i).y) * sin(temp_rotation)
                        ) + chip->get_Die_Center(i).x
                    , (
                            (dice_pads_cartesian[i][j].x - chip->get_Die_Center(i).x) * sin(temp_rotation) 
                            + (dice_pads_cartesian[i][j].y - chip->get_Die_Center(i).y) * cos(temp_rotation)
                        ) + chip->get_Die_Center(i).y
                );
            }
        }

        // final seting
        chip->set_Die_Pads(i, dice_pads_name[i], dice_pads_cartesian[i], dice_pads_rotation[i]);
    }

    

	fin.close();
    cout << "parse pin done\n";
    return;
}

/* parse netlist of chip */
void parse_Netlist(ifstream& fin)
{
    string str, str_temp;   // str_temp: get multi line netlist and combine into str
    vector<string> BGAs;
    vector<string> DIEs;
    vector<InnerRelationship> inner_netlist;
    vector<OuterRelationship> outer_netlist;

    str.clear();
    str_temp.clear();
    inner_netlist.clear();
    outer_netlist.clear();

    // Try to find "$NET"
    do getline(fin, str_temp); while (str_temp.find("$NETS") == string::npos && !fin.eof());

    if (!fin.eof()) // found $NET
    {
        // start to parse single die netlist
        while (getline(fin, str_temp) && str_temp.find("$END") == string::npos)
        {
            if (str_temp.find(",") != string::npos)    // concatenate multi-line
            {
                str_temp.erase(str_temp.size() - 1, 1);
                str += str_temp;
            }
            else    // analysis net
            {
                string netlist_name;
                stringstream ss;

                netlist_name.clear();
                ss.clear();

                str += str_temp;
                ss << str;
                ss >> netlist_name;  // get outer_netlist name

                if (ignore_Power_Ground(netlist_name))
                {}
                else if (str.find("BGA") != string::npos && str.find("DIE") != string::npos)   // filter only one kind of pin
                {
                    OuterRelationship rela_s;
                    string sub_str;
                    sub_str.clear();
                    
                    rela_s.relation_name = netlist_name;    // set relation_name
                    ss >> sub_str;  // get ";"
                    if (sub_str.find(";") != string::npos)
                    {
                        BGAs.clear();
                        DIEs.clear();

                        // Categories balls and dice pads
                        while (ss >> sub_str)
                        {
                            if (sub_str.find("BGA") != string::npos)
                                BGAs.push_back(sub_str);
                            else if (sub_str.find("DIE") != string::npos)
                                DIEs.push_back(sub_str);
                            else
                                cout << "Warning: netlist has no BGA or DIE\n";
                        }
                        
                        // Get all data of balls
                        vector<Cartesian> cars;
                        vector<size_t> balls_index;

                        cars.clear();
                        balls_index.clear();
                        for (size_t i = 0; i < BGAs.size(); i++)
                        {
                            size_t index = chip->get_Ball_Index(BGAs[i]);   // set ball_index
                            if (index == -1UL)
                            {
                                cout << "error: Not found " << BGAs[i] << endl;
                                exit(0);
                            }
                            else
                            {
                                cars.push_back(chip->get_Ball_Position(index));
                                balls_index.push_back(index);
                            }
                        }
                        
                        // set data of balls
                        rela_s.balls_index = balls_index;
                        rela_s.ball_car = CG(cars);
                        
                        // Get all data of die pads
                        vector<vector<size_t>> dice_pads_index;
                        
                        cars.clear();
                        dice_pads_index.clear();
                        for (size_t i = 0; i < DIEs.size(); i++)
                        {
                            pair<size_t, size_t> index(-1UL, -1UL);
                            stringstream string_cache(DIEs[i].substr(3, DIEs[i].find(".") - 3));

                            string_cache >> index.first;    // set die_index
                            for (size_t i = dice_pads_index.size(); i < index.first; i++) {
                                dice_pads_index.push_back(vector<size_t>());
                            }

                            index.second = chip->get_Die_Pad_Index(index.first - 1, DIEs[i]);   // set pad_index
                            if (index.second == -1UL)
                            {
                                cout << "error: Not found " << DIEs[i] << endl;
                                exit(0);
                            }
                            else
                            {
                                cars.push_back(chip->get_Die_Pad_Position(index.first - 1, index.second));
                                dice_pads_index[index.first - 1].push_back(index.second);
                            }

                            // assign data of dice pads
                            rela_s.dice_pads_index = dice_pads_index;
                            rela_s.pad_car = CG(cars);
                        }
                        outer_netlist.push_back(rela_s);
                    }
                    else
                    {
                        cout << "error: string has no \";\"\n";
                        exit(0);
                    }
                }
                else
                {
                    cout << "warring: string \"" << str.substr(0, str.size() - 1) << "\" has no BGA and DIE\n";
                }
                str.clear();
            }
        }
    }
    else
    {
        // start to parse multi_netlist
        cout << "There is no \"$NET\" in netlist file\n";
        cout << "Use Net Name to parse netlist file\n";
        fin.clear();
        fin.seekg(ios::beg);   // Seek to begin of fin
        BGAs.clear();
        DIEs.clear();

        bool isNet = false;
        string temp_str, netlist_name;
        stringstream str_stream;    // string stream of each line of file
        OuterRelationship rela_s_O;
        InnerRelationship rela_s_I;
        vector<int> die_number;    // Store the die number that appeared
        
        temp_str.clear();
        netlist_name.clear();
        str_stream.clear();
        str_stream.str("");
        die_number.clear();

        while (getline(fin, str))
        {
            str_stream.clear();
            str_stream.str(str);
            str_stream >> temp_str; // get first string of line

            // first: get netlist name(Net Name)
            if (temp_str.find("Net") != string::npos)
            {
                str_stream >> temp_str; // get "Name:"
                if (temp_str.find("Name:") != string::npos)
                {
                    str_stream >> netlist_name;  // get netlist name
                    
                    if (!ignore_Power_Ground(netlist_name))  // ignore power and ground if ignore_P_G is true
                    {
                        while(getline(fin, str) && str.find("---") == string::npos);    // skip middle data
                        isNet = true;
                    }
                }
            }

            // second: get all pins and classify
		    if (isNet)
		    {
                BGAs.clear();
                DIEs.clear();

                // get all pins of net
                while(getline(fin,str) && ( str.find("BGA")  != string::npos || str.find("DIE")  != string::npos ))
                {
                    str_stream.clear();
                    str_stream.str(str);
                    str_stream >> temp_str;  // get pin name

                    if (temp_str.find("BGA") != string::npos)
                        BGAs.push_back(temp_str);
                    else
                    {
                        bool not_appeared = true;
                        int pin_number = atoi(temp_str.substr(3, temp_str.find('.') - 3).c_str());
                        
                        // Check if pin_number has appeared
                        for (size_t i = 0; i < die_number.size() && not_appeared; i++)  if (die_number[i] == pin_number) not_appeared = false;
                        // If pin_number has not appeared, add pin_number into die_number
                        if (not_appeared) die_number.push_back(pin_number);

                        DIEs.push_back(temp_str);
                    }
                }

                getline(fin,str);   // skip space line

                // There is some problem maybe!!!
                if (str.find("No connections remaining") == string::npos)
                    cout << "Warning: netlist ‘" << netlist_name << "‘  has no BGA or DIE, and may have some pin miss catch.\n";
                
                // Decide to do combination or calculate center of gravity
                if (BGAs.size() != 0 && DIEs.size() != 0)   // BGA balls to die pads
                {
                    // Calculate the center of gravity of BGA balls 
                    vector<Cartesian> cars;
                    vector<size_t> balls_index;
                    balls_index.clear();

                    rela_s_O = OuterRelationship();
                    rela_s_O.relation_name = netlist_name;  // set relation_name
                    for (size_t i = 0; i < BGAs.size(); i++)
                    {
                        size_t index = chip->get_Ball_Index(BGAs[i]);   // set ball_index
                        if (index == -1UL)
                        {
                            cout << "error: Not found " << BGAs[i] << endl;
                            exit(0);
                        }
                        else
                        {
                            cars.push_back(chip->get_Ball_Position(index));
                            balls_index.push_back(index);
                        }
                    }
                    rela_s_O.balls_index = balls_index;
                    rela_s_O.ball_car = CG(cars);
                    
                    // Calculate the center of gravity of die pads 
                    vector<vector<size_t>> dice_pads_index;

                    cars.clear();
                    dice_pads_index.clear();
                    for (size_t i = 0; i < DIEs.size(); i++)
                    {
                        pair<size_t, size_t> index(-1UL, -1UL);
                        stringstream string_cache(DIEs[i].substr(3, DIEs[i].find(".") - 3));
                        
                        string_cache >> index.first;           // set die_index
                        for (size_t i = dice_pads_index.size(); i < index.first; i++) {
                            dice_pads_index.push_back(vector<size_t>());
                        }
                        index.second = chip->get_Die_Pad_Index(index.first - 1, DIEs[i]);   // set pad_index
                        if (index.second == -1UL)
                        {
                            cout << "error: Not found " << DIEs[i] << endl;
                            exit(0);
                        }
                        else
                        {
                            cars.push_back(chip->get_Die_Pad_Position(index.first - 1, index.second));
                            dice_pads_index[index.first - 1].push_back(index.second);
                        }
                    }
                    rela_s_O.dice_pads_index = dice_pads_index;
                    rela_s_O.pad_car = CG(cars);
                    
                    // insert relationship into outer netlist
                    outer_netlist.push_back(rela_s_O);
                }
                /*else if (DIEs.size() >= 2)  // die pads to die pads
                {
                    //cout << "kind of dice number: " << die_number.size() << endl;
                    if (die_number.size() == 2) // Can not handle three or more dice's relationship
                    {
                        vector<Cartesian> cars1, cars2;
                        vector<size_t> pads1_index, pads2_index;
                        pair<size_t, vector<size_t>> pads1, pads2;
                        cars1.clear();
                        cars2.clear();
                        pads1_index.clear();
                        pads2_index.clear();
                        pads1.first = 0;
                        pads2.first = 0;
                        pads1.second.clear();
                        pads2.second.clear();

                        rela_s_I = InnerRelationship();
                        rela_s_I.relation_name = netlist_name;  // set relation_name
                        for (size_t i = 0; i < DIEs.size(); i++)
                        {
                            size_t die_index = -1UL, pad_index = -1UL;
                            stringstream string_cache(DIEs[i].substr(3, DIEs[i].find(".") - 3));
                            string_cache >> die_index;
                            pad_index = chip->get_Die_Pad_Index(die_index - 1, DIEs[i]);

                            if (pads1.first == 0 || pads1.first == die_index)
                            {
                                pads1.first = die_index;
                                cars1.push_back(chip->get_Die_Pad_Position(die_index - 1, pad_index));
                                pads1_index.push_back(pad_index);
                            }
                            else if (pads2.first == 0 || pads2.first == die_index)
                            {
                                pads2.first = die_index;
                                cars2.push_back(chip->get_Die_Pad_Position(die_index - 1, pad_index));
                                pads2_index.push_back(pad_index);
                            }
                            else
                            {
                                cout << "error: more than two kind of dice";
                                exit(0);
                            }
                        }

                        if (pads1_index.size() != 0 && pads2_index.size() != 0) // pads1 and pads2 must have element
                        {
                            rela_s_I.dice_pads1_index.first = pads1.first;  // set die number
                            rela_s_I.dice_pads1_index.second = pads1_index; // set pads number
                            rela_s_I.pad1_car = CG(cars1);
                            rela_s_I.dice_pads2_index.first = pads2.first;  // set die number
                            rela_s_I.dice_pads2_index.second = pads2_index; // set pads number
                            rela_s_I.pad2_car = CG(cars2);

                            // insert relationship into netlist
                            inner_netlist.push_back(rela_s_I);
                        }
                    }
                }*/
                isNet = false;
            }
        }
    }

    // Cartesian convert to Polar (Outer Relationship)
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        outer_netlist[i].pad_pol = convert_cart_to_polar(outer_netlist[i].pad_car);
    }

    // Cartesian convert to Polar (Inner Relationship)
    for (size_t i = 0; i < inner_netlist.size(); i++) {
        inner_netlist[i].pad1_pol = convert_cart_to_polar(inner_netlist[i].pad1_car);
        inner_netlist[i].pad2_pol = convert_cart_to_polar(inner_netlist[i].pad2_car);
    }

    // final assign
    chip->set_All_O_Netlist(outer_netlist);
    chip->set_All_I_Netlist(inner_netlist);

    fin.close();
    cout << "parse netlist done\n";
    return;
}

/* Parse drc, pin, netlist file */
int parser(int argc, char** argv)
{
    string str;
	string index_suffle;
    ifstream PIN_fin;		// PIN
	ifstream netlist_fin;	// netlist
    ifstream drc_fin;   // drc
	//ofstream fout;			// relationship

    str.clear();
    index_suffle.clear();
    PIN_fin.clear();
    netlist_fin.clear();
    drc_fin.clear();
    //fout.clear();

    // parse drc
    drc_fin.open(argv[1]);
    if (!netlist_fin) {
        cout << "file \"" << argv[1] << "\" open error.";
        return -1;
    }
    parse_Drc(drc_fin);

    // parse pins
    PIN_fin.open(argv[2]);
	if (!PIN_fin.is_open()) {
		cout << "file \"" << argv[2] << "\" open error.";
		return -1;
	}
    parse_PIN(PIN_fin);
    
    // parse netlist
	netlist_fin.open(argv[3]);
	if (!netlist_fin) {
		cout << "file \"" << argv[3] << "\" open error.";
		return -1;
	}
    parse_Netlist(netlist_fin);
    
    chip->basic_infomation();
    chip->balls_Content();
    chip->dice_Content();
    chip->netlist_Content();
    
    return 0;
}

/*
// move back original point
        for (size_t j = 0; j < dice_pads_name[i].size(); j++)
        {
            double temp_rotation = dice_pads_rotation[i][j];
            while(temp_rotation >= M_PI_2) temp_rotation -= M_PI_2;
            while (temp_rotation <= -M_PI_2) temp_rotation += M_PI_2;

            dice_pads_cartesian[i][j] = Cartesian(
                    (
                        (dice_pads_cartesian[i][j].x - dice_center[i].x) * cos(-temp_rotation) 
                        - (dice_pads_cartesian[i][j].y - dice_center[i].y) * sin(-temp_rotation)
                    ) + dice_center[i].x
                , (
                        (dice_pads_cartesian[i][j].x - dice_center[i].x) * sin(-temp_rotation) 
                        + (dice_pads_cartesian[i][j].y - dice_center[i].y) * cos(-temp_rotation)
                    ) + dice_center[i].y
            );
        }
*/