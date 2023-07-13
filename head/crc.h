#ifndef CRC_H
#define CRC_H
#include <stdio.h>
#include <stdint.h>
#include "protocol.h"

#define CRC32_POLYNOMIAL 0xEDB88320
//常见的多项式0xEDB88320、0x04C11DB7、0x1EDC6F41

// uint32_t check_crc32(uint8_t *data, size_t length) {
//     uint32_t crc = 0xFFFFFFFF;
    
//     for (size_t i = 0; i < length; i++) {
//         crc ^= data[i];
        
//         for (int j = 0; j < 8; j++) {
//             if (crc & 1) {
//                 crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
//             } else {
//                 crc = (crc >> 1);
//             }
//         }
//     }
    
//     return crc;
// }

uint32_t calculate_crc32(Message *message) {
    uint32_t crc = 0xFFFFFFFF;
    // 将CRC校验字段置零
    message->header.crc32 = 0;

    // 数据包长度
    size_t length = sizeof(*message);

    // 转换为字节数组
    uint8_t *packet_bytes = (uint8_t *)message;

    // 计算CRC32校验值，不包括CRC本身
    for (size_t i = 0; i < length; i++) {
        crc ^= packet_bytes[i];
        
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc = (crc >> 1);
            }
        }
    }
    
    return crc;

}

#endif