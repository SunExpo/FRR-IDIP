#include "idip_map.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define MAX_IDIP_MAP_SIZE 256

struct idip_map_entry {
    uint32_t id;
    struct in_addr ip;
};

static struct idip_map_entry idip_map[MAX_IDIP_MAP_SIZE];
static int map_count = 0;

void idip_map_init(void) {
    memset(idip_map, 0, sizeof(idip_map));
    map_count = 0;
}

int idip_map_add(uint32_t id, struct in_addr ip) {
    for (int i = 0; i < map_count; ++i) {
        if (idip_map[i].id == id) {
            idip_map[i].ip = ip;
            return 1;  // updated
        }
    }
    if (map_count >= MAX_IDIP_MAP_SIZE)
        return -1; // full

    idip_map[map_count].id = id;
    idip_map[map_count].ip = ip;
    map_count++;
    return 0;  // added
}

int id_to_ip_lookup(uint32_t id, struct in_addr *ip_out) {
    for (int i = 0; i < map_count; ++i) {
        if (idip_map[i].id == id) {
            if (ip_out)
                *ip_out = idip_map[i].ip;
            return 0;
        }
    }
    return -1;  // not found
}

int idip_map_delete(uint32_t id) {
    for (int i = 0; i < map_count; ++i) {
        if (idip_map[i].id == id) {
            idip_map[i] = idip_map[map_count - 1];
            memset(&idip_map[map_count - 1], 0, sizeof(struct idip_map_entry));
            map_count--;
            return 0;
        }
    }
    return -1;  // not found
}

void idip_map_print(void) {
    char ipbuf[INET_ADDRSTRLEN];
    printf("=== ID→IP 映射表 (%d 项) ===\n", map_count);
    for (int i = 0; i < map_count; ++i) {
        if (!inet_ntop(AF_INET, &idip_map[i].ip, ipbuf, sizeof(ipbuf)))
            strcpy(ipbuf, "转换失败");

        printf("ID: %u → IP: %s\n", idip_map[i].id, ipbuf);
    }
}



void idip_map_iterate(idip_map_iter_cb cb, void *arg) {
    for (int i = 0; i < map_count; ++i) {
        cb(idip_map[i].id, idip_map[i].ip, arg);
    }
}
