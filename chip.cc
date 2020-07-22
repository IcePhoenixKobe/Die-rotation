#include"chip.h"

using namespace std;

static const double inner_min_distance = 5760000;
static const double expand_dietance = 1500;

/* 
    Ball class defination
*/

/* 
    Die class defination
*/
// Get all pin number
vector<string> Die::get_Pads_Number() const
{
    vector<string> return_vector;
    return_vector.clear();
    for (map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); map_it++)
        return_vector.push_back(map_it->first);
    return return_vector;
}

// Get all pin name
vector<string> Die::get_Pads_Name() const
{
    vector<string> return_vector;
    return_vector.clear();
    for (map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); map_it++)
        return_vector.push_back(map_it->second.name);
    return return_vector;
}

// Get all pin location-xy
map<string, Cartesian> Die::get_Pads_Location() const
{
   map<string, Cartesian> return_map;
    return_map.clear();
    for (map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); map_it++)
        return_map[map_it->first] = map_it->second.xy;
    return return_map;
}

// Get all pin rotation
map<string, double> Die::get_Pads_Rotation() const
{
    map<string, double> return_map;
    return_map.clear();
    for (map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); map_it++)
        return_map[map_it->first] = map_it->second.rotation;
    return return_map;
}

// Get all pin radian of rotation
map<string, double> Die::get_Pads_Radian()
{
    map<string, double> return_map;
    return_map.clear();
    for (map<string, item>::iterator map_it = pads.begin(); map_it != pads.end(); map_it++)
        return_map[map_it->first] = map_it->second.getRadian();
    return return_map;
}

// calculate width and height with all pads
void Die::calculate_WH()
{
    double maxX = numeric_limits<double>::min(), minX = numeric_limits<double>::max(), 
                    maxY = numeric_limits<double>::min(), minY = numeric_limits<double>::max();
    for (map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); map_it++) {
        item pad_location = map_it->second;
        if(maxX < pad_location.xy.x) maxX = pad_location.xy.x;  // update maxX
        if(minX > pad_location.xy.x) minX = pad_location.xy.x;  // update minX
        if(maxY < pad_location.xy.y) maxY = pad_location.xy.y;  // update maxY
        if(minY > pad_location.xy.y) minY = pad_location.xy.y;  // update minY
    }
    width = maxX - minX + 80;
    height = maxY - minY + 80;
    return;
}

// shift to new_center and rotate rotation degree (rotation is angle)
void Die::shift_Rotate(Cartesian new_center, double rotation)
{
    for (map<string, item>::iterator map_it = pads.begin(); map_it != pads.end(); map_it++)
    {
        map_it->second.xy = shift_rotation(map_it->second.xy, center, new_center, (rotation * M_PI / 180.0));
        map_it->second.rotation += rotation;
    }
    center = new_center;
    return;
}

/* 
    Chip class defination
*/
// Constructor
Chip::Chip()
{
    balls.clear();
    dice.clear();
    internal_netlist.clear();
    external_netlist.clear();
}

// Output basic data
void Chip::basic_infomation()
{
    cout << "\n----------Basic Information----------\n";
    cout << fixed << setprecision(3)
              << setw(15) << "PackageSizeX" << setw(10) << drc.packageSize.x << endl
              << setw(15) << "PackageSizeY" << setw(10) << drc.packageSize.y << endl
              << setw(15) << "BallPitch" << setw(10) << drc.ballPitch << endl
              << setw(15) << "BallDiameter" << setw(10) << drc.ballDiameter << endl
              << setw(15) << "BallDimensionX" << setw(10) << drc.ballDimensionX << endl
              << setw(15) << "BallDimensionY" << setw(10) << drc.ballDimensionY << endl
              << setw(15) << "ViaDiameter" << setw(10) << drc.viaDiameter << endl
              << setw(15) << "WireWidth" << setw(10) << drc.wireWidth << endl
              << setw(15) << "Spacing" << setw(10) << drc.spacing << endl
              << setw(15) << "NumLayer" << setw(10) << drc.numLayer << endl;
    cout << "----------Basic Information----------\n";
    return;
}

// Output data of BGA balls
void Chip::balls_Content()
{
    cout << "\n----------Ball Content----------\n"
     << "Amount: " << balls.size() << endl;
    for (map<string, Ball>::const_iterator ball_it = balls.begin(); ball_it != balls.end(); ball_it++)
        cout << fixed << setprecision(3) 
            << setw(8) << ball_it->first 
            << " (x,y): (" 
            << setw(10) << ball_it->second.get_Location().x << ", "
            << setw(10) << ball_it->second.get_Location().y << ")\n";
    cout << "----------Ball Content----------\n";
    return;
}

