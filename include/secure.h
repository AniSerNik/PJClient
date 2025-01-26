#ifndef SECURE_H
#define SECURE_H

/** 
  * @defgroup Secure module
  * @brief TODO
  * @details TODO
  * @{
*/

#include "mbedtls/md.h" //for hashing

#define HASH_MAX_SIZE MBEDTLS_MD_MAX_SIZE
#define HASH_STR_MAX_SIZE MBEDTLS_MD_MAX_SIZE * 2

void sc_get_hash(const char* str, uint8_t* hash_out);

void sc_get_str_hash(const char* str, char* str_out);

void sc_hash_to_str(char* str, uint8_t* hash);

/*! @} */

#endif /* SECURE_H */