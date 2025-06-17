#include "idip.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  // for inet_ntoa, htons, htonl, ntohl
// 封装函数：创建包含 IDIP Header 的包
uint8_t *idip_encapsulate(const uint8_t *payload, size_t len,
                          uint32_t src_id, uint32_t dst_id,
                          size_t *out_len) {
    struct idip_header hdr;
    hdr.src_id = htonl(src_id);
    hdr.dst_id = htonl(dst_id);
    hdr.flags = htons(0);
    hdr.reserved = 0;

    *out_len = sizeof(struct idip_header) + len;
    uint8_t *packet = malloc(*out_len);
    if (!packet) return NULL;

    memcpy(packet, &hdr, sizeof(hdr));
    memcpy(packet + sizeof(hdr), payload, len);
    return packet;
}

// 解封装函数：解析 header，返回 payload 与字段
int idip_decapsulate(const uint8_t *packet, size_t len,
                     uint32_t *src_id, uint32_t *dst_id,
                     const uint8_t **payload_out, size_t *payload_len) {
    if (len < sizeof(struct idip_header)) return -1;

    const struct idip_header *hdr = (const struct idip_header *)packet;
    *src_id = ntohl(hdr->src_id);
    *dst_id = ntohl(hdr->dst_id);

    *payload_out = packet + sizeof(struct idip_header);
    *payload_len = len - sizeof(struct idip_header);

    return 0;
}
