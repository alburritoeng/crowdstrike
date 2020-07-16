#include "utilities.h"      
#include "memorymapping.h"
#include <cstring>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <stdio.h> 
#include <map>
#include <iostream> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "messages.h"
#include <sstream>
#include <ostream>
#include <string.h>
#include <unistd.h>

namespace sharedmemory
{  
  memorymapping::~memorymapping()
  {
    detach_memory();
  }

  memorymapping::memorymapping( const memorymapping &obj)
  {      
      _data = new mapped_data();
      *_data = *obj._data; // copy the value
      keys = obj.keys;
  }

  void memorymapping::operator=(const memorymapping &obj )
  {
      _data = new mapped_data();
      *_data = *obj._data;
      keys = obj.keys;
  }

  bool memorymapping::create_memory_map(int size, int sharedBuffer)
  {   
      std::string fileName(utilities::helpers::get_file_name());

      key_t key = ftok(fileName.c_str(), sharedBuffer);
      if(key == -1) 
      {
        std::cout << key_generation_failed << std::strerror(errno) << std::endl;
        return false;
      }
      
      keys.insert({sharedBuffer, key});
      
      int shmid = shmget(key,buffer_size, permissions|IPC_CREAT); 
      if(shmid < 0)
      {
          std::cout << existing_key_found_found << std::endl;
          return false;
      }
      
      _data = reinterpret_cast<mapped_data*>(shmat(shmid,(void*)0,0));

      // make a shared mutex for 
      init_get_shared_mutex(sharedBuffer);

      if (_data == reinterpret_cast<void*>(-1) )
      {
        std::cout << attach_to_shared_mem_failed << std::strerror(errno) << std::endl;
        return false;
      }
    
    //set memory state to init state, the producer (when mutex is received) sets to written, consumer sets to read, then we can overwrite in producer
     _data->state = data_rw_state::data_init;
    
    return true;   
  }

  bool memorymapping::write_to_memory(mapped_data* ptr, size_t size, bool canWrite)
  {
    if(canWrite)
    {
      memset(_data, 0x0, sizeof(struct mapped_data) );
      memcpy(_data, ptr, sizeof(struct mapped_data) );
    }
  }

  mapped_data* memorymapping::get_memory_map(int entry)
  {
      key_t key;
      auto it = keys.find(entry);
   
      if (it != keys.end()) //do we already have a key, producer kills token file on terminate
      {
        key = it->second;
      }
      else //create it & store it
      {
        key = ftok(utilities::helpers::get_file_name().c_str(), entry);
        if(key == -1) 
        {
          std::cout << key_generation_failed << " for entry " << entry << std::strerror(errno) << std::endl;
          return nullptr;
        }
        keys.insert({entry, key});
      }

      int shm_id=-1;
      // try to get an existing using previous key
      if ((shm_id = shmget(key, buffer_size, permissions)) == -1)
      {
          std::cout << existing_key_found_found << std::endl;
          return nullptr;
      }

      // shmat to attach to shared memory 
      _data =  reinterpret_cast<mapped_data*>(shmat(shm_id,NULL,0)); 
      if (_data == reinterpret_cast<void *>(-1) )
      {
          std::cout << attach_to_shared_mem_failed << std::strerror(errno) << std::endl;
          return nullptr;
      }
      
      return _data;
  }

  void memorymapping::destroy_shared_memory(int entry)
  {
      key_t key;
      auto it = keys.find(entry);
   
      if (it != keys.end()) //do we already have a key, producer kills token file on terminate
      {
        key = it->second;
      }
      else //create it & store it
      {
        key = ftok(utilities::helpers::get_file_name().c_str(), entry);
        if(key == -1) 
        {
          std::cout << key_generation_failed << std::strerror(errno) << std::endl;
          return;
        }
        keys.insert({entry, key});
      }

      int shm_id=-1;
      // try to get an existing using previous key
      if ((shm_id = shmget(key, buffer_size, permissions)) == -1)
      {
          std::cout << existing_key_found_found << std::endl;
          return;
      }

      shmctl(shm_id, IPC_RMID, 0);
  }

  pthread_mutex_t* memorymapping::init_get_shared_mutex(uint num) 
  {
      bool succeess;
      pthread_mutex_t* mutex;
      std::ostringstream str_mutex_name_num;
      str_mutex_name_num << mutex_mmap_name << num;
      std::string fileName(str_mutex_name_num.str());

      mutex = static_cast<pthread_mutex_t *>(mmap(NULL, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0));

      pthread_mutexattr_t mutexattr;
      auto rc = pthread_mutexattr_init(&mutexattr);
      if (rc != 0)
      {
        std::cout << "init_get_shared_mutex: " << "pthread_mutexattr_init failed" << std::endl;
        return nullptr;
      }

      rc = pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
      if (rc != 0)
      {
           std::cout << "init_get_shared_mutex: " << "pthread_mutexattr_setpshared failed" << std::endl;
           return nullptr;
      }
      pthread_mutex_init(mutex, &mutexattr);
      if (rc != 0)
      {
          std::cout << "init_get_shared_mutex: " <<"pthread_mutex_init failed" << std::endl;
          return nullptr;
      }      
      
      return std::move(mutex);

  }

  void memorymapping::detach_memory()
  {
    if(_data != nullptr)
    {
      shmdt(_data);      
    } 
  }
}