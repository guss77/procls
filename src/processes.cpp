#include "processes.h"

#include <algorithm>
#include <functional>
#include <system_error>
#include <cerrno>
#include <dirent.h>

class DirectoryReader {
    public:
        DirectoryReader(const char* path) : _path(path), _next(nullptr), _dir(opendir(path)) {
            if (!_dir)
                throw std::system_error(errno, std::system_category(), "Failed to open " + _path + " for reading!");
        }
        ~DirectoryReader() {
            closedir(_dir);
        }
        bool has_next() {
            errno = 0;
            _next = readdir(_dir);
            if (!_next && errno)
                throw std::system_error(errno, std::system_category(), "Error reading " + _path + " directory!");
            return _next;
        }
        std::string next() {
            if (!_next)
                throw std::system_error(0, std::system_category(), "No more files in " + _path + "!");
            return _next->d_name;
        }
    private:
        DirectoryReader(const DirectoryReader&); // non-copyable
        DirectoryReader& operator=(const DirectoryReader&); // non-copyable
        std::string _path;
        struct dirent* _next;
        DIR* _dir;
};

Processes::Processes() : _procs()
{
	std::vector<pid_t> pids;
    for (DirectoryReader procfs("/proc"); procfs.has_next();) {
        auto procfile = procfs.next();
        if (procfile[0] < '0' || procfile[0] > '9') continue; // skip non pid members
        pids.push_back(std::stoi(procfile));
    }

    // scan for process data
    std::for_each(pids.cbegin(), pids.cend(), [this](const pid_t &pid){
        try {
        	_procs.push_back(Process(pid));
        } catch (const std::system_error &e) {
            // silently ignore - the process either went away between when we scanned the list and now, or we don't have permissions to its data
        }
    });
}

void Processes::for_each(std::function<void(const Process&)> lambda) {
    std::for_each(_procs.cbegin(), _procs.cend(), [&lambda](const Process &p){
		lambda(p);
    });
}

void Processes::update_processes() {
	std::for_each(_procs.begin(), _procs.end(), [](Process &proc){
		proc.calc_cpu();
	});
}
