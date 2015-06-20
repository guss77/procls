#ifndef HELPERS_H_INCLUDED
#define HELPERS_H_INCLUDED

#include <string>
#include <vector>

namespace std { // add some std helpers I wish were implemented
    std::string to_string(float value, const char* format = "%f");
}

std::vector<std::string> string_split(const std::string &s, char delim);

uid_t get_file_owner(std::string fname);
std::string readfile(std::string fname);

#endif // HELPERS_H_INCLUDED
