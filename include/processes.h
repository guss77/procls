#ifndef PROCESSES_H
#define PROCESSES_H

#include <functional>
#include <vector>
#include <sys/types.h>

#include "process.h"

class Processes
{
    public:
        Processes();
        void for_each(std::function<void(const Process&)> lambda);
        void update_processes();
    protected:
    private:
        std::vector<Process> _procs;
};

#endif // PROCESSES_H
