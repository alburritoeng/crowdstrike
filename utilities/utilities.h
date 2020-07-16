#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>

namespace utilities
{
    class helpers
    {
        public:
            static bool file_exists(const char* fp);
            static std::string get_file_name();
            static void write_ftok_file();
            static void delete_ftok_file();
    };
}

#endif