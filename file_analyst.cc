#include"file_analyst.h"

using namespace std;

/* parse basic information of chip */
void parse_Drc(std::ifstream& fin)
{
	string str, info_name;
    stringstream str_stream;

    str.clear();
    str_stream.clear();

    while (getline(fin, str))
	{
        str_stream.clear();
        info_name.clear();

        str_stream << str;
        str_stream >> info_name;

        if (info_name.find("packagesizeX") != string::npos) {
            str_stream >> chip->drc.packageSize.x;
        }
        else if (info_name.find("packagesizeY") != string::npos) {
            str_stream >> chip->drc.packageSize.y;
        }
        else if (info_name.find("ballpitch") != string::npos) {
            str_stream >> chip->drc.ballPitch;
        }
        else if (info_name.find("balldiameter") != string::npos) {
            str_stream >> chip->drc.ballDiameter;
        }
        else if (info_name.find("balldimensionX") != string::npos) {
            str_stream >> chip->drc.ballDimensionX;
        }
        else if (info_name.find("balldimensionY") != string::npos) {
            str_stream >>  chip->drc.ballDimensionY;
        }
        else if (info_name.find("viadiameter") != string::npos) {
            str_stream >> chip->drc.viaDiameter;
        }
        else if (info_name.find("wirewidth") != string::npos) {
            str_stream >> chip->drc.wireWidth;
        }
        else if (info_name.find("spacing") != string::npos) {
            str_stream >> chip->drc.spacing;
        }
        else if (info_name.find("numlayer") != string::npos) {
            str_stream >> chip->drc.numLayer;
        }
        else {
            cout << "skip " << info_name << endl;
        }
    }

	fin.close();
    cout << "parse drc DONE\n";
    return;
}

/* parse all pin of chip */
void parse_PIN(ifstream& fin)
{
	bool isBall = false;
	bool isDie = false;
	size_t die_number = 0, dice_amount = 0;
	string str, temp_str;                           // variable of each line and some  word of line
    item temp_item;
	string pin_number;                          // variable of pin number
    Cartesian die_center(0.0, 0.0);     // current die center
    stringstream str_stream;                // string stream of each line of file
    
    str.clear();
    temp_str.clear();
    pin_number.clear();
    str_stream.clear();
    str_stream.str("");

    while (getline(fin, str))
	{
        str_stream.clear();
        str_stream.str(str);
        str_stream >> temp_str; // get first word of line
        //cout << "temp_str: " << temp_str << endl;

        // first: get pin number & pin_name
		if (temp_str.find("pin") != string::npos)
        {
            str_stream >> temp_str; // get "number:" or "name:"
            if (temp_str.find("number:") != string::npos)
            {
                // re-initialize
                temp_item = item();
                die_center = Cartesian(0.0, 0.0);
                str_stream >> pin_number; // get pin number
                if (pin_number.find("BGA") != string::npos) {
                    isBall = true;
                }
                else if (pin_number.find("DIE") != string::npos) {
                    isDie = true;
                    die_number = atoi(pin_number.substr(3, pin_number.find(".") - 3).c_str());
                }
                else {
                    cout << "Warning: pin " << pin_number << " be ignored\n";
                    isBall = false;
                    isDie = false;
                }
            }
            else if (temp_str.find("name:") != string::npos) {
                str_stream >> temp_item.name; // get pin name
            }
        }

		// second: get location of die pad or BGA ball
		if (temp_str.find("location-xy:") != string::npos && (isDie || isBall)) // if found location of pad or ball
		{
			str_stream >> temp_str;  // x coordinate(e.g. "(0.00")
			temp_str.erase(0, 1);    // clean left quote
			temp_item.xy.x = stod(temp_str);
			str_stream >> temp_str;  // y coordinate(e.g. "0.00)")
			temp_str.erase(temp_str.size() - 1, 1);//clean right quote
			temp_item.xy.y = stod(temp_str);
        }

        // third: get rotation of pad
        if (temp_str.find("padstack") != string::npos && (isDie || isBall))
        {
            str_stream >> temp_str; // get "rotation:"
            if (temp_str.find("rotation:") != string::npos)
            {
                str_stream >> temp_item.rotation; // store angle

                // set data
                if (isBall && !isDie)   // ball location
                {
                    chip->insert_Ball(pin_number, Ball(temp_item));
                    isBall = false;
                }
                else if (isDie && !isBall)  // die pad location
                {
                    chip->insert_Die_Pad(die_number - 1, pin_number, temp_item);
                    if (die_number > dice_amount)
                        dice_amount = die_number;
                }
                else {
                    cout << "error: At " << pin_number << ", isBall and isDie at the same times.\n";
                    exit(0);
                }
            }
        }

        // fourth: get center of die
        if (temp_str.find("Symbol:") != string::npos && isDie) // if found center of die
        {
            str_stream >> temp_str;
            str_stream >> temp_str; // get "at"
            assert(temp_str.find("at") != string::npos);

            str_stream >> die_center.x;  // get x coordinate of center
            str_stream >> temp_str; // get ","
            assert(temp_str.find(",") != string::npos);
            str_stream >> die_center.y;  // get x coordinate of center

            chip->set_Die_Center(die_number - 1, die_center);
            isDie = false;
        }
	}

    // other processing
    if (dice_amount == 1) chip->set_Die_Center(0, Cartesian(0.0, 0.0)); // set  center as (0.0, 0.0) if only one die
    if (chip->get_Dice_Amount() == 2) {
        chip->shift_Rotate_Die_Pads(0, Cartesian(0.0, 0.0), -15.0);
        chip->shift_Rotate_Die_Pads(1, Cartesian(0.0, 0.0), -180.0);
    }
    // calculate all dice width and height
    chip->calculate_All_Dice_WH();

    fin.close();
    cout << "parse pin DONE\n";
    return;
}

