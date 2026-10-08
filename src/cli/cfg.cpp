#include "cfg.hpp"

#include <set>
#include <stdexcept>
#include <vector>

#include "../common/errors.hpp"
#include "../common/fileIO.hpp"
#include "../common/stringTools.hpp"

// parses {{uint} {B/KB/MB/GB} | unlimited} into an amount of bytes. can assume the input is trimmed
// already. if allowUnlimited is false, will throw upon encountering this value
ssize_t parseSize(std::string s, bool allowUnlimited) {
    s = lowercase(s);
    if (s == "unlimited") {
        if (allowUnlimited) {
            return -1;
        }
        throw InputError("\"Unlimited\" not allowed here");
    }
    int64_t len = s.length();
    int64_t mult = 1;
    int64_t suffLen = 0;
    if (len > 1 && s[len - 1] == 'b') {
        suffLen = 1;
        if (len > 2) {
            if (s[len - 2] == 'k') {
                suffLen = 2;
                mult = 1024;
            } else if (s[len - 2] == 'm') {
                suffLen = 2;
                mult = 1024 * 1024;
            } else if (s[len - 2] == 'g') {
                suffLen = 2;
                mult = 1024 * 1024 * 1024;
            }
        }
    }
    if (suffLen == 0) {
        throw InputError("Invalid size - " + s);
    }
    int64_t val;
    s = s.substr(0, s.length() - suffLen);
    try {
        val = std::stoll(s);
    } catch (const std::invalid_argument& e) {
        throw InputError("Value cannot be parsed as a uint");
    } catch (const std::out_of_range& e) {
        throw InputError("Value out of range (unsigned long)");
    }
    int64_t orig = val;

    val *= mult;
    if (val < orig) {
        throw InputError("Overflow! The amount of bytes does not fit into uint64_t max / 2");
    }
    return val;
}

ssize_t parseInt(const std::string& s) {
    if (s == "unlimited") {
        return -1;
    }
    try {
        return std::stoul(s);
    } catch (const std::invalid_argument& err) {
        throw InputError("Value cannot be parsed as a uint");
    } catch (const std::out_of_range& err) {
        throw InputError("Value out of range (unsigned long)");
    }
}

bool parseBool(const std::string& s) {
    if (s == "true")
        return true;
    if (s == "false")
        return false;
    throw InputError("Invalid value - expected true or false");
}

std::map<std::string, std::string> loadConfig(const std::string& conf) {
    std::map<std::string, std::string> m;
    std::vector<std::string> forbiddenOptions = {"target", "help", "disclaimer", "config"};
    std::vector<std::string> lines = split(readFileAsString(conf), '\n', true);
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string l =
            trim(removeAfterSuffix(lines[i], "#"));  // will also cut on hash symbols inside strings
        if (l.empty())
            continue;
        std::string key;
        std::string value;
        size_t eq = l.find('=');
        if (eq == std::string::npos) {
            throw InputError("Invalid config - \"=\" not found at line " + std::to_string(i + 1));
        }
        key = trim(l.substr(0, eq));
        if (key.length() > 2 && (key[0] == '-') && (key[1] == '-')) {
            key = key.substr(2);
        }

        std::string invOpt;
        for (const auto& s : forbiddenOptions) {
            if (s == key) {
                invOpt = s;
            }
        }

        if (invOpt != "") {
            throw InputError("Invalid option in " + conf + " - " + invOpt);
        }

        value = trim(l.substr(eq + 1));
        m[key] = value;
    }

    return m;
}

void fillConfig(VMConfig& conf, std::map<std::string, std::string>& args) {
    std::set<std::string> names = {"max_cycles", "max_stack", "max_heap", "out",    "in",
                                   "args",       "verbose",   "config",   "target", "ram_size"};

    conf.max_cycles = parseInt(args["max_cycles"]);
    conf.max_stack = parseSize(args["max_stack"], true);
    conf.max_heap = parseSize(args["max_heap"], true);
    conf.ram_size = parseSize(args["ram_size"], false);

    auto& out = args["out"];
    auto& in = args["in"];
    if (!(out.empty()))
        conf.out = out;

    if (!(in.empty()))
        conf.in = in;

    conf.args = args["args"];

    conf.verbose = parseBool(args["verbose"]);

    for (const auto& p : args) {
        if (!(names.contains(p.first)))
            throw InputError("Unknown config option - " + p.first);
    }
}
