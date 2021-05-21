#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>

/**
 * The base class for formatters. This is an abstract class
 * and should be extended to implement specific formatters.
 * Formatters must implement the `format()` function, which
 * should return a std::string containing Python code to process
 * a variable called _, which will contain the output from shell
 * code in the transpiled program. As a template, the code should
 * end up assigning _ to the correct type. The Python code should
 * end in a newline.
 */
class basic_formatter
{
public:
    basic_formatter() = default;
    virtual std::string format() = 0; 
};

class type_formatter : public basic_formatter
{
    std::string fmt;
    std::string get_safe_formatter();

public:
    type_formatter(const std::string&);
    type_formatter() = delete;
    virtual std::string format();
};



#endif