// Output data of dice
void Chip::dice_Content()
{
    cout << "\n----------Dice Content----------\n"
     << "Amount: " << dice.size() << endl;
    for (size_t die_index = 0; die_index < dice.size(); die_index++)
    {
        cout << "\nDIE " << die_index + 1 << ":\n"
            << "Center: (" << dice[die_index].get_Center().x << ", " << dice[die_index].get_Center().y << ")\n"
            << "Die Size X: " << dice[die_index].get_Width() << endl
            << "Die Size Y: " << dice[die_index].get_Height() << endl
            << "Pad Amount: " << dice[die_index].get_Pads_Amount() << endl;
        
        map<string, item> pads = dice[die_index].get_Pads();
        for (map<string, item>::const_iterator map_it = pads.begin(); map_it != pads.end(); map_it++) {
            Polar pad_pol = convert_cart_to_polar(map_it->second.xy, dice[die_index].get_Center());
            cout << fixed << setprecision(3) << setw(9) << map_it->first
                            << " (x,y): (" << setw(10) << map_it->second.xy.x << ", " << setw(10)<< map_it->second.xy.y << ") |"
                            << " (r,θ): (" << setw(9)<< pad_pol.radius << ", " << setw(7) << pad_pol.angle << ") |"
                            << " rotation: " << setw(7) << map_it->second.rotation << endl;
        }
    }
    cout << "----------Dice Content----------\n";
    return;
}

// Output data of netlist
void Chip::netlist_Content()
{
    cout << "\n----------Netlist Content----------\n"
     << "Outer Netlist: \n"
     << "Amount: " << external_netlist.size() << endl;
    int count = 1;
    for (map<string, relationship>::const_iterator map_it = external_netlist.begin(); map_it != external_netlist.end(); map_it++, count++)
    {
        cout << setw(4) << count << " " << setw(10) << map_it->first << ": ";
        for (size_t index = 0; index < map_it->second.pins1.size(); index++)
            cout << "\n\t" << map_it->second.pins1[index];
        cout << "\n\t↓\n";
        for (size_t index = 0; index < map_it->second.pins2.size(); index++)
            cout << "\t" << map_it->second.pins2[index] << endl;
    }
    cout << "\nInner Netlist: \n"
     << "Amount: " << internal_netlist.size() << endl;
    count = 1;
    for (map<string, relationship>::const_iterator map_it = internal_netlist.begin(); map_it != internal_netlist.end(); map_it++, count++)
    {
        cout << setw(4) << count << " " << setw(10) << map_it->first << ": ";
        for (size_t index = 0; index < map_it->second.pins1.size(); index++)
            cout << "\n\t" << map_it->second.pins1[index];
        cout << "\n\t↓\n";
        for (size_t index = 0; index < map_it->second.pins2.size(); index++)
            cout << "\t" << map_it->second.pins2[index] << endl;
    }
    cout << "----------Netlist Content----------\n";
    return;
}

// get all balls item
std::map<std::string, item> Chip::get_Balls_Item() const
{
    map<string, item> return_map;
    return_map.clear();
    for (map<string, Ball>::const_iterator map_it = balls.begin(); map_it != balls.end(); map_it++) {
        return_map[map_it->first] = map_it->second.get_Item();
    }
    return return_map;
}

// get all number of the ball
vector<string> Chip::get_Balls_Number() const
{
    vector<string> return_vector;
    return_vector.clear();
    for (map<string, Ball>::const_iterator ball_it = balls.begin(); ball_it != balls.end(); ball_it++)
        return_vector.push_back(ball_it->first);
    return return_vector;
}

// get all name of the ball
map<string, string> Chip::get_Balls_Name() const
{
    map<string, string> return_map;
    return_map.clear();
    for (map<string, Ball>::const_iterator ball_it = balls.begin(); ball_it != balls.end(); ball_it++)
        return_map[ball_it->first] = ball_it->second.get_Name();
    return return_map;
}

// get all location of the ball
map<string, Cartesian> Chip::get_Balls_Location() const
{
    map<string, Cartesian> return_map;
    return_map.clear();
    for (map<string, Ball>::const_iterator ball_it = balls.begin(); ball_it != balls.end(); ball_it++)
        return_map[ball_it->first] = ball_it->second.get_Location();
    return return_map;
}

// get location of some balls
vector<Cartesian> Chip::get_Some_Balls_Location(vector<string> balls_number) const
{
    vector<Cartesian> return_vector;
    return_vector.clear();
    for (size_t i = 0; i < balls_number.size(); i++)
        return_vector.push_back(get_Ball_Location(balls_number[i]));
    return return_vector;
}

// get location of the pad
Cartesian Chip::get_Die_Pad_Location(std::string par_number) const
{
    for (size_t die_index = 0; die_index < dice.size(); die_index++)
    {
        map<string, Cartesian> pads = dice[die_index].get_Pads_Location();
        if (pads.find(par_number) != pads.end()) return pads[par_number];
    }
    cout << "Warning: can not find " << par_number << " in all of the dice pads\n";
    return Cartesian(0.0, 0.0);
}

