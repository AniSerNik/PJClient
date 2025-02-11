#ifndef LORA_H
#define LORA_H

/** 
  * @defgroup lora LoRA
  * @brief TODO
  * @details TODO
  * @{
*/


#define LORA_SF_DEFAULT 9
#define LORA_BW_DEFAULT 125000
#define LORA_CR_DEFAULT 5

typedef struct loraConfigAddress {
  uint8_t clientAddress = 0;
  uint8_t gatewayAddress = 0;
} loraConfigAddress;

void loraSetGatewayAddress(uint8_t addr);

void loraSetClientAddress(uint8_t addr);

bool loraInit();

bool loraSendKeys();

bool loraSendData();

bool loraSendClearData();

bool loraSendFull();

void loraSleep();

bool _loraSendAndGet();

void _loraPrintBuf(uint8_t *buf);

/*! @} */

#endif /* LORA_H */