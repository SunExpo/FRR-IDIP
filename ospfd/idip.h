#ifndef _IDIP_H_
#define _IDIP_H_

#include <stdint.h>

// 自定义 IP 层协议号（可以在 /etc/protocols 中注册）
// 253~254 为实验性协议号范围
#define IDIP_PROTO 0xFD

// IDIP 封装头，紧跟在 IP 头后（不含 IP），占 12 字节
struct idip_header {
    uint32_t src_id;      // 源端逻辑 ID（如 DroneID、NodeID）
    uint32_t dst_id;      // 目标端逻辑 ID
    uint16_t flags;       // 保留扩展位，如优先级、可靠性等
    uint16_t reserved;    // 对齐用，占位
} __attribute__((packed)); // 紧凑结构，确保无填充

#endif