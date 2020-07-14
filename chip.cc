#include"chip.h"

using namespace std;

/* 
    Ball class defination
*/
// Default Constructor
Ball::Ball()
{
    amount = 0;
    name.clear();
    position.clear();
}

// Constructor
Ball::Ball(size_t ball_num)
{
    amount = ball_num;
    name.clear();
    position.clear();
    name.resize(amount);
    position.resize(amount);
    for (size_t i = 0; i < position.size(); i++) {
        name[i] = "unknown";
        position[i].x = 0.0;
        position[i].y = 0.0;
    }
}

// Use BGA ball name to get index of vector
size_t Ball::get_Ball_Index(string str) const
{
    for (size_t t = 0; t < name.size(); t++)
        if (name[t] == str)
            return t;
    return -1;
}

/* 
    Die class defination
*/
// Constructor with pad amount
Die::Die(size_t die_pad_amount) : center(Cartesian(0.0, 0.0))
{
    Clear_Pad();
    Resize_Pad(die_pad_amount);
}

// Constructor with pad amount and die center
Die::Die(size_t die_pad_amount, Cartesian xy) : center(xy)
{
    Clear_Pad();
    Resize_Pad(die_pad_amount);
}

// Clear the position and position
void Die::Clear_Pad()
{
    amount = 0;
    name.clear();
    rotation.clear();
    position.clear();
}

/* Resize the pads name and position, and set all element 
 * of name and position to "umknownPad" and (0.0, 0.0).
 * */
void Die::Resize_Pad(size_t size)
{
    amount = size;
    name.resize(size);
    position.resize(size);
    rotation.resize(size);
    for (size_t i = 0; i < size; i++) {
        name[i] = "unknownPad";
        position[i].first = Cartesian(0.0, 0.0);
        position[i].second = Polar(0.0, 0.0);
        rotation[i] = 0.0;
    }
}

// Use die pad name to get index of (name) vector
size_t Die::get_Pad_Index(string str) const
{
    for (size_t i = 0; i < name.size(); i++)
        if (name[i].find(str) != string::npos)
            return i;
    return -1UL;
}

// Clear all pad and set
void Die::set_Pads(vector<string> par_name, vector<Cartesian> par_position, vector<double> par_rotation)
{
    // Clear all pads
    Clear_Pad();

    assert(par_name.size() == par_position.size());
    assert(par_position.size() == par_rotation.size());

    // Set amount
    amount = par_name.size();
    
    // Set all pads(name  and position(Cartesian))
    name = par_name;
    for (vector<Cartesian>::iterator it = par_position.begin(); it != par_position.end(); it++) {
        position.push_back(make_pair(*it, Polar(0.0, 0.0)));
    }

    // Set all pads rotations
    rotation = par_rotation;

    // Set all pads polar coordinate
    for (size_t i = 0; i < position.size(); i++)
    {
        //cout << "pad " << i + 1 << ":";

        // calculate radius
        position[i].second = convert_cart_to_polar(position[i].first);

        //cout << "\tradius = " << position[i].second.radius;
        //cout << "\ttheta = " << position[i].senond.angle << endl;
    }
    return;
}

void Die::Original_Position()
{
    assert(name.size() == position.size());
    assert(position.size() == rotation.size());
    for (size_t i = 0; i < name.size(); i++)
    {
        double temp_rotation = rotation[i];
        while (temp_rotation >= M_PI_2) temp_rotation -= M_PI_2;
        while (temp_rotation <= M_PI_2) temp_rotation += M_PI_2;
        temp_rotation -= M_PI_2;
        position[i].first = Cartesian(
                    (
                        (position[i].first.x - center.x) * cos(-temp_rotation) 
                        - (position[i].first.y - center.y) * sin(-temp_rotation)
                    ) + center.x
                , (
                        (position[i].first.x - center.x) * sin(-temp_rotation) 
                        + (position[i].first.y - center.y) * cos(-temp_rotation)
                    ) + center.y
            );
        position[i].second = convert_cart_to_polar(position[i].first);
    }
    
    return;
}

/* 
    Chip class defination
*/
// Constructor
Chip::Chip()
{
    balls = Ball();
    dice.clear();
    inner_netlist.clear();
    outer_netlist.clear();
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
    cout << "\n----------Ball content----------\n"
     << "amount: " << balls.get_Amount() << endl;
    vector<string> name;
    name = balls.get_All_Name();
    vector<Cartesian> position;
    position = balls.get_All_Position();
    for (size_t i = 0; i < position.size(); i++)
        cout << fixed << setprecision(3) << setw(4) << i + 1 << " " << setw(8) << name[i]
         << " x: " << setw(10) << position[i].x << ", y: " << setw(10) << position[i].y << endl;
    cout << "----------Ball content----------\n";
    return;
}

