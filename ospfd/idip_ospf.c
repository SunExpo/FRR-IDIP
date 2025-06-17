#include "zebra.h"
#include "log.h"
#include "memory.h"

#include "lib/if.h"             // for ifindex_t
#include "lib/libospf.h"   // 推荐来源，含 OSPF_LS_INFINITY
#include "ospfd/ospfd.h" // 若还需 OSPF_OPTION_DC

#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_opaque.h"


#include "lib/log.h"
#include <arpa/inet.h>      // for inet_ntoa

#include "idip_map.h"
#include "idip_packet.h"
#include "idip_opaque.h"
#include "idip_ospf.h"


#include "idip.h"


#include "log.h"
#include <string.h>
#include <netinet/in.h>

uint32_t get_this_router_id(void)
{
    struct ospf *o = ospf_get_instance();
    return o ? o->router_id : 0;
}


// 模拟本节点的逻辑 ID

// 接收到封装数据包后的统一入口
int idip_receive_handler(const uint8_t *packet, size_t len) {
    uint32_t this_router_id = get_this_router_id();
    uint32_t src_id, dst_id;
    const uint8_t *payload;
    size_t payload_len;

    if (idip_decapsulate(packet, len, &src_id, &dst_id, &payload, &payload_len) < 0)
        return -1;

    if (dst_id == this_router_id) {
        zlog_info("IDIP: 收到来自 ID=%u 的数据包", src_id);
        // TODO: 调用业务处理 payload
    } else {
        struct in_addr ip_next;
        if (id_to_ip_lookup(dst_id, &ip_next) == 0) {
            zlog_info("IDIP: 中继转发目标 ID=%u 到 IP=%s", dst_id, inet_ntoa(ip_next));
            // TODO: IP 转发调用
        } else {
            zlog_warn("IDIP: 未找到目标 ID=%u 的映射，丢弃", dst_id);
        }
    }

    return 0;
}

// IDIP 初始化入口（在 OSPF 启动后调用）
void idip_ospf_init(void) {
    idip_map_init();
    idip_opaque_register();
    zlog_info("IDIP 模块初始化完成");
}