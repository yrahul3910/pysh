#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "formatter.hpp"
#include "transpile.hpp"

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
            out << "proc = subprocess.Popen(f'" << substr << "', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)\n";
            out << std::string(indent_level * 4, ' ');
            out << "proc.wait()\n";
            out << std::string(indent_level * 4, ' ');
            out << "EXIT_CODE = proc.returncode\n";
            out << std::string(indent_level * 4, ' ');
            out << "__comm = proc.communicate()\n";
            out << std::string(indent_level * 4, ' ');
            out << "_, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n";

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
