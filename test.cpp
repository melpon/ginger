#include "ginger.h"
#include <iostream>
#include <ios>
#include <vector>
#include <string>
#include <list>
#include <unordered_map>

std::string getStdin() {
    std::string output;
    std::string input;
    std::cin >> std::noskipws;
    while (std::getline(std::cin, input)) {
        output += input + '\n';
    }
    return output;
}

int failed = 0;

template<class Input>
void print_error(int line, Input input, std::string expected, std::string actual, std::string error = "") {
    std::cerr << "------------- TEST ERROR (" << line << ") ---------------" << std::endl;
    /*
    for (auto c: expected)
        std::cout << (int)c << std::endl;
    std::cout << "---" << std::endl;
    for (auto c: actual)
        std::cout << (int)c << std::endl;
    */
    std::cerr << "input: " << std::string(std::begin(input), std::end(input)) << std::endl;
    std::cerr << "expected: " << expected << std::endl;
    std::cerr << "actual: " << actual << std::endl;
    std::cerr << error << std::endl;
    ++failed;
}
void print_error(int line, const char* input, std::string expected, std::string actual, std::string error = "") {
    print_error(line, std::string(input), expected, actual, error);
}

template<class Input, class Dict>
void test_eq(Input input, std::string expected, Dict dic, int line) {
    try {
        std::stringstream ss;
        ginger::parse(input, dic, ginger::from_ios(ss));
        auto actual = ss.str();
        if (actual != expected) {
            print_error(line, input, expected, actual, "");
        }
    } catch (const ginger::parse_error& error) {
        print_error(line, input, expected, "PARSE ERROR", error.long_error());
    } catch (...) {
        print_error(line, input, expected, "UNKNOWN EXCEPTION", "");
    }
}
template<class Input, class Dict>
void test_exc(Input input, Dict dic, int line) {
    try {
        std::stringstream ss;
        ginger::parse(input, dic, ginger::from_ios(ss));
        auto actual = ss.str();
        print_error(line, input, "THROW PARSE ERROR", actual, "");
    } catch (ginger::parse_error& error) {
        //visible errors.
        //std::cout << error.long_error();
    } catch (...) {
        print_error(line, input, "THROW PARSE ERROR", "UNKNOWN EXCEPTION", "");
    }
}
#define TEST_EQ(input, expected) test_eq(input, expected, ginger::temple(), __LINE__)
#define TEST_EQ_T(input, expected, t) test_eq(input, expected, t, __LINE__)
#define TEST_EXC(input) test_exc(input, ginger::temple(), __LINE__)
#define TEST_EXC_T(input, t) test_exc(input, t, __LINE__)

