#include <zebra.h>
#include "vty.h"
#include "command.h"
#include "idip_map.h"
#include "idip_vty.h"

#include <arpa/inet.h>  // for inet_ntoa

extern struct idip_map_entry idip_map[];  // 必须在 idip_map.c 中取消 static，或提供访问函数

/* 添加映射命令 */
DEFUN(idip_map_add_cmd,
      idip_map_add_cmd_cmd,
      "idip-map add ID A.B.C.D",
      "IDIP Mapping\nAdd mapping\nID value\nIPv4 address\n")
{
    uint32_t id = strtoul(argv[0], NULL, 10);
    struct in_addr ip;

    if (!inet_aton(argv[1], &ip)) {
        vty_out(vty, "%% Invalid IP address format.\n");
        return CMD_WARNING;
    }

    int ret = idip_map_add(id, ip);
    if (ret >= 0)
        vty_out(vty, "%% Mapping added: ID=%u → IP=%s\n", id, argv[1]);
    else
        vty_out(vty, "%% Mapping table full or error.\n");

    return CMD_SUCCESS;
}

/* 删除映射命令 */
DEFUN(idip_map_del_cmd,
      idip_map_del_cmd_cmd,
      "idip-map del ID",
      "IDIP Mapping\nDelete mapping\nID value\n")
{
    uint32_t id = strtoul(argv[0], NULL, 10);
    int ret = idip_map_delete(id);
    if (ret == 0)
        vty_out(vty, "%% Mapping deleted: ID=%u\n", id);
    else
        vty_out(vty, "%% No such ID in mapping.\n");

    return CMD_SUCCESS;
}

/* 显示映射命令 */
DEFUN(idip_map_show_cmd,
      idip_map_show_cmd_cmd,
      "show idip-map",
      "Show commands\nShow IDIP ID→IP map\n")
{
    vty_out(vty, "=== IDIP Mapping Table ===\n");
    for (int i = 0; i < MAX_IDIP_MAP_SIZE; ++i) {
        if (idip_map[i].id != 0) {
            vty_out(vty, "ID: %u → IP: %s\n",
                    idip_map[i].id, inet_ntoa(idip_map[i].ip));
        }
    }
    return CMD_SUCCESS;
}

/* 初始化 IDIP 命令注册 */
void idip_vty_init(void)
{
    install_element(CONFIG_NODE, &idip_map_add_cmd_cmd);
    install_element(CONFIG_NODE, &idip_map_del_cmd_cmd);
    install_element(ENABLE_NODE, &idip_map_show_cmd_cmd);
}