/* parse netlist of chip */
void parse_Netlist(ifstream& fin)
{
    string str;   // str_temp: get multi line netlist and combine into str
    vector<string> BGAs;
    vector<string> DIEs;
    vector<relationship> internal_netlist;
    vector<relationship> external_netlist;

    str.clear();
    BGAs.clear();
    DIEs.clear();
    internal_netlist.clear();
    external_netlist.clear();

    // Try to find "$NET"
    do getline(fin, str); while (str.find("$NETS") == string::npos && !fin.eof());

    if (!fin.eof()) // found $NET, parse single die netlist
    {
        string str_temp;
        str.clear();
        str_temp.clear();

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
                string net_name;
                stringstream str_stream;

                net_name.clear();
                str_stream.clear();
                str_stream.str("");

                str += str_temp;
                str_stream << str;
                str_stream >> net_name;  // get external_netlist name

                if (ignore_Power_Ground(net_name))
                {}
                else if (str.find("BGA") != string::npos && str.find("DIE") != string::npos)   // filter only one kind of pin
                {
                    relationship rela_s = relationship();
                    string sub_str;
                    sub_str.clear();
                    
                    str_stream >> sub_str;  // get ";"
                    if (sub_str.find(";") != string::npos)
                    {
                        BGAs.clear();
                        DIEs.clear();

                        // Categories balls and dice pads
                        while (str_stream >> sub_str)
                        {
                            if (sub_str.find("BGA") != string::npos)
                                BGAs.push_back(sub_str);
                            else if (sub_str.find("DIE") != string::npos)
                                DIEs.push_back(sub_str);
                            else
                                cout << "Warning: netlist has no BGA or DIE\n";
                        }
                        
                        // set data of balls
                        rela_s.pins1_number = 0;    // BGA balls
                        rela_s.pins1 = BGAs;
                        // set data of die pads
                        rela_s.pins2_number = 1;    // only one die
                        rela_s.pins2 = DIEs;
                        // set external net
                        chip->insert_External_Net(net_name, rela_s);
                    }
                    else
                    {
                        cout << "error: string has no \";\"\n";
                        exit(0);
                    }
                }
                else
                {
                    cout << "Warning: string \"" << str.substr(0, str.size() - 1) << "\" has no BGA and DIE\n";
                }
                str.clear();
            }
        }
    }
    else    // parse multi_netlist
    {
        cout << "There is no \"$NET\" in netlist file\n";
        cout << "Use Net Name to parse netlist file\n";

        bool isNet = false;
        string temp_str, net_name;
        stringstream str_stream;    // string stream of each line of file
        vector<int> dice_number;
        relationship rela_s = relationship();     // net
        
        fin.clear();
        fin.seekg(ios::beg);   // Seek to begin of fin
        BGAs.clear();
        DIEs.clear();
        temp_str.clear();
        net_name.clear();
        str_stream.clear();
        str_stream.str("");
        dice_number.clear();

        // start to parse multi_netlist
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
                    str_stream >> net_name;  // get netlist name
                    
                    if (!ignore_Power_Ground(net_name))  // ignore power and ground if ignore_P_G is true
                    {
                        while(getline(fin, str) && str.find("---") == string::npos);    // skip the middle data until "---" appears
                        isNet = true;
                    }
                }
            }

            // second: get all pins and classify. final insert
		    if (isNet)
		    {
                BGAs.clear();
                DIEs.clear();
                dice_number.clear();
                rela_s = relationship();

                // get all pins of net
                while(getline(fin,str) && ( str.find("BGA")  != string::npos || str.find("DIE")  != string::npos ))
                {
                    str_stream.clear();
                    str_stream.str(str);
                    str_stream >> temp_str;  // get pin name

                    if (temp_str.find("BGA") != string::npos)
                        BGAs.push_back(temp_str);
                    else if (temp_str.find("DIE") != string::npos) {
                        DIEs.push_back(temp_str);
                        int die_number = atoi(temp_str.substr(3, temp_str.find('.') - 3).c_str());
                        
                        // Check whether die_number has appeared
                        bool not_appeared = true;
                        for (size_t i = 0; i < dice_number.size() && not_appeared; i++)  if (dice_number[i] == die_number) not_appeared = false;
                        // If pin_number has not appeared, add pin_number into die_number
                        if (not_appeared) dice_number.push_back(die_number);
                    }
                    else {
                        cout << "warring: pin \"" << temp_str << "\" be ignored\n";
                    }
                }

                getline(fin,str);   // skip space line

                // There is some problem maybe!!!
                if (str.find("No connections remaining") == string::npos)
                    cout << "Warning: netlist ‘" << net_name << "‘  has no BGA or DIE, and may have some pin miss catch.\n";
                else {
                    if (dice_number.size() == 0) {
                        cout << "Warning: Ignoring \"" << net_name << "\". ";
                        if (BGAs.size() != 0)
                            cout << "There is only BGA balls.\n";
                        else
                            cout << "There is no any BGA ball and die pad.\n";
                        
                    }
                    if (BGAs.size() != 0 && dice_number.size() == 1)   // BGA balls to die pads
                    {
                        rela_s.pins1_number = 0;  // set pin1_number to 0
                        rela_s.pins1 = BGAs;
                        rela_s.pins2_number = dice_number[0]; // set pin2_number to die_number
                        rela_s.pins2 = DIEs;
                        // insert relationship into external netlist
                        chip->insert_External_Net(net_name, rela_s);
                    }
                    else if (BGAs.size() != 0 && dice_number.size() > 1) {  // BGA balls connect to die pads of many die
                        cout << "Ignoring \"" << net_name << "\". Current program can not handle.\n";
                    }
                    else if (dice_number.size() == 2)  // die pads of one die connect to another die pads of one die
                    {
                        int pads1_die_number = dice_number[0], pads2_die_number = dice_number[1];
                        vector<string> pads1, pads2;
                        pads1.clear();
                        pads2.clear();

                        for (size_t i = 0; i < DIEs.size(); i++) // classify pads1 and pads2
                        {
                            int die_number = atoi(DIEs[i].substr(3, DIEs[i].find(".") - 3).c_str());
                            if (pads1_die_number == die_number) {
                                pads1.push_back(DIEs[i]);
                            }
                            else if (pads2_die_number == die_number) {
                                pads2.push_back(DIEs[i]);
                            }
                            else {
                                cout << "Error: DIEs has more than two kind of dice";
                                exit(0);
                            }
                        }
                        rela_s.pins1_number = pads1_die_number;
                        rela_s.pins1 = pads1;
                        rela_s.pins2_number = pads2_die_number;
                        rela_s.pins2 = pads2;
                        // insert relationship into internal netlist
                        chip->insert_Internal_Net(net_name, rela_s);
                    }
                    else if (dice_number.size() > 2) {  // die pads of many die connect each other
                        cout << "Ignoring \"" << net_name << "\". Current program can not handle.\n";
                    }
                    isNet = false;
                }
            }
        }
    }

    fin.close();
    cout << "parse netlist DONE\n";
    return;
}

/* Parse drc, pin, netlist file */
int parser(int argc, char** argv)
{
    string str;
	string index_suffle;
    ifstream PIN_fin;       // PIN
	ifstream netlist_fin;   // netlist
    ifstream drc_fin;       // drc
	//ofstream fout;          // relationship

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
    
    cout << "parse all files DONE\n";
    return 0;
}