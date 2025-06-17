#ifndef _IDIP_MAP_H_
#define _IDIP_MAP_H_

#include <stdint.h>
#include <netinet/in.h>

void idip_map_init(void);
int idip_map_add(uint32_t id, struct in_addr ip);
int id_to_ip_lookup(uint32_t id, struct in_addr *ip_out);
int idip_map_delete(uint32_t id);
void idip_map_print(void);

typedef void (*idip_map_iter_cb)(uint32_t id, struct in_addr ip, void *arg);

void idip_map_iterate(idip_map_iter_cb cb, void *arg);


#endif
