#ifndef _IDIP_MAP_H_
#define _IDIP_MAP_H_

#include <stdint.h>
#include <netinet/in.h>

#define MAX_IDIP_MAP_SIZE 256

struct idip_map_entry {
    uint32_t id;
    struct in_addr ip;
};

void idip_map_init(void);
int idip_map_add(uint32_t id, struct in_addr ip);
int id_to_ip_lookup(uint32_t id, struct in_addr *ip_out);
int idip_map_delete(uint32_t id);
void idip_map_print(void);

#endif