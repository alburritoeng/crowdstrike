#ifndef MEMORYMAPPING_H
#define MEMORYMAPPING_H

#include "constants.h"
#include <map>
#include <semaphore.h>


enum data_rw_state 
{
  data_init = 0, 
  data_written = 1,
  data_read = 2 
}; 

struct mapped_data
{
  unsigned long datalength;
  data_rw_state state;  
  char sentences[buffer_size];  
};

namespace sharedmemory
{
  class memorymapping
  {
      private:
          
          mapped_data* _data;
          std::map<int, key_t> keys;
          
      protected:
          void detach_memory();          
      public:    
          //rule of 3
          memorymapping(){} //constructor
          ~memorymapping(); //destructor
          memorymapping( const memorymapping &obj);  //copy constructor
          void operator = (const memorymapping &obj ); //copy assign operator

          //use this class to create mutexes for each shared buffer
          static pthread_mutex_t* init_get_shared_mutex(uint num);

          bool create_memory_map(int size, int sharedBuffer);
          mapped_data* get_memory_map(int entry);        
          bool write_to_memory(mapped_data* ptr, size_t size, bool canWrite);                    
          void destroy_shared_memory(int entry);
  };
}

#endif