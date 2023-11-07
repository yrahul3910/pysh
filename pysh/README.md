# pysh

<a href="https://codecov.io/gh/yrahul3910/pysh" > 
 <img src="https://codecov.io/gh/yrahul3910/pysh/graph/badge.svg?token=16LRWU2DMA"/> 
 </a>

The goal of this project is to develop a superset of Python that allows users to write Shell commands in Python. These are transpiled into Python that call the required shell commands.

## Syntax

pysh follows a simple syntax, but has the following basic rules:

* Most valid Python code is valid pysh code
* Shell commands are written in the form `` `cmd` ``
* You can escape backticks using backslashes. This is necessary in edge cases, where for example, you have a backtick in a string.
* Within the backticks, you can reference variables using the syntax `` {var} ``
* You can add **formatters** before the backtick to indicate the type you wish to cast the result to. For example, `` int`cat file.txt | wc -l` `` will attempt to cast the result of the command to an integer.
* Formatters must be alphanumerics or underscores, other characters are not permitted in the name.
* There are a few built-in formatters: `str`, `int`, `float`, `list`. If no formatter is specified, `str` is assumed.
* Within a _formatted_ expression (i.e., inside backticks), standard Python 3 f-string syntax applies. For example, to use awk expressions, use double curly braces. `` awk "{{print $1}}" ``
* Inside formatted expressions, single quotes are not permitted. Use double quotes instead.
* pysh extends the `list` class to add a `map` function that works similarly to the JavaScript `Array.map`. It takes a lambda as input, applies it to each element, and then returns the final list.

## Global variables

When you use the template syntax to run a process via `pysh`, it exposes two variables after the process has run: `EXIT_CODE` and `STDERR`. `EXIT_CODE` is the exit code of the process, and `STDERR` is the standard error output of the process. Note that
calling a process via the template syntax is synchronous.

## Compiling

### Requirements

pysh requires C++20 and CMake 3.13 or higher, and uses Boost with the `program_options` library. It has been tested on macOS Ventura. Install Boost as follows:

### macOS

```sh
./bootstrap.sh
./b2 install --with-program_options
```

### Debian-based distros

```sh
sudo apt update && sudo apt install libboost-all-dev
```

### RHEL-based distros

```sh
sudo yum install epel-release && sudo yum install boost-devel
```

### SUSE-based distros

```sh
sudo zypper install boost-devel
```

### Arch-based distros

```sh
sudo pacman -S boost
```

### Gentoo

```sh
sudo emerge dev-libs/boost
```

### Instructions

From the source directory, run

```shell
cmake -S . -B build
cd build
make && sudo make install
```

On Apple Silicon Macs, you might need `cmake -S . -B build -DCMAKE_OSX_ARCHITECTURES=arm64`.

## Testing

Testing is implemented at two levels: one using Catch2 for unit tests, and another using Python's 
`unittest` module for integration tests. To run the unit tests, run `make test` from the build directory. 

To run the integration tests, run 

```sh
pysh tests.pysh -o tests.py -t
pytest tests.py --cov
rm tests.py .coverage
```

Both of these are done by the `run_tests.sh` script.

## Known Issues

As of the latest version, there is one major known issue: list and dict comprehensions. Specifically, the following syntax
is not yet supported:

```
numbers = [int`cat {file}` for file in files]
```

That is, while comprehensions themselves are fine, using template strings within them is not. A future version will address
this.

## Change Log

### v1.3.0

* Support for user-defined formatters! Any Python callable that takes one argument is a valid formatter.

### v1.2.3

* Added global `EXIT_CODE` and `STDERR` variables.

### v1.2.2

* Added `-o` option (output file).
* Bug fix when templated commands are in a comment.

### v1.2.1

* Updated `subprocess.run` to use `cwd=os.getcwd()`.

### v1.2

* Added the `--transpile` flag.

### v1.1

* Added the `-v` option for version.
* Added the `map` extension for the `list` class.
* Bug fixes