// Output data of dice
void Chip::dice_Content()
{
    cout << "\n----------Dice content----------\n"
     << "amount: " << dice.size() << endl;
    for (size_t index = 0; index < dice.size(); index++)
    {
        cout << "\nDie " << index + 1 << ":"
         << "\ncenter: " << dice[index].get_Center().x << " " << dice[index].get_Center().y
         << "\npad amount: " << dice[index].get_Pad_Amount() << endl;
        vector<string> name;
        name = dice[index].get_Pads_Name();
        vector<pair<Cartesian, Polar>> position;
        position = dice[index].get_Pads_Position();
        vector<double> rotation;
        rotation = dice[index].get_Pads_Rotation();
        assert(name.size() == position.size());
        assert(position.size() == rotation.size());
        for (size_t i = 0; i < position.size(); i++) {
            cout << fixed << setprecision(3) << setw(4) << i + 1 << " " << setw(8) << name[i]
                            << " x: " << setw(10) << position[i].first.x 
                            << ", y: " << setw(10)<< position[i].first.y
                            << " | r: " << setw(9)<< position[i].second.radius 
                            << ", theta: " << setw(7) << position[i].second.angle * 180 / M_PI
                            << ", rotation: " << setw(7) << rotation[i] * 180 / M_PI << endl;
        }
    }
    cout << "----------Dice content----------\n";
    return;
}

// Output data of netlist
void Chip::netlist_Content()
{
    cout << "\n----------Netlist content----------\n"
     << "Outer Netlist: \n"
     << "amount: " << outer_netlist.size() << endl;
    for (size_t i = 0; i < outer_netlist.size(); i++)
    {
        cout << setw(4) << i + 1 << " " << setw(10) << outer_netlist[i].relation_name << ": ";
        for (size_t j = 0; j < outer_netlist[i].balls_index.size(); j++)
            cout << "\n\t" << balls.get_Name(outer_netlist[i].balls_index[j]);
        cout << "\n\t-->\n";
        for (size_t j = 0; j < outer_netlist[i].dice_pads_index.size(); j++)
            for (size_t k = 0; k < outer_netlist[i].dice_pads_index[j].size(); k++)
                cout << "\t" << dice[j].get_Pad_Name(outer_netlist[i].dice_pads_index[j][k]) << endl;
    }
    cout << "\nInner Netlist: \n"
     << "amount: " << inner_netlist.size() << endl;
    for (size_t i = 0; i < inner_netlist.size(); i++)
    {
        cout << setw(4) << i + 1 << " " << setw(10) << inner_netlist[i].relation_name << ": ";
        for (size_t j = 0; j < inner_netlist[i].dice_pads1_index.second.size(); j++)
            cout << "\n\t" << dice[inner_netlist[i].dice_pads1_index.first - 1].get_Pad_Name(inner_netlist[i].dice_pads1_index.second[j]);
        cout << "\n\t-->\n";
        for (size_t j = 0; j < inner_netlist[i].dice_pads2_index.second.size(); j++)
            cout << "\t" << dice[inner_netlist[i].dice_pads2_index.first - 1].get_Pad_Name(inner_netlist[i].dice_pads2_index.second[j]) << endl;
    }
    cout << "----------Netlist content----------\n";
    return;
}

size_t Chip::get_Die_Index(std::string str) const
{
    for (size_t i = 0; i < dice.size(); i++) {
        for (size_t j = 0; j < dice[i].get_Pad_Amount(); j++) {
            if (dice[i].get_Pad_Index(str) != -1UL) return i;
        }
    }
    return -1UL;
}

// Will be rewrite
size_t Chip::get_Netlist_Index(string str) const
{
    for (size_t t = 0; t <= outer_netlist.size(); t++)
        if (outer_netlist[t].relation_name == str)
            return t;
    return -1UL;
}

