#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <boost/program_options.hpp>
#include "transpile.hpp"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " FILE [OPTIONS]\n";
        return 1;
    }

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("version,v", "print version")
            ("transpile,t", "transpile-only mode")
            ("output,o", po::value<std::string>()->default_value("out.py"), "output file")
            ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    if (vm.count("version")) {
        std::cout << "pysh version 1.2.2" << std::endl;
        return 0;
    }

    std::filesystem::path filename = argv[1];
    if (!exists(filename)) {
        std::cerr << argv[1] << " does not exist." << std::endl;
        return 2;
    }

    std::string out_filename = vm["output"].as<std::string>();
    std::ifstream fin(argv[1]);
    std::ofstream fout(out_filename);

    fout << "import subprocess\nimport os\n\n";
    fout << "class list(list):\n"
            "    def map(self, f):\n"
            "        return list(map(f, self))\n\n";

    std::string line;

    while (std::getline(fin, line)) {
        process_line(line, fout);
    }

    fout.flush();
    fout.close();

    if (!vm.count("transpile")) {
        // Run the code
        const char *path = std::getenv("PATH");
        std::filesystem::path cur_path = std::filesystem::current_path();
        std::string new_path = std::string(path) + ":" + cur_path.string();

        if (setenv("PATH", new_path.c_str(), 1) != 0)
            throw std::runtime_error("Failed to set PATH");

        std::string command = "python3 " + out_filename;
        std::system(command.c_str());
    }

    return 0;
}