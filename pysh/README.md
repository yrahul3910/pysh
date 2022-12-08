# pysh

The goal of this project is to develop a superset of Python that allows users to write Shell commands in Python. These are transpiled into Python that call the required shell commands.

## Syntax

pysh follows a simple syntax, but has the following basic rules:

* Most valid Python code is valid pysh code
* Shell commands are written in the form `` `cmd` ``
* You can escape backticks using backslashes. This is necessary in edge cases, where for example, you have a backtick in a string.
* Within the backticks, you can reference variables using the syntax `` {var} ``
* You can add **formatters** before the backtick to indicate the type you wish to cast the result to. For example, `` int`wc -l file.txt` `` will attempt to cast the result of the command to an integer.
* Formatters must be alphanumerics or underscores, other characters are not permitted in the name.
* There are a few built-in formatters: `str`, `int`, `float`, `list`. If no formatter is specified, `str` is assumed.
* Within a _formatted_ expression (i.e., inside backticks), standard Python 3 f-string syntax applies. For example, to use awk expressions, use double curly braces. `` awk`{{print $1}}` ``
* Inside formatted expressions, single quotes are not permitted. Use double quotes instead.
* pysh extends the `list` class to add a `map` function that works similarly to the JavaScript `Array.map`. It takes a lambda as input, applies it to each element, and then returns the final list.

## Compiling

### Requirements

pysh requires C++20 and CMake 3.13 or higher. It has been tested on macOS Ventura.

### Instructions

From the source directory, run

```
cmake -S . -B build
cd build
make
```

## Examples

There are several examples in the `examples/` directory. Here are a few:

### Counting lines in a file

```
filename = './data/lines.txt'
n_lines = int`wc -l {filename}`
print(n_lines, 'lines in file')
```

## Change Log

### v1.1

* Added the `-v` option for version.
* Added the `map` extension for the `list` class.
* Bug fixes