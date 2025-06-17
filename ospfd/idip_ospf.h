#ifndef _IDIP_OSPF_H_
#define _IDIP_OSPF_H_
#include "ospfd/ospfd.h"     // for struct ospf and 'ospf' pointer
#include <stdint.h>
#include <stddef.h>
/* Master instance of OSPF structure. */

// 初始化模块（在 OSPF 启动后调用）
void idip_ospf_init(void);

// 收到封装包的处理入口（由 FRR hook 调用）
int idip_receive_handler(const uint8_t *packet, size_t len);
#endif