// get rotation of the pad
double Chip::get_Die_Pad_Rotation(std::string par_number) const
{
    for (size_t die_index = 0; die_index < dice.size(); die_index++)
    {
        map<string, double> pads = dice[die_index].get_Pads_Rotation();
        if (pads.find(par_number) != pads.end()) return pads[par_number];
    }
    cout << "Warning: can not find " << par_number << " in all of the dice pads\n";
    return 0.0;
}

// get radian of the pad
double Chip::get_Die_Pad_Radian(std::string par_number)
{
    for (size_t die_index = 0; die_index < dice.size(); die_index++)
    {
        map<string, double> pads = dice[die_index].get_Pads_Rotation();
        if (pads.find(par_number) != pads.end()) return pads[par_number];
    }
    cout << "Warning: can not find " << par_number << " in all of the dice pads\n";
    return 0.0;
}

// get location of some pads
vector<Cartesian> Chip::get_Pads_Location(vector<string> pads_number) const
{
    vector<Cartesian> return_vector;
    return_vector.clear();
    for (size_t i = 0; i < pads_number.size(); i++)
        return_vector.push_back(get_Die_Pad_Location(pads_number[i]));
    return return_vector;
}

// Output .lp file for Gurobi
void Chip::output_LP_File(ofstream& fout)
{
    cout << "Notice: can not use in current version!!!\n";
    // Minimize
    /*if (min_output)
    {
        fout << "Minimize\n";
        for (size_t i = 0; i < external_netlist.size(); i++) {
            if (i == 0) fout << "  a0x + a0y";
            else {
                fout << " + a" << i << "x + a" << i << "y";
            }
        }
        fout << "\n";
    }

   // Subject To
    fout << "Subject To\n";
    for (size_t i = 0; i < external_netlist.size(); i++) {
        double radius = external_netlist[i].pad_pol.radius;
        if (i == 0) fout << "  goal: " << radius << " s0p - " << radius << " c0p";
        else {
            fout << " + " << radius << " s" << i << "p - " << radius << " c" << i << "p";
        }
    }
    fout << " = 0\n";

    // Bounds
    fout << "Bounds\n";
    fout << "  0 <= alpha <= " << 2 * M_PI << "\n";
    for (size_t i = 0; i < external_netlist.size(); i++) {
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
    vector<Cartesian> ball_pos;
    ball_pos = balls.get_All_Location();
    fout << "General Constraint\n";
    for (size_t i = 0; i < external_netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 alpha + " << external_netlist[i].pad_pol.angle << " )\n" 
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  sin" << i << ": s" << i << "p = POLY ( "  << ( external_netlist[i].ball_car.x - dice[0].get_Center().x ) << " n" << i << "s )\n"
                 << "  cos" << i << ": c" << i << "p = POLY ( " << ( external_netlist[i].ball_car.y - dice[0].get_Center().y ) << " n" << i << "c )\n";
        if (min_output)
        {
            fout << " \\minimize equation\n";
            fout << "  miner" << i << "x: m" << i << "x = POLY ( -" << external_netlist[i].pad_pol.radius 
                 << " n" << i << "c + " << external_netlist[i].ball_car.x - dice[0].get_Center().x << " )\n"
                 << "  miner" << i << "y: m" << i << "y = POLY ( -" << external_netlist[i].pad_pol.radius
                 << " n" << i << "s + " << external_netlist[i].ball_car.y - dice[0].get_Center().y << " )\n";
            fout << " \\absolute value\n"
                     << "  abs" << i << "x: a" << i << "x = ABS ( m" << i << "x )\n" 
                     << "  abs" << i << "y: a" << i << "y = ABS ( m" << i << "y )\n";
        }
    }*/

    /*// Minimize
    if (min_output)
    {
        fout << "Minimize\n";
        for (size_t i = 0; i < external_netlist.size(); i++) {
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
    vector<vector<Polar>> dice_pads;
    dice_pads.resize(dice.size());
    for (size_t i = 0; i < dice.size() && i < dice_pads.size(); i++)
        dice_pads[i] = dice[i].get_Pads_Pol_Location();
    fout << "Bounds\n";
    for (size_t i = 0; i < external_netlist.size(); i++) {
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
    vector<Cartesian> ball_pos;
    ball_pos = balls.get_All_Location();
    fout << "General Constraint\n";
    for (size_t i = 0; i < external_netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 alpha + " << external_netlist[i].pad_pol.angle << " )\n"
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  net" << i << "x: n" << i << "x = POLY ( -" << external_netlist[i].pad_pol.radius << " n" << i << "c + "
                 << external_netlist[i].ball_car.x - dice[external_netlist[i].dice_pads_index[0].first - 1].get_Center().x << " )\n"
                 << "  net" << i << "y: n" << i << "y = POLY ( -" << external_netlist[i].pad_pol.radius << " n" << i << "s + "
                 << external_netlist[i].ball_car.y - dice[external_netlist[i].dice_pads_index[0].first - 1].get_Center().y << " )\n"
                 << "  abs" << i << "x: a" << i << "x = ABS ( n" << i << "x )\n"
                 << "  abs" << i << "x: a" << i << "y = ABS ( n" << i << "y )\n";
    }*/

    // End
    /*fout << "End";*/
    return;
}

