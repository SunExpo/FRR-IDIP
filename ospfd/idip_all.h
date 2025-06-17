#ifndef _IDIP_ALL_H_
#define _IDIP_ALL_H_

// 基础封装头定义
#include "idip.h"

// 封装与解封装接口
#include "idip_packet.h"

// ID→IP 映射表逻辑
#include "idip_map.h"

// OSPF Opaque 映射广播中继逻辑
#include "idip_opaque.h"

// idip-OSPF 初始化及处理入口
#include "idip_ospf.h"

// idip命令输入接口
#include "idip_vty.h"

#endif /* _IDIP_ALL_H_ */
