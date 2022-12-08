#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <exception>
#include <filesystem>
#include "formatter.h"


// Replace all occurrences of a substring within a string
// from https://stackoverflow.com/a/28766792/2713263
std::string string_replace( const std::string & s, const std::string & findS, const std::string & replaceS )
{
    std::string result = s;
    auto pos = s.find( findS );
    if ( pos == std::string::npos ) {
        return result;
    }
    result.replace( pos, findS.length(), replaceS );
    return string_replace( result, findS, replaceS );
}

/** Process a single line.
 *
 * @param line - The line to process
 * @return The processed Python code
 */
std::ostream& process_line(std::string& line, std::ostream& out)
{
    static int indent_level = 0;
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
        if (!spaces_to_indent)
            line.replace(match.position(), match.length(), std::string(4 * indent_level, ' '));
    } else {
        indent_level = 0;
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
                line = string_replace( line, p.first, p.second );
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
            if (spaces_to_indent)
                out << std::string(indent_level * 4, ' ');
            else
                out << std::string(indent_level, '\t');

            // Inject subprocess call
            out << "_ = subprocess.Popen(f'" << substr << "', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).communicate()[0].decode('utf-8')\n";

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
                    type_formatter formatter{format, indent_level, spaces_to_indent};
                    std::string formatted = formatter.format();

                    // Add indents
                    if (spaces_to_indent)
                        out << std::string(indent_level * 4, ' ');
                    else
                        out << std::string(indent_level, '\t');

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
        std::cerr << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    if (std::string(argv[1]) == "-v") {
        std::cout << "pysh version 1.1" << std::endl;
        return 0;
    }

    std::filesystem::path filename = argv[1];
    if (!exists(filename)) {
        std::cerr << argv[1] << " does not exist." << std::endl;
        return 2;
    }

    // TODO: Change this to a filename input
    std::ifstream fin(argv[1]);
    std::ofstream fout("out.py");

    fout << "import subprocess\n\n";
    fout << "class list(list):"
            "    def map(self, f):"
            "        return list(map(f, self))\n\n";

    std::string line;

    while (std::getline(fin, line)) {
        process_line(line, fout);
    }

    fout.flush();
    fout.close();

    // Run the code
    const char* path = std::getenv("PATH");
    std::filesystem::path cur_path = std::filesystem::current_path();
    std::string new_path = std::string(path) + ":" + cur_path.string();

    if (setenv("PATH", new_path.c_str(), 1) != 0)
        throw std::runtime_error("Failed to set PATH");

    std::system("python3 out.py");

    return 0;
}
