#include"main.h"
#include"chip.h"

int LP_out = -1;
int M_out = -1;
bool GUI = false;
bool min_output = true;
bool ignore_P_G = false;

int main(int argc, char* argv[])
{
    Chip chip;

    std::ifstream pad2ball_fin;
    pad2ball_fin.clear();

    check_argument(argc, argv);

    if (chip.parser(argc, argv) == -1) {
        std::cout << "Program terminated because file open error!\n";
        return 0;
    }
    
    // chip.netlist_Content();

    // Output "netlist.lp" file
    if (LP_out != -1)
    {
        std::ofstream LP_file;
        LP_file.clear();

        LP_file.open(argv[LP_out + 1]);
        if (LP_file.is_open()) {
            chip.output_LP_File(LP_file);
        }
        else {
            std::cout << "open file \"" << argv[LP_out + 1] << "\" error.\n";
        }
        
        LP_file.close();
        std::cout << "\n----------output \"" << argv[LP_out + 1] << "\" file done!----------\n";
    }

    if (M_out != -1)
    {
        std::ofstream M_file;
        M_file.clear();

        M_file.open(argv[M_out + 1]);
        if (M_file.is_open()) {
            chip.output_M_File(M_file, argv[M_out + 1]);
        }
        else {
            std::cout << "open file \"" << argv[M_out + 1] << "\" error.\n";
        }
        
        M_file.close();
        std::cout << "\n----------output \"" << argv[M_out + 1] << "\" file done!----------\n";
    }

    if (GUI)
    {
        std::cout << "no graph haha!\n";
        //auto app = Application::create(argc, argv, "org.gtkmm.example");
    }

    return 0;
}

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
            GUI = true;
    }
    return;
}

Cartesian CG(std::vector<Cartesian> pos)
{
    Cartesian cg;
    for (size_t i = 0; i < pos.size(); i++)
    {
        cg.x += pos[i].x;
        cg.y += pos[i].y;
    }
    cg.x /= static_cast<double>(pos.size());
    cg.y /= static_cast<double>(pos.size());
    return cg;
}

bool ignore_Power_Ground(std::string str)
{
    if (ignore_P_G && ( str.find("P") != std::string::npos || 
						str.find("V") != std::string::npos || 
						str.find("VCC") != std::string::npos || 
						str.find("VDD") != std::string::npos || 
						str.find("G") != std::string::npos || 
						str.find("VSS") != std::string::npos
					  ))
	{
		std::cout << "ignore " << str <<std::endl;
		return true;
	}
    else
		return false;
}
