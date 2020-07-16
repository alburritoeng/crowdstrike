#include "utilities.h"
#include <sys/stat.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include "constants.h"
#include "messages.h"

namespace utilities
{
    bool helpers::file_exists (const char *filename) 
    {
        struct stat   buffer;   
        return (stat (filename, &buffer) == 0);
    }

    std::string helpers::get_file_name()
    {
        std::ostringstream s;
        s << var_temp << ftok_file_name << dot_txt;
        std::string fileName(s.str());
        return std::move(fileName); //avoid a copy
    }

    void helpers::write_ftok_file()
    {    
        std::ofstream fTokFile;
        std::string fileName = helpers::get_file_name();
        if(helpers::file_exists(fileName.c_str()))
        {
            return;
        }
        fTokFile.open (fileName);
        fTokFile << untrusted_producer_consumer_blurp;
        fTokFile.close();
    }

    void helpers::delete_ftok_file()
    {
        std::string fileName = helpers::get_file_name();
        if( remove( fileName.c_str() ) != 0 )
        {
            std::cout << error_deleting_file << std::endl;
        }
        else
        {
            std::cout << file_successfully_deleted << std::endl; 
        }
    }
}