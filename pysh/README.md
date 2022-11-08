# pysh

The goal of this project is to develop a superset of Python that allows users to write Shell commands in Python. These are transpiled into Python that call the required shell commands.

## Syntax

pysh follows a simple syntax, but has the following basic rules:

* All valid Python code is valid pysh code
* Shell commands are written in the form `` `cmd` ``
* You can escape backticks using backslashes. This is necessary in edge cases, where for example, you have a backtick in a string.
* Within the backticks, you can reference variables using the syntax `` {var} ``
* You can add **formatters** before the backtick to indicate the type you wish to cast the result to. For example, `` int`wc -l file.txt` `` will cast the result of the command to an integer.
* Formatters must be alphanumerics or underscores, other characters are not permitted in the name.
* There are a few built-in formatters: `str`, `int`, `float`, `list`. If no formatter is specified, `str` is assumed.

## Compiling

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

## Future Goals

- [ ] Provide built-in commands as Python code