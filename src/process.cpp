#include "process.h"

#include <algorithm>
#include <system_error>
#include <pwd.h> // for getpwuid
#include <unistd.h> // for clock ticks

#include "helpers.h"

static const unsigned int CLOCK_TICKS_SEC = sysconf(_SC_CLK_TCK);

Process::Process(pid_t pid) : _pid(pid), _name(),
		_owner(get_file_owner("/proc/" + std::to_string(pid))), _memsize(0), _cpu(0),
		_last_measure(0)
{
    auto statline = string_split(readfile("/proc/" + std::to_string(pid) + "/stat"),' ');
    auto stats = statline.begin();
    ++stats; // skip pid
    auto tempname = *stats;
    while (tempname[tempname.size()-1] != ')') {
        ++stats;
        tempname += *stats;
    }
    _name = tempname.substr(1, tempname.size() - 2);

    calc_mem();
    calc_cpu();
}

Process::Process(const Process&& other) : _pid(other._pid), _name(other._name),
		_owner(other._owner), _memsize(other._memsize), _cpu(other._cpu),
		_last_measure(other._last_measure), _last_measure_time(other._last_measure_time)
{
}

Process& Process::operator=(const Process&& rhs)
{
    if (this == &rhs || this->_pid == rhs._pid) return *this; // handle self assignment
    //assignment operator
    _pid = rhs._pid;
    _name = rhs._name;
    _owner = rhs._owner;
    _memsize = rhs._memsize;
    _cpu = rhs._cpu;
    _last_measure = rhs._last_measure;
    _last_measure_time = rhs._last_measure_time;
    return *this;
}

const std::string Process::owner_name() const {
    errno = 0;
    auto passwd(getpwuid(owner()));
    if (!passwd)
        throw std::system_error(errno, std::system_category(), "Failed to get user information for uid " + std::to_string(owner()));
    return std::string(passwd->pw_name);
}


void Process::calc_mem() {
    auto memblocks = string_split(readfile("/proc/" + std::to_string(_pid) + "/maps"),'\n');
    if (memblocks.size() <= 0) // failed to read process maps - we don't have permissions?
    	return;
    // find the column where the block pathname is listed
    auto memblockfileline = std::find_if(memblocks.cbegin(), memblocks.cend(), [](const std::string& memblock){
        return memblock.find('/') != std::string::npos;
    });
    std::string memblockfileline_s = *memblockfileline;
    auto pathstart = memblockfileline_s.find('/');
    // examine memory blocks
    std::for_each(memblocks.cbegin(), memblocks.cend(), [this, pathstart](const std::string& memblock){
        auto firstsp = memblock.find(' '), secondsp = memblock.find(' ', firstsp + 1);
        auto addrs = string_split(memblock.substr(0, firstsp), '-');
        auto flags = memblock.substr(firstsp + 1, secondsp - firstsp);
        auto block = memblock.substr(memblock.size() > pathstart ? pathstart : memblock.size()); // some memory blocks are anonymous

        // skip blocks that I don't want to cound
        if (flags.find('s') != std::string::npos || // shared memeory
            block.find("[vvar]") != std::string::npos || // kernel vars
            block.find("[vdso]") != std::string::npos || // virtual kernel so
            block.find("[vsyscall]")  != std::string::npos) // kernel syscal optimization
            return;

        _memsize += std::stol(addrs.back(), 0, 16) - std::stol(addrs.front(), 0, 16);
    });
}

void Process::calc_cpu() {
	auto statline = readfile("/proc/" + std::to_string(_pid) + "/stat");
	auto stats = string_split(statline.substr(statline.find(')')+2),' ');
	auto measure = std::stoul(stats[11]) + std::stoul(stats[12]);
	auto measure_time = std::chrono::high_resolution_clock::now();

	if (_last_measure) {
		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(measure_time - _last_measure_time);
		auto cputime = measure - _last_measure;
		_cpu = float(cputime * 1000 / CLOCK_TICKS_SEC) / dur.count();
	}

	_last_measure = measure;
	_last_measure_time = measure_time;
}
