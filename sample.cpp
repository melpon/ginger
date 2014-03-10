#include "ginger.h"
#include <iostream>
#include <sstream>
#include <ios>
#include <vector>
#include <map>
#include <string>

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

    std::stringstream ss;
    ss << std::cin.rdbuf();
    try {
        ginger::parse(ss.str(), t);
    } catch (ginger::parse_error& error) {
        std::cerr << error.long_error() << std::endl;
    }
}
