#include <iostream>
#include <map>

#include "../common/errors.hpp"
#include "../common/stringTools.hpp"
#include "cfg.hpp"

std::map<std::string, std::string> parseArgs(int argc, char** argv) {
    std::map<std::string, std::string> m;
    bool targetFound = false;
    for (size_t i = 1; i < argc; ++i) {
        if (startswith(argv[i], "--")) {
            std::string arg = (argv[i] + 2);
            auto eq = arg.find('=');
            std::string key;
            std::string val;
            if (eq != std::string::npos) {
                key = arg.substr(0, eq);
                val = arg.substr(eq);
            } else {
                key = arg;
            }
            if (key == "target") {
                throw InputError("Invalid option - \"--target\"");
            }
            m[key] = arg;
        } else {
            if (!targetFound) {
                targetFound = true;
                m["target"] = argv[i];
            } else {
                throw InputError("Invalid argument - \"" + std::string(argv[i]) +
                                 "\". Target already specified - " + m["target"]);
            }
        }
    }

    return m;
}

int main(int argc, char** argv) {
    std::map<std::string, std::string> args;
    try {
        args = parseArgs(argc, argv);
    } catch (const InputError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    if (args.size() == 0) {
        std::cout << "No input provided. Use --help for usage information." << std::endl;
        return 1;
    }

    if (args.contains("help")) {
        std::cout
            << "Iremu - Isolated Runtime EMUlator\nA program emulator designed for safe execution, "
               "testing, inspection, and experimentation. Iremu emulates a CPU with a minimal "
               "operating system, running the programs in an isolated environment, potentially "
               "enabling you to "
               "run anything safely.\nRun \"./iremu "
               "--disclaimer\" to show the disclaimer\n\nUsage - "
               "./iremu {executable} "
               "{options}\n\nConfiguration options:\n"
            << "\n--max_cycles={uint | \"unlimited\"} - Max CPU cycles to execute before "
               "termination. If set to unlimited, Iremu will run until the executable exits on its "
               "own."
            << "\n--max_stack={uint B/KB/MB/GB | \"unlimited\"} - Max stack memory size. The "
               "program will be terminated if it exceeds the limit. If set to unlimited, the "
               "executable will be able to allocate as much as it wants. (examples: "
               "--max_stack=64MB, --max_stack=unlimited)"
            << "\n--max_heap={uint B/KB/MB/GB | \"unlimited\"} - Same as max_stack, but for heap "
               "memory."
            << "\n--stdout={filename | default} - Where to put the console output of the "
               "executable. If default, will be printed out in the console."
            << "\n--stdin={filename | default} - Where to take the console input for the "
               "executable from. If default, will be taken from the console."
            << "\n--args=\"{string}\" - Arguments to be passed to the executable, must be "
               "placed between two (backslash) unescaped double quotes. (example: ./iremu "
               "./capitalize --args=\"hello world\")"
            << "\n--verbose={true | false} - Enable diagnostic messages about "
               "emulator events. "
            << "\n--config={filename} - Instead of specifying the options via the command line on "
               "every run, you can create a config file Iremu will take the settings from. Command "
               "line still gets the priority, so all options in the config will be overridden if "
               "specified."

            << std::endl;
        if (args.size() > 1) {
            std::cout << "\n--help was specified, so other arguments were ignored" << std::endl;
        }
        return 0;
    }
    if (args.contains("disclaimer")) {
        std::cout
            << "Disclaimer: Iremu might still be buggy and incomplete. It aims to run "
               "programs in an isolated environment, but it is not guaranteed to protect your "
               "device. Please, don't run actually dangerous software in it."
            << std::endl;
        if (args.size() > 1) {
            std::cout << "\n--disclaimer was specified, so other arguments were ignored"
                      << std::endl;
        }
        return 0;
    }

    return 0;
}