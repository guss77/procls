#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>
#include <string>
#include <algorithm>
#include <exception>
#include <unistd.h> // for sleep()

#include "helpers.h"
#include "processes.h"
#include "process.h"

const std::array<const char *, 4> HEADERS{ "name", "memsize", "%CPU", "owner" };

typedef std::array<std::string, HEADERS.size()> row_data;
typedef std::array<size_t, HEADERS.size()> column_sizes;

const std::string COL_DELIM("\t");

void max_sizes(column_sizes &curmax, const row_data &strings_to_check) {
    for (size_t i = 0; i < curmax.size(); ++i)
        curmax[i] = std::max(curmax[i], strings_to_check[i].size());
}

std::string output_row(column_sizes column_size, const row_data &row_data, std::string delim) {
    std::ostringstream s;
    s << std::left << std::setw(column_size.front()) << row_data.front();
    for (size_t i = 1; i < column_size.size(); ++i) {
        s << delim << std::setw(column_size[i]) << row_data[i];
    }
    return s.str();
}

int main() {
    row_data headers;
    for (size_t i = 0; i < HEADERS.size(); ++i)
        headers[i] = HEADERS[i];
    std::vector<row_data> table = { headers };

    try {
        Processes ps;
        sleep(2);
        ps.update_processes();
        ps.for_each([&table](const Process &p){
            table.push_back({
                p.name(),
                std::to_string(p.memsize()) + "K",
                std::to_string(p.cpu()*100, "%02.0f") + "%",
                p.owner_name()
                });
        });

        column_sizes column_size;
        for (size_t i = 0; i < column_size.size(); ++i)
        	column_size[i] = 0;

        std::for_each(table.cbegin(), table.cend(), [&column_size](const row_data& row){
            max_sizes(column_size, row);
        });

        std::for_each(table.cbegin(), table.cend(), [&column_size](const row_data &procrow){
            std::cout << output_row(column_size, procrow, COL_DELIM) << std::endl;
        });
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }
}
