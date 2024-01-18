/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-22
 * Description: 属性宏相关内部头文件
 */

// src/arch/cpu/armv8/common/os_attr_armv8_external.h

#ifndef OS_ATTR_ARMV8_EXTERNAL_H
#define OS_ATTR_ARMV8_EXTERNAL_H

/* 定义操作系统的代码数据分段 */
#ifndef CONFIG_OPTION_FX_SECTIONS
/* L0表示最高性能内存段 */
#ifndef OS_SEC_L0_TEXT
#define OS_SEC_L0_TEXT __attribute__((section(".os.text")))
#endif

#ifndef OS_SEC_LX0_TEXT
#define OS_SEC_LX0_TEXT __attribute__((section(".os.text")))
#endif

/* OS_SEC_L1_TEXT, L1表示性能敏感内存段，如PL2，默认缺省，不显示指定 */
#ifndef OS_SEC_TEXT
#define OS_SEC_TEXT __attribute__((section(".os.text")))
#endif

#ifndef OS_SEC_L2_TEXT
#define OS_SEC_L2_TEXT __attribute__((section(".os.minor.text")))
#endif

#ifndef OS_SEC_L4_TEXT
#define OS_SEC_L4_TEXT __attribute__((section(".os.init.text")))
#endif

#ifndef OS_SEC_LX_TEXT
#define OS_SEC_LX_TEXT __attribute__((section(".os.init.text")))
#endif

#ifndef OS_SEC_DATA
#define OS_SEC_DATA __attribute__((section(".os.data")))
#endif

#ifndef OS_SEC_L4_DATA
#define OS_SEC_L4_DATA __attribute__((section(".os.data")))
#endif

#ifndef OS_SEC_BSS
#define OS_SEC_BSS __attribute__((section(".os.bss")))
#endif

#ifndef OS_SEC_L4_BSS
#define OS_SEC_L4_BSS __attribute__((section(".os.bss")))
#endif

#ifndef OS_SEC_L4_INSTSH_DATA
#define OS_SEC_L4_INSTSH_DATA __attribute__((section(".os.data")))
#endif
#endif /* CONFIG_OPTION_FX_SECTIONS */

#endif /* OS_ATTR_ARMV8_EXTERNAL_H */
