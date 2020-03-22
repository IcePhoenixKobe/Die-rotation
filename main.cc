#include"main.h"
#include"chip.h"

int main(int argc, char* argv[])
{
    Chip chip;

    std::ifstream pad2ball_fin;
    pad2ball_fin.clear();

    if (chip.parser(argc, argv) == -1) {
        std::cout << "Program terminated because file open error!\n";
        return 0;
    }
    
    // chip.netlist_Content();

    // Output "netlist.lp" file
    std::ofstream LP_file;
    LP_file.clear();

    LP_file.open(argv[3]);
    if (LP_file.is_open()) {
        chip.output_LP_File(LP_file);
    }
    else {
        std::cout << "open file \"" << argv[3] << "\" error.\n";
    }
    
    LP_file.close();
    std::cout << "\n----------output \"" << argv[3] << "\" file done!----------\n";
    return 0;
}