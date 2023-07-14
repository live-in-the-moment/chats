#ifndef CRC_H
#define CRC_H
#include <stdio.h>
#include <stdint.h>
#include "protocol.h"

#define CRC32_POLYNOMIAL 0xEDB88320
//常见的多项式0xEDB88320、0x04C11DB7、0x1EDC6F41


uint32_t calculate_crc32(Message *message);

#endif