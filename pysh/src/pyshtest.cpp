#include <boost/program_options/positional_options.hpp>
#include <filesystem>
#include <boost/program_options.hpp>
#include "transpile.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("input,i", po::value<std::vector<std::string>>(), "input file (if specified, turns off recursive mode)")
            ("recursive,r", po::value<bool>()->default_value(true), "recursively transpile all files (default)")
            ("version,v", "print version");
    
    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return EXIT_SUCCESS;
    }

    if (vm.count("version")) {
        std::cout << "pysh version 1.4.0" << std::endl;
        return EXIT_SUCCESS;
    }

    std::vector<std::string> files;
    if (vm.count("input")) {
        files = vm["input"].as<std::vector<std::string>>();
    }

    // input files override the recursive option
    if (vm.count("input")) {
        for (const auto& file : files) {
            fs::path filename = file;
            if (!fs::exists(filename) && !vm.count("recursive")) {
                std::cerr << argv[1] << " does not exist." << std::endl;
                return 1;
            }
            std::string out_filename = filename.replace_extension(".py").string();

            transpile_file(filename, out_filename, false);
        }
    }
    else if (vm.count("recursive")) {
        for (const auto& entry : fs::recursive_directory_iterator(fs::current_path())) {
            if (entry.path().extension() == ".pysh") {
                // transpile_file will auto-change the extension
                transpile_file(entry.path().string(), "", false);
            }
        }
    } 

    const char *path = std::getenv("PATH");
    fs::path cur_path = fs::current_path();
    std::string new_path = std::string(path) + ":" + cur_path.string();

    if (setenv("PATH", new_path.c_str(), 1) != 0)
        throw std::runtime_error("Failed to set PATH");

    std::string test_paths;
    if (!files.size()) {
        test_paths = ".";
    } else {
        for (const std::string& file : files) {
            // We've checked for file not existing above already
            test_paths += file + " ";
        }
    }

    std::string command = "pytest " + test_paths;
    std::system(command.c_str());
}
