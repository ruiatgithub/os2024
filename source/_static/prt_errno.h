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
 * Create: 2009-12-22
 * Description: 通用错误码定义头文件。
 */
#ifndef PRT_ERRNO_H
#define PRT_ERRNO_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * OS错误码标记位，0x00表示OS
 *
 */
#define ERRNO_OS_ID    (0x00U << 16)

/*
 * 定义错误的等级:提示级别
 *
 */
#define ERRTYPE_NORMAL (0x00U << 24)

/*
 * 定义错误的等级:告警级别
 *
 */
#define ERRTYPE_WARN   (0x01U << 24)

/*
 * 定义错误的等级:严重级别
 *
 */
#define ERRTYPE_ERROR  (0x02U << 24)

/*
 * 定义错误的等级:致命级别
 *
 */
#define ERRTYPE_FATAL  (0x03U << 24)

/*
 * @brief 定义OS致命错误。
 *
 * @par 描述
 * 宏定义，定义OS致命错误。
 *
 * @attention 无
 *
 * @param  mid   [IN] 模块ID编号。
 * @param  errno [IN] 错误码编号。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_errno.h：该宏定义所在的头文件。</li>
 * @see OS_ERRNO_BUILD_ERROR | OS_ERRNO_BUILD_WARN | OS_ERRNO_BUILD_NORMAL
 */
#define OS_ERRNO_BUILD_FATAL(mid, errno) (ERRTYPE_FATAL | ERRNO_OS_ID | ((U32)(mid) << 8) | (errno))

/*
 * @brief 定义OS严重错误
 *
 * @par 描述
 * 宏定义，定义OS严重错误
 *
 * @attention 无
 * @param  mid   [IN] 模块ID编号。
 * @param  errno [IN] 错误码编号。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_errno.h：该宏定义所在的头文件。</li>
 * @see OS_ERRNO_BUILD_FATAL | OS_ERRNO_BUILD_WARN | OS_ERRNO_BUILD_NORMAL
 */
#define OS_ERRNO_BUILD_ERROR(mid, errno) (ERRTYPE_ERROR | ERRNO_OS_ID | ((U32)(mid) << 8) | (errno))

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_ERRNO_H */
