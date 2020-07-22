#include"main.h"
#include"file_analyst.h"

Chip *chip = nullptr;

int LP_out = -1;
int M_out = -1;
int GUI = -1;
bool min_output = true;
bool ignore_P_G = false;

Cartesian GOD_Center(0.0, 0.0);
double GOD_Rotation = 0.0;
Cartesian GOD_GOD_Center(0.0, 0.0);
double GOD_GOD_Rotation = 0.0;

//vector<double> ori_rotas;

int main(int argc, char* argv[])
{
    chip = new Chip;

    check_argument(argc, argv);

    if (parser(argc, argv) == -1) {
        std::cout << "Program terminated because file open error!\n";
        return 0;
    }

    // Output .lp file
    if (LP_out != -1)
    {
        std::ofstream LP_file;
        LP_file.clear();

        LP_file.open(argv[LP_out + 1]);
        if (LP_file.is_open()) {
            chip->output_LP_File(LP_file);
        }
        else {
            std::cout << "open file \"" << argv[LP_out + 1] << "\" error.\n";
        }
        
        LP_file.close();
        std::cout << "\n----------output \"" << argv[LP_out + 1] << "\" file DONE!----------\n";
    }

    // Output .m file
    if (M_out != -1)
    {
        std::ofstream M_file;
        M_file.clear();

        M_file.open(argv[M_out + 1]);
        if (M_file.is_open()) {
            chip->output_M_File(M_file, argv[M_out + 1]);
        }
        else {
            std::cout << "open file \"" << argv[M_out + 1] << "\" error.\n";
        }
        
        M_file.close();
        std::cout << "\n----------output \"" << argv[M_out + 1] << "\" file DONE!----------\n";
    }

    // Show GUI
    if (GUI != -1)
    {
        // Set GOD parameter
        if (GUI + 3 < argc) {
            GOD_Center.x = stod(argv[GUI + 1]);
            GOD_Center.y = stod(argv[GUI + 2]);
            GOD_Rotation = stod(argv[GUI + 3]);
            if (GUI + 6 < argc) {
                GOD_GOD_Center.x = stod(argv[GUI + 4]);
                GOD_GOD_Center.y = stod(argv[GUI + 5]);
                GOD_GOD_Rotation = stod(argv[GUI + 6]);
            }
        }

        WindowApp app;
        app.Run(argc, argv);
    }

    return 0;
}