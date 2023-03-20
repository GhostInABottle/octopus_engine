#ifndef HPP_PATH_INFO
#define HPP_PATH_INFO

#include <string>
#include <ctime>

// Basic file/dir info
struct Path_Info {
    std::string name;
    bool is_regular;
    bool is_directory;
    unsigned long long timestamp;
    std::tm calendar_time;
};

#endif
