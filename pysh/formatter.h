#ifndef FORMATTER_H
#define FORMATTER_H

#include <string>
#include <memory>

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
    virtual ~basic_formatter() = default;
    virtual std::string format() const = 0; 
};

class type_formatter : public basic_formatter
{
    std::string fmt;
    std::string get_safe_formatter() const;

public:
    explicit type_formatter(const std::string);
    std::string format() const override;
};



#endif
