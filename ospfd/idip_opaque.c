#include "idip_map.h"
#include "log.h"
#include "stream.h"
#include "prefix.h"
#include "lib/libospf.h"   // 推荐来源，含 OSPF_LS_INFINITY
#include "ospfd/ospfd.h" // 若还需 OSPF_OPTION_DC
#include "ospfd/ospf_lsa.h"
#include "ospfd/ospf_lsdb.h"


#include "ospf_opaque.h"
#include "ospf_neighbor.h"
#include "ospf_interface.h"
#include "ospfd.h"
#include "zebra/rib.h"
#include "nexthop.h"
#include <string.h>
#include <arpa/inet.h>

#define IDIP_TLV_TYPE_MAPPING 0x90FA
#define IDIP_TLV_TYPE_DELETE  0x90FB
#define IDIP_TLV_LENGTH       8
#define IDIP_TLV_LEN_DELETE   4
#define OPAQUE_TYPE_IDIP      99

/* =====================================================
 * 判断 IP 是否为直连邻居（遍历 OSPF 邻居列表）
 * ===================================================== */
static bool is_ip_my_neighbor(struct in_addr ip) {
    struct ospf *ospf = ospf_get_instance();
    if (!ospf || !ospf->oiflist)
        return false;

    struct ospf_interface *oi;
    struct listnode *ifnode;

    for (ALL_LIST_ELEMENTS_RO(ospf->oiflist, ifnode, oi)) {
        if (!oi || !oi->nbrs)
            continue;

        struct route_node *rn;
        for (rn = route_top(oi->nbrs); rn; rn = route_next(rn)) {
            struct ospf_neighbor *nbr = rn->info;
            if (!nbr)
                continue;

            if (nbr->address.u.prefix4.s_addr == ip.s_addr)
                return true;
        }
    }

    return false;
}

/* =====================================================
 * 查找通往目标 ID 的下一跳（relay IP）
 * 使用公共 RIB 接口，默认查 VRF 0
 * ===================================================== */
static struct in_addr get_back_route_ip_to(uint32_t id) {
    struct in_addr fallback = {0};
    if (id_to_ip_lookup(id, &fallback) < 0)
        return fallback;

    struct prefix_ipv4 p = { .family = AF_INET, .prefix = fallback, .prefixlen = 32 };
    struct route_entry *re = rib_lookup_ipv4(&p, 0);  // VRF 0
    if (!re)
        return fallback;

    struct nexthop_group *nhg = rib_get_fib_nhg(re);
    if (!nhg)
        return fallback;

    for (struct nexthop *nh = nhg->nexthop; nh; nh = nh->next) {
        if (nh->type == NEXTHOP_TYPE_IPV4 || nh->type == NEXTHOP_TYPE_IPV4_IFINDEX)
            return nh->gate.ipv4;
    }

    return fallback;
}

/* =====================================================
 * 发送 TLV（用于注册/中继广播）
 * ===================================================== */
static void idip_opaque_send_local_mapping(uint32_t id, struct in_addr ip) {
    struct stream *s = stream_new(12);
    stream_putw(s, IDIP_TLV_TYPE_MAPPING);
    stream_putw(s, IDIP_TLV_LENGTH);
    stream_putl(s, id);
    stream_put(s, &ip, 4);
    ospf_opaque_lsa_type9_update(OPAQUE_TYPE_IDIP, s);  // 广播 TLV
    stream_free(s);
}

/* =====================================================
 * 发送 TLV（用于删除广播）
 * ===================================================== */
void idip_opaque_send_delete(uint32_t id) {
    struct stream *s = stream_new(8);
    stream_putw(s, IDIP_TLV_TYPE_DELETE);
    stream_putw(s, IDIP_TLV_LEN_DELETE);
    stream_putl(s, id);

    // 调用 OSPF 的 LSA 9 更新函数，自动泛洪
    ospf_opaque_lsa_type9_update(OPAQUE_TYPE_IDIP, s);

    stream_free(s);
}



/* =====================================================
 * 接收 TLV 回调：由 OSPF opaque LSA 触发
 * 支持添加/更新/delete 映射表
 * 支持 relay 替换并重广播
 * ===================================================== */
void idip_opaque_parse(struct ospf_lsa *lsa, struct stream *s) {
    while (STREAM_READABLE(s) >= 4) {
        uint16_t type = stream_getw(s);
        uint16_t len = stream_getw(s);

        if (type == IDIP_TLV_TYPE_MAPPING && len == IDIP_TLV_LENGTH) {
            uint32_t id = stream_getl(s);
            struct in_addr ip;
            stream_get(&ip, s, 4);

            // 若本地已存在该 ID → 跳过处理（不管 IP 是否相同，防环）
            if (id_to_ip_lookup(id, NULL) == 0) {
                zlog_debug("IDIP: 已存在 ID=%u，跳过（防环）", id);
                continue;
            }

            // 不是邻居 → relay 模式
            if (!is_ip_my_neighbor(ip)) {
                struct in_addr relay_ip = get_back_route_ip_to(id);

                if (relay_ip.s_addr != 0 && relay_ip.s_addr != ip.s_addr) {
                    idip_map_add(id, relay_ip);
                    idip_opaque_send_local_mapping(id, relay_ip);
                    zlog_info("IDIP: relay ID=%u, 原IP=%s → relay IP=%s",
                              id, inet_ntoa(ip), inet_ntoa(relay_ip));
                } else {
                    zlog_debug("IDIP: relay 不需要，保持原 IP");
                    idip_map_add(id, ip);
                }

            } else {
                // 是邻居 → 直接记录并广播
                idip_map_add(id, ip);
                idip_opaque_send_local_mapping(id, ip);
                zlog_debug("IDIP: 邻居 IP 映射添加并广播，ID=%u IP=%s", id, inet_ntoa(ip));
            }

        } else if (type == IDIP_TLV_TYPE_DELETE && len == IDIP_TLV_LEN_DELETE) {
            uint32_t id = stream_getl(s);
            if (id_to_ip_lookup(id, NULL) == 0) {
                idip_map_delete(id);
                zlog_info("IDIP: 删除 ID=%u 映射", id);
            } else {
                zlog_debug("IDIP: 收到删除 ID=%u，但本地无记录，跳过", id);
            }

        } else {
            // 跳过未知 TLV
            stream_forward_getp(s, len);
        }
    }
}



/* =====================================================
 * 模块初始化：注册 TLV 回调
 * ===================================================== */
void idip_opaque_register(void) {
    ospf_register_opaque_lsa_handler(OPAQUE_TYPE_IDIP, idip_opaque_parse);
}
