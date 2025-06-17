#include "zebra.h"
#include "log.h"
#include "memory.h"

#include "lib/if.h"             // ifindex_t
#include "lib/libospf.h"        // OSPF_LS_INFINITY
#include "ospfd/ospfd.h"        // struct ospf, OSPF_OPTION_DC
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_opaque.h"

#include <arpa/inet.h>          // inet_ntoa
#include <netinet/in.h>         // struct in_addr
#include <string.h>

#include "idip.h"
#include "idip_map.h"
#include "idip_packet.h"
#include "idip_opaque.h"
#include "idip_ospf.h"


// 获取 OSPF 实例指针
struct ospf *ospf_get_instance(void)
{
    return ospf_lookup(0, NULL);  // ✅ 默认 OSPF 实例
}

// 获取 Router-ID（uint32_t 形式）
uint32_t get_this_router_id_u32(void)
{
    struct ospf *o = ospf_get_instance();
    return o ? o->router_id.s_addr : 0;
}

// 全局保存本节点 Router-ID（用 ID 表示）
uint32_t this_router_id = 0;

// 接收到封装数据包后的统一入口
int idip_receive_handler(const uint8_t *packet, size_t len)
{
    // 延迟初始化 router-id（可以考虑改为启动时初始化）
    if (this_router_id == 0)
        this_router_id = get_this_router_id_u32();

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
            zlog_info("IDIP: 中继转发目标 ID=%u 到 IP=%s",
                      dst_id, inet_ntoa(ip_next));
            // TODO: IP 转发逻辑（暂未实现）
        } else {
            zlog_warn("IDIP: 未找到目标 ID=%u 的映射，丢弃", dst_id);
        }
    }

    return 0;
}

// IDIP 初始化入口（应在 OSPF 初始化之后调用）
void idip_ospf_init(void)
{
    idip_map_init();
    // idip_opaque_register();
    zlog_info("IDIP 模块初始化完成");
}
