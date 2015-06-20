#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <chrono>
#include <sys/types.h>

class Process
{
    public:
        Process(pid_t pid);
        Process(const Process&& other);
        Process& operator=(const Process&& other);

        const std::string name() const { return _name; }
        const uid_t owner() const { return _owner; }
        const int memsize() const { return _memsize / 1024; }
        const float cpu() const { return _cpu; }
        const std::string owner_name() const;
        void calc_cpu();

    protected:
    private:
        void calc_mem();

        pid_t _pid;
        std::string _name;
        uid_t _owner;
        int _memsize; //in KBytes
        float _cpu; // current CPU usage as percentage of a single CPU

        unsigned long _last_measure;
        std::chrono::time_point<std::chrono::high_resolution_clock> _last_measure_time;
};

#endif // PROCESS_H
