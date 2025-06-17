#ifndef _IDIP_PACKET_H_
#define _IDIP_PACKET_H_

#include <stdint.h>
#include <stdlib.h>

// 封装函数
uint8_t *idip_encapsulate(const uint8_t *payload, size_t len,
                          uint32_t src_id, uint32_t dst_id,
                          size_t *out_len);

// 解封装函数：从包中解析 src_id、dst_id 和 payload 指针
int idip_decapsulate(const uint8_t *packet, size_t len,
                     uint32_t *src_id, uint32_t *dst_id,
                     const uint8_t **payload_out, size_t *payload_len);

#endif
