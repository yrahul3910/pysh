#include "formatter.hpp"

type_formatter::type_formatter(std::string fmt, int indent_level)
    : fmt{std::move(fmt)}, indent_level{indent_level}
{}

std::string type_formatter::get_indent_string(int additional=0) const {
    std::string indent_string;
    indent_string = std::string(std::max(0, indent_level + additional) * 4, ' ');

    return indent_string;
}

/**
 * Returns Python code that checks whether the string
 * can be safely casted to the desired type.
 */
std::string type_formatter::get_safe_formatter() const {
    std::string check_cast_code = get_indent_string() + "try:\n" + get_indent_string(1) +
        "_ = " + fmt + "(_)\n" + get_indent_string() + "except ValueError:\n" + get_indent_string(1) +
        "raise\n";

    return check_cast_code;
}

/**
 * Returns Python code that checks whether the string
 * can be safely cast to a user-defined formatter.
 */
std::string type_formatter::get_safe_custom_formatter() const {
    std::string check_cast_code = get_indent_string() + \
        "try:\n" + \
        get_indent_string() + "_ = " + fmt + "(_)\n" + \
        get_indent_string() + "except NameError:\n" + \
        get_indent_string(1) + "raise NameError('" + fmt + " is not a valid custom formatter.')\n" + \
        get_indent_string() + "except TypeError:\n" + \
        get_indent_string(1) + "raise TypeError('" + fmt + " is not callable, or takes too many args.')\n";

    return check_cast_code;
}

std::string type_formatter::format() const
{
    if (fmt == "int" ||
        fmt == "float")
        return get_safe_formatter();

    if (fmt == "list")
        return get_indent_string() + "_ = _.split('\\n')\n" + get_safe_formatter();

    if (fmt.starts_with("list.")) {
        std::string list_type = fmt.substr(5);
        return get_indent_string() + "_ = [" + list_type + "(x) for x in _.split('\\n')]\n";
    }

    return get_safe_custom_formatter();
};
