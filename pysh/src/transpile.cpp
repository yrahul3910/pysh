#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "formatter.hpp"
#include "transpile.hpp"

std::ostream& inject_cmd_call(std::ostream& out, const std::string& cmd, const int indent_level)
{
    // Add in indents
    out << std::string(indent_level * 4, ' ');

    // Inject subprocess call
    out << "__proc = subprocess.Popen(f'" << cmd << "', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n";
    out << std::string(indent_level * 4, ' ');
    out << "__proc.wait()\n";
    out << std::string(indent_level * 4, ' ');
    out << "EXIT_CODE = __proc.returncode\n";
    out << std::string(indent_level * 4, ' ');
    out << "__comm = __proc.communicate()\n";
    out << std::string(indent_level * 4, ' ');
    out << "_, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n";

    return out;
}

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
    // TODO: We might want to support single quotes too.
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

            // Get the formatter before the backtick
            size_t fmt_start_idx = (i == 0) ? 0 : cmd_idx[i - 1];
            size_t fmt_end_idx = cmd_idx[i];
            std::string potential_fmt = line.substr(fmt_start_idx, fmt_end_idx - fmt_start_idx + 1);

            /*
             * The regex for a formatter string. See https://github.com/yrahul3910/pysh/issues/7#issuecomment-1803175723
             */
            boost::regex fmt_rgx(R"((?:([a-zA-Z0-9_.]+)(?:\.(foreach)(\[[a-z]+\])?(\([a-zA-Z0-9_]+(?:,\s*?[a-zA-Z0-9_]+)*\))(?::([^`]*))?)?)?`)");
            boost::smatch matches;
            if (boost::regex_search(potential_fmt, matches, fmt_rgx)) {
                std::string collection, attributes, func_call, format;

                if (matches.size() > 1 && !std::string(matches[2]).empty()) {
                    // We have a more complex formatter.
                    collection = matches[1];
                    // In the future, if we implement more functions than `foreach`, we need to capture the function name too.
                    attributes = matches[3];
                    // TODO: Implement async attribute
                    func_call = matches[4];

                    if (!func_call.empty()) {
                        func_call = func_call.substr(1, func_call.size() - 2);
                    }

                    format = matches[5];

                    out << std::string(indent_level * 4, ' ');
                    out << "__coll = []\n";
                    out << "for " << func_call << " in enumerate(" << collection << "):\n";
                    indent_level++;
                } else {
                    format = matches[1];
                }

                // Inject the subprocess call
                inject_cmd_call(out, substr, indent_level);

                // Parse the formatter. If "str", do nothing.
                if (format != "str" && !format.empty()) {
                    type_formatter formatter{format, indent_level};
                    std::string formatted = formatter.format();

                    // Output formatted string
                    out << formatted;
                }

                if (matches.size() > 1 && !std::string(matches[2]).empty()) {
                    out << std::string(indent_level * 4, ' ');
                    out << "__coll.append(_)\n";

                    indent_level--;
                }

                // Replace the pysh line with transpiled line
                size_t match_pos = matches.position();

                if (matches.size() > 1 && !std::string(matches[2]).empty()) {
                    out << line.replace(match_pos, cmd_idx[j] - match_pos + 1, "__coll") << "\n";
                } else {
                    out << line.replace(match_pos, cmd_idx[j] - match_pos + 1, "_") << "\n";
                }
            }
        }
    } else {
        out << line << "\n";
    }

    return out;
}
