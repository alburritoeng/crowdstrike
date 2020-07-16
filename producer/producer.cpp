// producer.cpp 
#include <semaphore.h>
#include <sstream>
#include <iostream>
#include <istream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>
#include <thread> 
#include <mutex>  
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstring>
#include "producer.h"
#include "../utilities/messages.h"

namespace cwpp
{
    using namespace utilities;
    using namespace sharedmemory;

    producer::producer(std::string const& file, uint shared_buffers)
    {        
        numberOfSharedBuffers = shared_buffers;
        file_name = std::move(file);
        memoryMap = std::unique_ptr<sharedmemory::memorymapping>(new sharedmemory::memorymapping());       
        
        //create a file that can be used for generating a System V Token 
        helpers::write_ftok_file();
        
        //initialize semaphores
        initialize_semaphores(numberOfSharedBuffers);

        for(uint i = 0; i < numberOfSharedBuffers; i++)
        {
            mutex_map.insert({i, sharedmemory::memorymapping::init_get_shared_mutex(i)});
        }

        //initialize the number of shared buffers requested    
        initialize_shared_buffers(numberOfSharedBuffers);     
    }

    producer::~producer()
    {       
        //clean up/release memory:
        //detach ourselves from this shared memory
        clean_up_shared_buffers();

        //close all semaphores
        clean_up_semaphores();

        //delete the file created for FTOK key
        //helpers::delete_ftok_file(); //purposly commented out - Found issues with sharing        
    }
    
    void producer::initialize_semaphores(uint num)
    {       
        sem_unlink(empty_semaphore_name);
        sem_unlink(full_semaphore_name);
        semaphore_empty = sem_open(empty_semaphore_name,  O_CREAT, 0600, num);
        if (semaphore_empty == SEM_FAILED) 
        {
            std::cout << failed_to_get_semaphore << empty_semaphore_name << std::endl;           
            return;            
        }
        else
        {
            int val;
            sem_getvalue(semaphore_empty, &val);            
        }        

        semaphore_full = sem_open(full_semaphore_name,  O_CREAT, 0600, 0);
        if (semaphore_full == SEM_FAILED) 
        {
            std::cout << failed_to_get_semaphore << full_semaphore_name << std::endl;           
            return;            
        }
        else
        {
            int val;
            sem_getvalue(semaphore_full, &val);            
        }
    }

    void producer::write_to_shared_memory(std::string& argStr)
    {           
        sem_wait(semaphore_empty);                   
                
        // find a "read" memory map
        int entry = 0;
        auto current_mutex = mutex_map.find(entry);
        mapped_data* sharedmemory = nullptr;
        do
        {                           
            /************start critical section************/  
            // critical section here is we lock the shared (buffer) memory 
            // and read the read-state value                 

            pthread_mutex_lock(current_mutex->second);    
            sharedmemory = memoryMap->get_memory_map(entry);             
            // has this data been read?
            if(sharedmemory->state == data_rw_state::data_read || sharedmemory->state == data_rw_state::data_init)
            {
                // use this shared buffer as a candidate to write into
                // maintain lock on mutex
                break;
            }
            /************end critical section************/     
            // not this shared buffer, find another ready for writing 
            // release the mutex lock 
            pthread_mutex_unlock(current_mutex->second);
            // find another shared buffer that has been read to use for writting            
            if(++entry >= numberOfSharedBuffers)
            {
                entry = 0;
            }            

        } while (true);
                     

        /************start critical section************/  
        // critical section here is we write to the shared (buffer) memoruy
        // write something
        // we still have mutex
        mapped_data* data = new mapped_data();
        if(argStr.length() < buffer_size)
        {
            memcpy(data->sentences, argStr.data(), argStr.length());            
            data->sentences[argStr.length()+1] = 0;
            data->datalength = strlen(data->sentences);
            // writing
            data->state = data_rw_state::data_written;
            memoryMap->write_to_memory(data, sizeof(struct mapped_data), true);                    
        }
        delete data;

        /************end critical section************/     
        pthread_mutex_unlock(current_mutex->second);
                
        sem_post(semaphore_full);        
    }

    void producer::start_producing()
    {       
        //this is checked in main but added a second sanity check here
        if(!utilities::helpers::file_exists(file_name.c_str()))
        {
            std::cout << source_file_dne_error << std::endl;
            return;
        }

        std::ifstream fileToRead(file_name, std::ifstream::binary);
        
        std::string sentence;    
        int currentCount = 0;
        
        std::string currentRead;
        std::stringstream lines;
        const int newline = 1;
        while (std::getline(fileToRead, sentence))
        {            
            if(sentence.length() + newline + currentCount >= buffer_size)
            {
                //save the data to a memory map file
                currentRead = lines.str();            
                write_to_shared_memory( currentRead );

                currentCount = 0;                        
                lines.str("");
                currentRead.clear();            
            }   
            //append what we read, we still have room        
            lines << sentence << std::endl;
            
            //instructed to print everything we read to console
            std::cout << sentence << std::endl;
            
            currentCount += sentence.length() + newline; //the + 1 = the newline std::endl
            sentence.clear();        
        }

        //the last read, didn't quit hit our buffer_size, but need to send it out
        currentRead = lines.str();    
        
        write_to_shared_memory( currentRead );

        currentCount = 0;                        
        lines.str("");
        currentRead.clear();      

        for (auto it = mutex_map.begin();it != mutex_map.end(); ++it)
        {            
            mutex_map.erase(it);
        }

        clean_up_shared_buffers();
        fileToRead.close();
        for(int i =0; i<numberOfSharedBuffers; i++)
        {            
            memoryMap->destroy_shared_memory(i);
        }
    }

    void producer::clean_up_shared_buffers()
    {        
        for(int i = 0; i < numberOfSharedBuffers; i++)
        {
            auto buffer = memoryMap->get_memory_map(i);
            //could probably be handled by a unique_ptr or destructor to cleean up/detach        
            int status = munmap(buffer, sizeof(pthread_mutex_t));  /* Unmap the page */

            std::ostringstream str_mutex_name_num;
            str_mutex_name_num << mutex_mmap_name << i;
            std::string fileName(str_mutex_name_num.str());
        
            status = shm_unlink(fileName.c_str());     /* Unlink shared-memory object */                       
        }
    }

    void producer::clean_up_semaphores()
    {        
        sem_unlink("empty");
        sem_unlink("full");
        sem_close(semaphore_empty);
        sem_close(semaphore_full);
    }

    void producer::initialize_shared_buffers(uint sharedBuffersCount)
    {
        for(uint i =0; i <sharedBuffersCount; i++)
        {            
            memoryMap->create_memory_map(buffer_size, i);         
        }        
    }

    std::map<uint, pthread_mutex_t*> producer::initalize_mutex(int numSharedBuffers)
    {
        for(int i = 0; i < numSharedBuffers; i++)
        {
            mutex_map.insert({i, sharedmemory::memorymapping::init_get_shared_mutex(i)});
        }        
    }
}