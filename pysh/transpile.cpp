#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "formatter.h"

 namespace po = boost::program_options;

/** Process a single line.
 *
 * @param line - The line to process
 * @return The processed Python code
 */
std::ostream& process_line(std::string& line, std::ostream& out)
{
    static int indent_level = 0;
    // TODO: Maybe this should be static?
    bool spaces_to_indent = true;

    std::smatch match;
    std::regex regex("^\\s+");

    // Add in indents
    if (std::regex_search(line, match, regex)) {
        // Count the number of indents
        int spaces = 0;
        for (char c : match[0].str()) {
            if (c == '\t') {
                spaces_to_indent = false;
                indent_level++;
            }
            else if (c == ' ')
                spaces++;
        }

        if (spaces_to_indent)
            indent_level = static_cast<int>(spaces / 4);

        // Now replace the indents with spaces
        if (!spaces_to_indent) {
            line.replace(match.position(), match.length(), std::string(4 * indent_level, ' '));
            spaces_to_indent = true;
        }
    } else {
        indent_level = 0;
    }

    // Remove everything after the first # sign. However, if the # sign is in a string, don't remove it.
    bool in_string = false;
    for (size_t i{}; i < line.size(); ++i) {
        if (line[i] == '"') {
            in_string = !in_string;
        } else if (line[i] == '#' && !in_string) {
            line.erase(i);
            break;
        }
    }

    // Parse template args in the string.
    if (line.find('`') != std::string::npos) {
        // Find all indices.
        std::vector<size_t> cmd_idx;
        size_t cur_tick_idx = 0;
        size_t next_tick_idx;

        // Find all backticks
        while ((next_tick_idx = line.find('`', cur_tick_idx)) != std::string::npos) {
            // First, check that it is not escaped.
            static std::vector<std::pair<std::string, std::string>> patterns = {
                    { "\\\\" , "\\" },
                    { "\\`", "`" },
            };
            for ( const auto & p : patterns ) {
                boost::replace_all(line, p.first, p.second);
            }

            if (next_tick_idx > 0 && line[next_tick_idx - 1] != '\\')
                cmd_idx.push_back(next_tick_idx);

            cur_tick_idx = next_tick_idx + 1;
        }

        // Ensure we have an even number of indices
        if (cmd_idx.size() % 2 == 1)
            throw std::invalid_argument("Invalid number of backticks in line: " + line);

        // Begin substitution using formatters.
        for (size_t i{}, j{1}; i < cmd_idx.size(); i += 2, j += 2) {
            std::string substr = line.substr(cmd_idx[i] + 1, cmd_idx[j] - cmd_idx[i] - 1);

            // Add in indents
            out << std::string(indent_level * 4, ' ');

            // Inject subprocess call
            out << "_ = subprocess.Popen(f'" << substr << "', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT).communicate()[0].decode('utf-8').rstrip()\n";

            // Check for formatters
            if (cmd_idx[i] > 0 && (std::isalnum(line[cmd_idx[i] - 1]) || line[cmd_idx[i] - 1] == '_')) {
                int k;
                for (k = cmd_idx[i] - 2; k >= 0; --k) {
                    if (!std::isalnum(line[k]) && line[k] != '_' && line[k] != '.')
                        break;
                }

                std::string format = line.substr(k + 1, cmd_idx[i] - k - 1);

                // Now that we have the format, remove it from the line.
                line.erase(k + 1, cmd_idx[i] - k - 1);
                cmd_idx[i] -= format.size();
                cmd_idx[j] -= format.size();

                // Apply formatter
                // If "str", do nothing.
                if (format != "str") {
                    type_formatter formatter{format, indent_level};
                    std::string formatted = formatter.format();

                    // Output formatted string
                    out << formatted;
                }
            }

            // Now, replace the part in quotes with our variable
            out << line.replace(cmd_idx[i], cmd_idx[j] - cmd_idx[i] + 1, "_") << "\n";
        }
    } else {
        out << line << "\n";
    }

    return out;
}

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

    std::ifstream fin(argv[1]);
    std::ofstream fout(vm["output"].as<std::string>());

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

        std::system("python3 out.py");
    }

    return 0;
}
