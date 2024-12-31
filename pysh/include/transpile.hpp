#ifndef PYSH_TRANSPILE_HPP
#define PYSH_TRANSPILE_HPP

#include <iostream>

std::ostream& process_line(std::string& line, std::ostream& out);
void transpile_file(const std::string&, const std::string&, bool);

#endif //PYSH_TRANSPILE_HPP
