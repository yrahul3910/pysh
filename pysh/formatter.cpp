#include "formatter.h"

type_formatter::type_formatter(const std::string& fmt) {
    this->fmt = fmt;
}

/**
 * Returns Python code that checks whether the string
 * can be safely casted to the desired type.
 * 
 * TODO: Check indent level
 */
std::string type_formatter::get_safe_formatter() {
    std::string check_cast_code = "try:\n\t"
        "_ = " + fmt + "(_)\nexcept ValueError:\n\t"
        "raise";

    return check_cast_code;
}

std::string type_formatter::format()
{
    if (fmt == "int" ||
        fmt == "float")
        return get_safe_formatter();

    if (fmt == "list")
        return "_ = _.split('\\n')\n";

    if (fmt.starts_with("list.")) {
        std::string list_type = fmt.substr(5);
        return "_ = [" + list_type + "(x) for x in _.split('\\n')]\n";
    }

    throw "Formatter for type does not exist.";
};
