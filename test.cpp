#include "ginger.h"
#include <iostream>
#include <ios>
#include <vector>
#include <string>

std::string getStdin() {
    std::string output;
    std::string input;
    std::cin >> std::noskipws;
    while (std::getline(std::cin, input)) {
        output += input + '\n';
    }
    return output;
}

void print_error(int line, std::string input, std::string expected, std::string actual, std::string error = "") {
    std::cerr << "------------- TEST ERROR (" << line << ") ---------------" << std::endl;
    std::cerr << "input: " << input << std::endl;
    std::cerr << "expected: " << expected << std::endl;
    std::cerr << "actual: " << actual << std::endl;
    std::cerr << "error: " <<  error << std::endl;
}
void test_eq(std::string input, std::string expected, ginger::temple* p, int line) {
    try {
        std::stringstream ss;
        std::stringstream ess;
        ginger::parse(input, p ? *p : ginger::temple(), ginger::from_ios(ss), ginger::from_ios(ess));
        auto actual = ss.str();
        if (actual != expected) {
            auto error = ess.str();
            print_error(line, input, expected, actual, error);
        }
    } catch (int gline) {
        print_error(line, input, expected, "EXCEPTION (" + std::to_string(gline) + ")");
    } catch (...) {
        print_error(line, input, expected, "UNKNOWN EXCEPTION");
    }
}
#define TEST_EQ(input, expected) test_eq(input, expected, nullptr, __LINE__)
#define TEST_EQ_T(input, expected, t) test_eq(input, expected, &t, __LINE__)

int main() {
    TEST_EQ("Hello", "Hello");
    TEST_EQ("${", "{");
    TEST_EQ("${{", "{{");

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
