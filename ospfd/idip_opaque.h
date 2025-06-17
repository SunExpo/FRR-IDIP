#ifndef _IDIP_OPAQUE_H_
#define _IDIP_OPAQUE_H_

#include <stdint.h>
#include <netinet/in.h>
#include "ospf_lsa.h"

// 注册 TLV handler，供 OSPF 模块调用
void idip_opaque_register(void);

// 主动发送添加本地 ID→IP 映射项
void idip_opaque_send_local_mapping(uint32_t id, struct in_addr ip);

// 主动发送删除本地 ID→IP 映射项
void idip_opaque_send_delete(uint32_t id);


// 从接收到的 Opaque LSA 解析 TLV 内容（注册回调）
void idip_opaque_parse(struct ospf_lsa *lsa, struct stream *s);

#endif
