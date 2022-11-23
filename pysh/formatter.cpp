#include "formatter.h"

type_formatter::type_formatter(std::string fmt, int indent_level, bool spaces_to_indent)
    : fmt{std::move(fmt)}, indent_level{indent_level}, spaces_to_indent{spaces_to_indent}
{}

std::string type_formatter::get_indent_string(bool additional=false) const {
    std::string indent_string;
    if (spaces_to_indent) {
        indent_string = std::string((indent_level + additional) * 4, ' ');
    } else {
        indent_string = std::string(indent_level + additional, '\t');
    }
    return indent_string;
}

/**
 * Returns Python code that checks whether the string
 * can be safely casted to the desired type.
 * 
 * TODO: Check indent level
 */
std::string type_formatter::get_safe_formatter() const {
    std::string check_cast_code = "try:\n" + get_indent_string(true) +
        "_ = " + fmt + "(_)\n" + get_indent_string() + "except ValueError:\n" + get_indent_string(true) +
        "raise\n";

    return check_cast_code;
}

std::string type_formatter::format() const
{
    if (fmt == "int" ||
        fmt == "float")
        return get_safe_formatter();

    if (fmt == "list")
        return "_ = _.split('\\n')\n" + get_safe_formatter();

    if (fmt.starts_with("list.")) {
        std::string list_type = fmt.substr(5);
        return "_ = [" + list_type + "(x) for x in _.split('\\n')]\n";
    }

    throw "Formatter for type does not exist.";
};
