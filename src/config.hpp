#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <vector>

struct WLConfig
{
    std::string cmd;
    std::string output; //output file
    std::vector<std::string> files;
    std::vector<std::string> lib; // -l
    std::vector<std::string> inc; // -i
    std::vector<std::string> libdirs; // -L
    std::vector<std::string> incdirs; // -I

    std::string tempName;

    bool link;
    bool debug;

    WLConfig()
    {
        link = true;
        debug = false;
    }
};

#endif
