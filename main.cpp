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

int main() {
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
}
