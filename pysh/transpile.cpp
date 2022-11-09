#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <memory>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include "formatter.h"

/** Process a single line.
 *
 * @param line - The line to process
 * @return The processed Python code
 */
std::string process_line(std::string& line)
{
    std::ostringstream generated;

    // Parse template args in the string.
    if (line.find("`") != std::string::npos) {
        // Find all indices.
        std::vector<size_t> cmd_idx;
        size_t cur_tick_idx = 0;
        size_t next_tick_idx;

        // Find all backticks
        while ((next_tick_idx = line.find("`", cur_tick_idx)) != std::string::npos) {
            // First, check that it is not escaped.
            if (next_tick_idx <= 0 || line[next_tick_idx - 1] != '\\')
                cmd_idx.push_back(next_tick_idx);

            cur_tick_idx = next_tick_idx + 1;
        }

        // Ensure we have an even number of indices
        if (cmd_idx.size() % 2 == 1)
            throw "Invalid number of template quotes";

        // Begin substitution using formatters.
        for (size_t i{}, j{1}; i < cmd_idx.size(); i += 2, j += 2) {
            std::string substr = line.substr(cmd_idx[i] + 1, cmd_idx[j] - cmd_idx[i] - 1);

            generated << "_ = subprocess.run(f'" << substr << "'.split(), capture_output=True).stdout.decode('utf-8').strip()\n";

            // Check for formatters
            if (cmd_idx[i] > 0 && (std::isalnum(line[cmd_idx[i] - 1]) || line[cmd_idx[i] - 1] == '_')) {
                size_t k;
                for (k = cmd_idx[i] - 2; k >= 0; --k) {
                    if (!std::isalnum(line[k]) && line[k] != '_')
                        break;
                }

                std::string format = line.substr(k + 1, cmd_idx[i] - k - 1);

                // Apply formatter
                // If "str", do nothing.
                if (format != "str") {
                    std::unique_ptr<type_formatter> formatter = std::make_unique<type_formatter>(format);
                    generated << formatter->format();
                }
            }

            // Now, replace the part in quotes with our variable
            generated << line.replace(cmd_idx[i], cmd_idx[j] - cmd_idx[i] + 1, "_");
        }
    } else {
        generated << line;
    }

    return generated.str();
}

int main(int argc, char* argv[])
{
    // TODO: Change this to a filename input
    std::ifstream fin(argv[1]);
    std::ofstream fout("out.py");

    fout << "import subprocess\n\n";

    std::string line;

    while (std::getline(fin, line)) {
        fout << process_line(line) << std::endl;
    }

    fout.close();

    // Run the code
    const char* path = std::getenv("PATH");
    std::filesystem::path cur_path = std::filesystem::current_path();
    std::string new_path = std::string(path) + ":" + cur_path.string();

    if (setenv("PATH", new_path.c_str(), 1) != 0)
        throw "Failed to set PATH";

    std::system("python out.py");

    return 0;
}
