#include "helpers.h"

#include <fstream>
#include <sstream>
#include <cerrno>
#include <system_error>

#include <pwd.h> // for getpwuid
#include <sys/stat.h> // for stat

namespace std { // add some std helpers I wish were implemented

    std::string to_string(float value, const char* format) {
        std::vector<char> out(std::snprintf(nullptr, std::size_t(0), format, value) + 1, '\0'); // leave room for terminating null
        std::snprintf(out.data(), out.size(), format, value);
        return std::string(out.data()); // data is null terminated, no need for size
    }

};

std::vector<std::string> string_split(const std::string &s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    for (std::string token; std::getline(ss, token, delim); )
        out.push_back(token);
    return out;
}

uid_t get_file_owner(std::string fname) {
    struct stat finfo;
    errno = 0;
    if (stat(fname.c_str(), &finfo))
        throw std::system_error(errno, std::system_category(), "Error reading file info '" + fname + "'!");
    return finfo.st_uid;
}

std::string readfile(std::string fname) {
    errno = 0;
    std::ifstream inf(fname, std::ios::in | std::ios::binary);
    if (!inf)
        throw std::system_error(errno, std::system_category(), "Error reading file '" + fname + "'!");

    std::ostringstream s;
    s << inf.rdbuf();
    return s.str();
}