// Output .m file for MATLABf
void Chip::output_M_File(ofstream& fout, char* file_name)
{
    // expand DIE1
    /*Die die1 = get_Die(0);
    map<string, item> die1_pads_change = die1.get_Pads();
    for (map<string, item>::iterator map_it = die1_pads_change.begin(); map_it != die1_pads_change.end(); map_it++) {
        Polar pad_pol = convert_cart_to_polar(map_it->second.xy, dice[0].get_Center());
        map_it->second.xy.x = (pad_pol.radius + expand_dietance) * cos(pad_pol.get_Radian());
        map_it->second.xy.y = (pad_pol.radius + expand_dietance) * sin(pad_pol.get_Radian());
    }
    die1.set_Pads(die1_pads_change);
    dice[0] = die1;*/

    //dice_Content();

    /* Get Filename */
    string fun_name(file_name);
    fun_name = fun_name.substr(fun_name.rfind('/') + 1);    // get file name(contain .m)
    size_t found = fun_name.rfind('.');    // get dot position
    fun_name.erase(fun_name.begin() + found, fun_name.end());   // get file name(no .m)
    fout << "function f = " << fun_name << "(x)\n\tf = ";
    // output outer relationship
    for (map<string, relationship>::const_iterator map_it = external_netlist.begin(); map_it != external_netlist.end(); map_it++)
    {
        assert(map_it->second.pins1_number == 0);    // assert pins1 are ball(s).

        // get data that will be output
        size_t die_number = map_it->second.pins2_number;
        Cartesian ball_cart = CG(get_Some_Balls_Location(map_it->second.pins1));
        Polar pads_pol = convert_cart_to_polar(CG(get_Pads_Location(map_it->second.pins2)), get_Die_Center(die_number));

        if (map_it != external_netlist.begin()) fout << " + ";
        fout << "(x(" << (die_number - 1) * 3 + 1 << ")+"
                 << pads_pol.radius << "*cos(" << pads_pol.get_Radian() << "+x(" << (die_number - 1) * 3 + 3 << "))-"
                 << ball_cart.x << ")^2 + "
                 << "(x(" << (die_number - 1) * 3 + 2 << ")+"
                 << pads_pol.radius << "*sin(" << pads_pol.get_Radian() << "+x(" << (die_number - 1) * 3 + 3 << "))-"
                 << ball_cart.y << ")^2";
    }
    // output inner relationship
    for (map<string, relationship>::const_iterator map_it = internal_netlist.begin(); map_it != internal_netlist.end(); map_it++)
    {
        // get data that will be output
        size_t die_number1 = map_it->second.pins1_number, die_number2 = map_it->second.pins2_number;
        Cartesian pads1_CG = CG(get_Pads_Location(map_it->second.pins1)),
                             pads2_CG = CG(get_Pads_Location(map_it->second.pins2));
        Polar pads1_pol = convert_cart_to_polar(pads1_CG, get_Die_Center(die_number1)), 
                    pads2_pol = convert_cart_to_polar(pads2_CG, get_Die_Center(die_number2));

        fout << " + (x(" << (die_number1 - 1) * 3 + 1 << ")+"
                 << pads1_pol.radius << "*cos(" << pads1_pol.get_Radian() << "+x(" << (die_number1 - 1) * 3 + 3 << "))"
                 << "-x(" << (die_number2 - 1) * 3 + 1 << ")-"
                 << pads2_pol.radius << "*cos(" << pads2_pol.get_Radian() << "+x(" << (die_number2 - 1) * 3 + 3 << ")))^2"
                 << " + (x(" << (die_number1 - 1) * 3 + 2 << ")+"
                 << pads1_pol.radius << "*sin(" << pads1_pol.get_Radian() << "+x(" << (die_number1 - 1) * 3 + 3 << "))"
                 << "-x(" << (die_number2 - 1) * 3 + 2 << ")-"
                 << pads2_pol.radius << "*sin(" << pads2_pol.get_Radian() << "+x(" << (die_number2 - 1) * 3 + 3 << ")))^2";
    }
    fout << ";";
    return;
}