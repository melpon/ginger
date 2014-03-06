#include "ginger.h"
#include <iostream>
#include <ios>
#include <vector>
#include <map>
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
    std::vector<std::map<std::string, ginger::object>> xs;
    xs.push_back({
        { "enable", true },
        { "url", "http://example.com" },
        { "value", "Example" },
    });
    xs.push_back({
        { "enable", false },
        { "url", "undefined" },
        { "value", "Test" },
    });
    xs.push_back({
        { "enable", true },
        { "url", "http://google.com" },
        { "value", "Google" },
    });

    std::map<std::string, ginger::object> t;
    t["title"] = "Sample Site";
    t["xs"] = xs;

    std::string input = getStdin();
    try {
        ginger::parse(input, t);
    } catch (ginger::parse_error& error) {
        std::cerr << error.long_error() << std::endl;
    }
}
