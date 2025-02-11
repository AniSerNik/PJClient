#include "src/include/common.h"
#include "src/include/secure.h"

void sc_get_hash(const char* str, uint8_t* hash_out) {
  if(str == NULL || hash_out == NULL)
    return;
  uint8_t shaResult[HASH_MAX_SIZE];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA512;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *) str, strlen(str));
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);
  memcpy(hash_out, shaResult, sizeof(shaResult));
}

void sc_get_str_hash(const char* str, char* str_out) {
  uint8_t hash[HASH_MAX_SIZE] = {0};
  sc_get_hash(str, hash);
  sc_hash_to_str(str_out, hash);
}

void sc_hash_to_str(char* str, uint8_t* hash) {
  if(str == NULL || hash == NULL)
    return;
  char strResult[HASH_STR_MAX_SIZE * 2];
  for(int i = 0; i < HASH_MAX_SIZE; i++){
      char str[3];
      sprintf(str, "%02x", (int)hash[i]);
      strcat(strResult, str);
  }
  memcpy(str, strResult, sizeof(strResult));
}

/*
  char str[HASH_STR_MAX_SIZE] = {0};
  sc_get_str_hash("abc", str);
*/