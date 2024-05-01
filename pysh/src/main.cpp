#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <boost/program_options.hpp>
#include "transpile.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;

enum ErrorCode {
    SUCCESS = 0,
    ERR_FILE_NOT_FOUND = 2,
    ERR_FILE_EXISTS_SAFE_MODE = 3
};

void transpile_file(const std::string& filename, const std::string& out_filename, bool safe_mode=false)
{
    if (safe_mode && fs::exists(fs::path(out_filename))) {
        std::cerr << out_filename << " already exists." << std::endl;
        std::cerr << "Note: Safe mode is enabled. If you wish to overwrite the file, please disable safe mode." << std::endl;
        exit(ERR_FILE_EXISTS_SAFE_MODE);
    }

    std::ifstream fin(filename.c_str());
    std::ofstream fout(out_filename.c_str());

    fout << "import subprocess\nimport os\nimport threading\n\n";
    fout << "class list(list):\n"
            "    def map(self, f):\n"
            "        return list(map(f, self))\n\n";

    std::string line;

    while (std::getline(fin, line)) {
        process_line(line, fout);
    }

    fout.flush();
    fout.close();
}

int main(int argc, char* argv[])
{
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("version,v", "print version")
            ("transpile,t", "transpile-only mode")
            ("input,i", po::value<std::string>()->default_value("main.pysh"), "input file")
            ("output,o", po::value<std::string>()->default_value(""), "output file")
            ("safe,s", "safe mode")
            ("recursive,r", "recursively transpile all files in the directory")
            ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return EXIT_SUCCESS;
    }

    if (vm.count("version")) {
        std::cout << "pysh version 1.4.0" << std::endl;
        return EXIT_SUCCESS;
    }

    fs::path filename = vm["input"].as<std::string>();
    if (!fs::exists(filename) && !vm.count("recursive")) {
        std::cerr << argv[1] << " does not exist." << std::endl;
        return ERR_FILE_NOT_FOUND;
    }

    std::string out_filename = vm["output"].as<std::string>();
    if (out_filename.empty()) {
        out_filename = filename.replace_extension(".py").string();
    }

    bool safe_mode = vm.count("safe");

    if (safe_mode && fs::exists(std::filesystem::path(out_filename))) {
        std::cerr << out_filename << " already exists." << std::endl;
        std::cerr << "Note: Safe mode is enabled. If you wish to overwrite the file, please disable safe mode." << std::endl;
        return ERR_FILE_EXISTS_SAFE_MODE;
    }

    // If recursive option is set, transpile all files in the directory
    if (vm.count("recursive")) {
        for (const auto& entry : fs::recursive_directory_iterator(filename.parent_path())) {
            if (entry.path().extension() == ".pysh") {
                // transpile_file will auto-change the extension
                transpile_file(entry.path().string(), "", safe_mode);
            }
        }
    } else {
        transpile_file(filename, out_filename);
  }

    if (!vm.count("transpile")) {
        // Run the code
        const char *path = std::getenv("PATH");
        fs::path cur_path = fs::current_path();
        std::string new_path = std::string(path) + ":" + cur_path.string();

        if (setenv("PATH", new_path.c_str(), 1) != 0)
            throw std::runtime_error("Failed to set PATH");

        std::string command = "python3 " + out_filename;
        std::system(command.c_str());
    }

    return 0;
}
