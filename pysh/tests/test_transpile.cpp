#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "transpile.hpp"

TEST_CASE("pure Python line is not modified", "[transpile]")
{
    std::string line = "print('Hello, world!')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "print('Hello, world!')\n");
}

TEST_CASE("indents are preserved", "[transpile]")
{
    std::string line = "    print('Hello, world!')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "    print('Hello, world!')\n");
}

TEST_CASE("tabs are preserved", "[transpile]")
{
    std::string line = "\tprint('Hello, world!')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "        print('Hello, world!')\n");
}

TEST_CASE("comments are removed", "[transpile]")
{
    std::string line = "print('Hello, world!') # This is a comment";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "print('Hello, world!') \n");
}

TEST_CASE("template literals are parsed", "[transpile]")
{
    std::string line = "int`cat file.txt`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__proc = subprocess.Popen(f'cat file.txt', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n__proc.wait()\nEXIT_CODE = __proc.returncode\n__comm = __proc.communicate()\n_, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\ntry:\n    _ = int(_)\nexcept ValueError:\n    raise\n_\n");
}

TEST_CASE("escaped backticks do not get transpiled", "[transpile]")
{
    std::string line = "print('\\`')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "print('`')\n");
}

TEST_CASE("escaped backticks do not get transpiled, test 2", "[transpile]")
{
    std::string line = "print('\\`xyz\\`')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "print('`xyz`')\n");
}

TEST_CASE("normal escaped characters transpile correctly", "[transpile]")
{
    std::string line = "print('\\n')";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "print('\\n')\n");
}

TEST_CASE("re-substitution works correctly", "[transpile]")
{
    std::string line = "n_lines = int(`wc -l {filename}`.split()[0])";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__proc = subprocess.Popen(f'wc -l {filename}', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "__proc.wait()\n"
                        "EXIT_CODE = __proc.returncode\n"
                        "__comm = __proc.communicate()\n"
                        "_, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n"
                        "n_lines = int(_.split()[0])\n");
}

TEST_CASE("async attribute transpiles correctly", "[transpile]")
{
    std::string line = "$a`cat {file}`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__proc = subprocess.Popen(f'cat {file}', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "_ = None\n"
                        "EXIT_CODE = None\n"
    );
}

TEST_CASE("list template parses to comprehension", "[transpile]")
{
    std::string line = "list.int`cat file.txt`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__proc = subprocess.Popen(f'cat file.txt', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "__proc.wait()\n"
                        "EXIT_CODE = __proc.returncode\n"
                        "__comm = __proc.communicate()\n"
                        "_, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n"
                        "try:\n"
                        "    _ = [int(x) for x in _.split('\\n')]\n"
                        "except ValueError:\n"
                        "    raise\n"
                        "_\n");
}

TEST_CASE("foreach works correctly", "[transpile]")
{
    std::string line = "lines = lines.foreach(i, file):int`cat {file}`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__coll = []\n"
                        "for i, file in enumerate(lines):\n"
                        "    __proc = subprocess.Popen(f'cat {file}', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "    __proc.wait()\n"
                        "    EXIT_CODE = __proc.returncode\n"
                        "    __comm = __proc.communicate()\n"
                        "    _, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n"
                        "    try:\n"
                        "        _ = int(_)\n"
                        "    except ValueError:\n"
                        "        raise\n"
                        "    __coll.append(_)\n"
                        "lines = __coll\n");
}

TEST_CASE("most complex formatting syntax transpiles correctly", "[transpile]")
{
    std::string line = "files[xyz].func().items().foreach(i, (key, val)):list.str`cat {file}`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__coll = []\n"
                        "for i, (key, val) in enumerate(files[xyz].func().items()):\n"
                        "    __proc = subprocess.Popen(f'cat {file}', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "    __proc.wait()\n"
                        "    EXIT_CODE = __proc.returncode\n"
                        "    __comm = __proc.communicate()\n"
                        "    _, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n"
                        "    _ = _.split('\\n')\n"
                        "    __coll.append(_)\n"
                        "__coll\n");
}

TEST_CASE("most complex formatting syntax transpiles correctly: test 2", "[transpile]")
{
    std::string line = "files[xyz].func().items().foreach(i, (key, val)):list.int`cat {file}`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__coll = []\n"
                        "for i, (key, val) in enumerate(files[xyz].func().items()):\n"
                        "    __proc = subprocess.Popen(f'cat {file}', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "    __proc.wait()\n"
                        "    EXIT_CODE = __proc.returncode\n"
                        "    __comm = __proc.communicate()\n"
                        "    _, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n"
                        "    try:\n"
                        "        _ = [int(x) for x in _.split('\\n')]\n"
                        "    except ValueError:\n"
                        "        raise\n"
                        "    __coll.append(_)\n"
                        "__coll\n");
}

TEST_CASE("most complex formatting syntax transpiles correctly: async case", "[transpile]")
{
    std::string line = "files[xyz].func().items().foreach(i, (key, val)):list.int$a`cat {file}`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__coll = []\n"
                        "for i, (key, val) in enumerate(files[xyz].func().items()):\n"
                        "    __proc = subprocess.Popen(f'cat {file}', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n"
                        "    _ = None\n"
                        "    EXIT_CODE = None\n");
}