// Output .lp file for Gurobi
void Chip::output_LP_File(ofstream& fout)
{
    cout << "Notice: can not use multi_io!!!\n";
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
    vector<Cartesian> ball_pos;
    ball_pos = balls.get_All_Position();
    fout << "General Constraint\n";
    for (size_t i = 0; i < outer_netlist.size(); i++) {
        fout << "\\net" << i << "\n";
        fout << " \\basic equation\n";
        fout << "  net" << i << "degree: n" << i << "d = POLY ( 1 alpha + " << outer_netlist[i].pad_pol.angle << " )\n" 
                 << "  net" << i << "sin: n" << i << "s = SIN ( n" << i << "d )\n"
                 << "  net" << i << "cos: n" << i << "c = COS ( n" << i << "d )\n"
                 << "  sin" << i << ": s" << i << "p = POLY ( "  << ( outer_netlist[i].ball_car.x - dice[0].get_Center().x ) << " n" << i << "s )\n"
                 << "  cos" << i << ": c" << i << "p = POLY ( " << ( outer_netlist[i].ball_car.y - dice[0].get_Center().y ) << " n" << i << "c )\n";
        if (min_output)
        {
            fout << " \\minimize equation\n";
            fout << "  miner" << i << "x: m" << i << "x = POLY ( -" << outer_netlist[i].pad_pol.radius 
                 << " n" << i << "c + " << outer_netlist[i].ball_car.x - dice[0].get_Center().x << " )\n"
                 << "  miner" << i << "y: m" << i << "y = POLY ( -" << outer_netlist[i].pad_pol.radius
                 << " n" << i << "s + " << outer_netlist[i].ball_car.y - dice[0].get_Center().y << " )\n";
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
    vector<vector<Polar>> dice_pads;
    dice_pads.resize(dice.size());
    for (size_t i = 0; i < dice.size() && i < dice_pads.size(); i++)
        dice_pads[i] = dice[i].get_Pads_Pol_Position();
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
    vector<Cartesian> ball_pos;
    ball_pos = balls.get_All_Position();
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

// Output .m file for MATLABf
void Chip::output_M_File(ofstream& fout, char* file_name)
{
    /* Get Filename */
    string fun_name(file_name);
    fun_name = fun_name.substr(fun_name.rfind('/') + 1);    // get file name(contain .m)
    size_t found = fun_name.rfind('.');    // get dot position
    fun_name.erase(fun_name.begin() + found, fun_name.end());   // get file name(no .m)
    fout << "function f = " << fun_name << "(x)\n\tf = ";
    for (size_t i = 0; i < outer_netlist.size(); i++)   // output outer relationship
    {
        // get x index for MATLAB variable
        size_t main_die_number = 0, temp_amount = 0;
        for (size_t j = 0; j < outer_netlist[i].dice_pads_index.size(); j++) {
            if (outer_netlist[i].dice_pads_index[j].size() > temp_amount) {
                main_die_number = j + 1;
                temp_amount = outer_netlist[i].dice_pads_index[j].size();
            }
        }

        if (i != 0) fout << " + ";
        fout << "(" << outer_netlist[i].ball_car.x << "-x(" << main_die_number << ")-"
                 << outer_netlist[i].pad_pol.radius << "*cos(" << outer_netlist[i].pad_pol.angle << "+x(" << 2 * dice.size() + main_die_number << "))).^2 + "
                 << "(" << outer_netlist[i].ball_car.y << "-x(" << dice.size() + main_die_number << ")-"
                 << outer_netlist[i].pad_pol.radius << "*sin(" << outer_netlist[i].pad_pol.angle << "+x(" << 2 * dice.size() + main_die_number << "))).^2";
    }
    for (size_t i = 0; i < inner_netlist.size(); i++)   // output inner relationship
    {
        fout << " + (x("
                 << inner_netlist[i].dice_pads1_index.first << ")+" << inner_netlist[i].pad1_pol.radius << "*cos(" << inner_netlist[i].pad1_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads1_index.first << "))-x("
                 << inner_netlist[i].dice_pads2_index.first << ")-" << inner_netlist[i].pad2_pol.radius << "*cos(" << inner_netlist[i].pad2_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads2_index.first << "))).^2 + (x("
                 << dice.size() + inner_netlist[i].dice_pads1_index.first << ")+" << inner_netlist[i].pad1_pol.radius << "*sin(" << inner_netlist[i].pad1_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads1_index.first << "))-x("
                 << dice.size() + inner_netlist[i].dice_pads2_index.first << ")-" << inner_netlist[i].pad2_pol.radius << "*sin(" << inner_netlist[i].pad2_pol.angle << "+x(" << 2 * dice.size() + inner_netlist[i].dice_pads2_index.first << "))).^2";
    }
    fout << ";";
    return;
}

void Chip::Original_Dice_Pads()
{
    for (size_t i = 0; i < dice.size(); i++)
        dice[i].Original_Position();
    return;
}