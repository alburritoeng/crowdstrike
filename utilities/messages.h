// message constants for std::cout
#ifndef MESSAGES_H
#define MESSAGES_H

constexpr const char file_successfully_deleted[]            = "File successfully deleted";
constexpr const char error_deleting_file[]                  = "Error deleting file";
constexpr const char untrusted_producer_consumer_blurp[]    = "Untrusted producer consumer task";
constexpr const char key_generation_failed[]                = "key generation failed with errno =";
constexpr const char attach_to_shared_mem_failed[]          = "attach to shared memory failed with errno = ";
constexpr const char existing_key_found_found[]             = "could not get existing map with key";
constexpr const char source_file_dne_error[]                = "source file does not exists";
constexpr const char error_reading_shared_mem[]             = "error reading data";
constexpr const char failed_to_get_semaphore[]              = "failed to get semaphore ";

#endif