int main() {
    TEST_EQ("Hello", "Hello");
    TEST_EQ("${{", "{{");
    TEST_EQ("$}}", "}}");
    TEST_EQ("$$", "$");
    TEST_EQ("$# comment", "");
    TEST_EQ("$# comment\n", "\n");

    TEST_EXC("$x");
    TEST_EXC("${");
    TEST_EXC("$}");

    // test ${variable}
    {
        ginger::temple t;
        t["value"] = 100;
        t["map"] = std::map<std::string, int>{ { "hoge", 1 }, { "fuga", 2 } };
        TEST_EQ_T("${value}", "100", t);
        TEST_EQ_T("${map.hoge}, ${map.fuga}", "1, 2", t);
        TEST_EQ_T("${  map.hoge  }, ${  map.fuga  }", "1, 2", t);
        TEST_EXC_T("${ map. hoge }", t);
        TEST_EXC_T("${ map .hoge }", t);
        TEST_EXC_T("${undefined}", t);
    }
    // test $for
    {
        ginger::temple t;
        t["xs"] = std::vector<int>{ 1, 2 };
        t["ys"] = std::map<std::string, std::vector<int>>{ { "hoge", { 1, 2, 3 } } };
        TEST_EQ_T("$for x in xs{{test}}", "testtest", t);
        TEST_EQ_T("$for x in xs {{ test }}", " test  test ", t);
        TEST_EQ_T("$for x in xs{{$for x in xs{{test}}}}", "testtesttesttest", t);
        TEST_EQ_T("$for y in ys.hoge{{${y}}}", "123", t);
        TEST_EQ_T("$for y in  \t  ys.hoge   {{${y}\n}}", "1\n2\n3\n", t);

        TEST_EXC_T("$forx in xs{{}}", t);
        TEST_EXC_T("$for xin xs{{}}", t);
        TEST_EXC_T("$for x inxs{{}}", t);
        TEST_EXC_T("$for x in xs{}}", t);
        TEST_EXC_T("$for x in xs{{}", t);
    }
    // test $if
    {
        ginger::temple t;
        t["true"] = true;
        t["false"] = false;

        // $if
        TEST_EQ_T("$if true {{hoge}}", "hoge", t);
        TEST_EQ_T("$if false{{hoge}}", "", t);
        TEST_EXC_T("$iffalse {{hoge}}", t);

        // $elseif
        TEST_EQ_T("$if true{{hoge}}$elseif true{{fuga}}", "hoge", t);
        TEST_EQ_T("$if true{{hoge}}$elseif undefined{{${undefined}}}", "hoge", t);
        TEST_EQ_T("$if false{{hoge}}$elseif true{{fuga}}", "fuga", t);
        TEST_EQ_T("$if false{{hoge}}$elseif true{{fuga}} $elseif undefined {{ fuga2 }}", "fuga", t);
        TEST_EQ_T("$if false{{hoge}}$elseif false{{fuga}} $elseif true {{ fuga2 }}", " fuga2 ", t);
        TEST_EXC_T("$if true{{hoge}}$elseiftrue{{fuga}}", t);
        TEST_EXC_T("$if true{{hoge}}$else if true{{fuga}}", t);

        // $else
        TEST_EQ_T("$if true {{hoge}}$else{{moke}}", "hoge", t);
        TEST_EQ_T("$if true {{hoge}} $else {{${undefined}}}", "hoge", t);
        TEST_EQ_T("$if false {{hoge}} $elseif true {{fuga}} $else{{moke}}", "fuga", t);
        TEST_EQ_T("$if false {{hoge}} $elseif false {{fuga}} $else{{moke}}", "moke", t);
        TEST_EXC_T("$if true {{}}$else${{}}", t);

        // confusing case
        TEST_EQ_T("$if true {{hoge}} ${true}", "hoge 1", t);
        TEST_EQ_T("$if true {{hoge}}${{", "hoge{{", t);
        TEST_EQ_T("$if true {{hoge}}  ", "hoge  ", t);
        TEST_EQ_T("$if true {{hoge}}  a", "hoge  a", t);
        TEST_EXC_T("$if true {{hoge}} $", t);
    }
    // test ForwardIterator
    {
        ginger::temple t;
        t["true"] = true;
        t["false"] = false;

        std::string str = "$if true {{hoge}} ${true}";
        std::list<char> input{str.begin(), str.end()};
        TEST_EQ_T(input, "hoge 1", t);
    }
    // test Generic Dictionary
    {
        std::unordered_map<std::string, ginger::object> t;
        t["value"] = 100;
        TEST_EQ_T("${value}", "100", t);
    }
    if (failed != 0) {
        std::cerr << "------- TEST FAILED --------" << std::endl;
        std::cerr << "FAILED COUNT: " << failed << std::endl;
        std::exit(1);
    }

    /*
    ginger::temple t;
    t["xs"] = std::vector<int>{ 1, 2, 3, 4 };
    t["value"] = "hoge-";
    t["x"] = true;
    t["p"] = false;
    t["q"] = false;
    t["ys"] = std::map<
                    std::string,
                    std::vector<
                        std::map<
                            std::string,
                            ginger::object>>> {
        { "values", {
            { { "test", true }, { "fuga", "aaaa" }, { "moke", "bbbb" } },
            { { "test", false }, { "fuga", "cccc" } },
        } },
    };

    std::string input = getStdin();
    try {
        ginger::parse(input, t);
    } catch (int line) {
        std::cerr << "error: " << line << std::endl;
    }
    */
}
