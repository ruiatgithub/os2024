/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-10-05
 * Description: 核架构相关的汇编宏头文件
 */
#ifndef PRT_ASM_CPU_EXTERNAL_H
#define PRT_ASM_CPU_EXTERNAL_H

/* 有名空间不支持时，从核不能从有名空间获取 */
#define GLB_NS_ADDR     0
#define GLB_NSM_START   0

#define OS_EXC_REGINFO_OFFSET 160
#define OS_EXC_MAGIC_WORD 0xEDCAACDC
#define OS_SYS_STACK_MAGIC_WORD 0xCACACACA /* 系统栈魔术字 */

/* 内核状态定义 */
#define OS_FLG_HWI_ACTIVE 0x0001
#define OS_FLG_BGD_ACTIVE 0x0002
#define OS_FLG_TICK_ACTIVE 0x0008
#define OS_FLG_SYS_ACTIVE 0x0010
#define OS_FLG_EXC_ACTIVE 0x0020
#define OS_FLG_TSK_REQ 0x1000
#define OS_FLG_TSK_SWHK 0x2000 /* 任务切换时是否调用切换入口函数 */

#endif /* PRT_ASM_CPU_EXTERNAL_H */
