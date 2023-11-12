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

TEST_CASE("list template parses to comprehension", "[transpile]")
{
    std::string line = "list.int`cat file.txt`";
    std::stringstream ss;
    process_line(line, ss);
    REQUIRE(ss.str() == "__proc = subprocess.Popen(f'cat file.txt', shell=True, cwd=os.getcwd(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)\n__proc.wait()\nEXIT_CODE = __proc.returncode\n__comm = __proc.communicate()\n_, STDERR = __comm[0].decode('utf-8').rstrip(), __comm[1].decode('utf-8').rstrip()\n_ = [int(x) for x in _.split('\\n')]\n_\n");
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
