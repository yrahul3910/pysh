#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

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

        while ((next_tick_idx = line.find("`", cur_tick_idx)) != std::string::npos) {
            // First, check that it is not escaped.
            if (next_tick_idx <= 0 || line[next_tick_idx-1] != '\\')
                cmd_idx.push_back(next_tick_idx);

            cur_tick_idx = next_tick_idx + 1;
        }

        // Ensure we have an even number of indices
        if (cmd_idx.size() % 2 == 1)
            throw "Invalid number of template quotes";

        // Begin substitution using formatters.
        for (size_t i{}, j{1}; i < cmd_idx.size(); i += 2, j += 2) {
            std::string substr = line.substr(cmd_idx[i], cmd_idx[j] - cmd_idx[i]);

            generated << "_ = subprocess.run(" << substr << "capture_output=True)";

            // Check for formatters
            if (i > 0 && (std::isalnum(line[i-1]) || line[i-1] == '_')) {
                size_t k;
                for (k = i - 2; k >= 0; --k) {
                    if (!std::isalnum(line[k]) && line[k] != '_')
                        break;
                }

                std::string formatter = line.substr(k, i - 1 - k);
                // Apply formatter
                // If "str", do nothing.
                if (formatter == "str")
                    ;

                // TODO: Keep track of tabs and substitute \n with \n followed by tabs.
            }

        }
    }

    return "";
}

int main()
{
    return 0;
}
