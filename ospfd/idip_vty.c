#include <zebra.h>
#include "vty.h"
#include "command.h"
#include "idip_map.h"
#include "idip_vty.h"

#include <arpa/inet.h>  // for inet_ntoa

/* 添加映射命令 */
DEFUN(idip_map_add_cmd,
      idip_map_add_cmd_cmd,
      "idip-map add <1-4294967295> <A.B.C.D>",
      "IDIP Mapping\nAdd mapping\nID value\nIPv4 address\n")
{
    if (argc != 2) {
        vty_out(vty, "%% Incorrect number of arguments.\n");
        return CMD_WARNING;
    }

    uint32_t id = strtoul(argv[0], NULL, 10);
    struct in_addr ip;

    if (!inet_aton(argv[1], &ip)) {
        vty_out(vty, "%% Invalid IP address format: %s\n", argv[1]);
        return CMD_WARNING;
    }

    int ret = idip_map_add(id, ip);
    if (ret >= 0)
        vty_out(vty, "%% Mapping added: ID=%u → IP=%s\n", id, inet_ntoa(ip));
    else
        vty_out(vty, "%% Mapping table full or error.\n");

    return CMD_SUCCESS;
}

/* 删除映射命令 */
DEFUN(idip_map_del_cmd,
      idip_map_del_cmd_cmd,
      "idip-map del <1-4294967295>",
      "IDIP Mapping\nDelete mapping\nID value\n")
{
    if (argc != 1) {
        vty_out(vty, "%% Missing ID to delete.\n");
        return CMD_WARNING;
    }

    uint32_t id = strtoul(argv[0], NULL, 10);
    int ret = idip_map_delete(id);
    if (ret == 0)
        vty_out(vty, "%% Mapping deleted: ID=%u\n", id);
    else
        vty_out(vty, "%% No such ID in mapping.\n");

    return CMD_SUCCESS;
}

/* 显示映射时的回调函数 */
static void vty_map_show_cb(uint32_t id, struct in_addr ip, void *arg)
{
    struct vty *vty = (struct vty *)arg;
    vty_out(vty, "ID: %u → IP: %s\n", id, inet_ntoa(ip));
}

/* 显示映射命令 */
DEFUN(idip_map_show_cmd,
      idip_map_show_cmd_cmd,
      "show idip-map",
      "Show commands\nShow IDIP ID→IP map\n")
{
    vty_out(vty, "=== IDIP Mapping Table ===\n");
    idip_map_iterate(vty_map_show_cb, vty);
    return CMD_SUCCESS;
}
/* 初始化 IDIP 命令注册 */
void idip_vty_init(void)
{
    install_element(CONFIG_NODE, &idip_map_add_cmd_cmd);
    install_element(CONFIG_NODE, &idip_map_del_cmd_cmd);
    install_element(ENABLE_NODE, &idip_map_show_cmd_cmd);
    printf("[IDIP] VTY commands registered\n");

}


