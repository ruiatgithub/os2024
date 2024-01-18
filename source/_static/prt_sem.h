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
 * Description: 信号量模块对外头文件。
 */
#ifndef PRT_SEM_H
#define PRT_SEM_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * 信号量错误码：初始化信号量内存不足。
 *
 * 值: 0x02000701
 *
 * 解决方案: 可以将私有的静态内存空间配大。
 */
#define OS_ERRNO_SEM_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x01)

/*
 * 信号量错误码：信号量句柄非法（错误或已删除）。
 *
 * 值: 0x02000702
 *
 * 解决方案: 查看输入的信号量句柄值是否有效。
 */
#define OS_ERRNO_SEM_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x02)

/*
 * 信号量错误码：输入指针为空。
 *
 * 值: 0x02000703
 *
 * 解决方案: 查看指针是否输入为空。
 */
#define OS_ERRNO_SEM_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x03)

/*
 * 信号量错误码：没有空闲信号量。
 *
 * 值: 0x02000704
 *
 * 解决方案: 查看信号量模块是否打开，或配置更多信号量。
 */
#define OS_ERRNO_SEM_ALL_BUSY OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x04)

/*
 * 信号量错误码：信号量没有可用资源且Pend时设为不等待(等待时间为0)时获取信号量失败。
 *
 * 值: 0x02000705
 *
 * 解决方案: 无。
 */
#define OS_ERRNO_SEM_UNAVAILABLE OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x05)

/*
 * 信号量错误码：禁止中断处理函数阻塞于信号量。
 *
 * 值: 0x02000706
 *
 * 解决方案: 查看是否在中断中Pend信号量。
 */
#define OS_ERRNO_SEM_PEND_INTERR OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x06)

/*
 * 信号量错误码：任务切换锁定时，禁止任务阻塞于信号量。
 *
 * 值: 0x02000707
 *
 * 解决方案: 不要在锁任务时pend没有资源可用的信号量。
 */
#define OS_ERRNO_SEM_PEND_IN_LOCK OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x07)

/*
 * 信号量错误码：信号量等待超时。
 *
 * 值: 0x02000708
 *
 * 解决方案: 无。
 */
#define OS_ERRNO_SEM_TIMEOUT OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x08)

/*
 * 信号量错误码：信号量发生溢出。
 *
 * 值: 0x02000709
 *
 * 解决方案: 查看输入的信号量计数值是否有效。
 */
#define OS_ERRNO_SEM_OVERFLOW OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x09)

/*
 * 信号量错误码：信号量删除和重设当前值时有阻塞于信号量的任务。
 *
 * 值: 0x0200070a
 *
 * 解决方案: 如果当前信号量有任务阻塞，不能进行删除和重设计数值操作。
 */
#define OS_ERRNO_SEM_PENDED OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x0a)

/*
 * 信号量错误码：注册核内信号量个数为0导致注册失败。
 *
 * 值: 0x0200070b
 *
 * 解决方案: 查看信号量模块配置的最大个数是否为0。
 */
#define OS_ERRNO_SEM_REG_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x0b)

/*
 * 信号量错误码：调用#PRT_SemGetPendList时，指定的内存空间不足，无法存入全部的阻塞任务PID。
 *
 * 值: 0x0200070c
 *
 * 解决方案: 建议将数组长度配置为(#OS_TSK_MAX_SUPPORT_NUM + 1) * 4。
 */
#define OS_ERRNO_SEM_INPUT_BUF_NOT_ENOUGH OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x0c)

/*
 * 信号量错误码：调用#PRT_SemGetPendList时，输入指针为空或者bufLen小于4。
 *
 * 值: 0x0200070d
 *
 * 解决方案: 出参不能为NULL，指定的缓存长度不能小于4。
 */
#define OS_ERRNO_SEM_INPUT_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x0d)

/*
 * 信号量错误码：获取信号量详细信息时出参结构体指针为NULL。
 *
 * 值: 0x0200070e
 *
 * 解决方案: 用来保存信号量详细信息的结构体指针不能为NULL。
 */
#define OS_ERRNO_SEM_INFO_NULL OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x0e)

/*
 * 信号量错误码：获取当前信号量计数时传入的出参为NULL。
 *
 * 值: 0x0200070f
 *
 * 解决方案: 互斥型信号量的唤醒方式不能为FIFO。
 */
#define OS_ERRNO_SEM_COUNT_GET_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x0f)

/*
 * 信号量错误码：非当前互斥信号量的持有者释放该信号量。
 *
 * 值: 0x02000710
 *
 * 解决方案: 互斥信号量只能由其持有者释放，即互斥信号量的PV操作必须配对使用。
 */
#define OS_ERRNO_SEM_MUTEX_NOT_OWNER_POST OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x10)

/*
 * 信号量错误码：在中断中释放互斥型信号量。
 *
 * 值: 0x02000711
 *
 * 解决方案: 只能在任务中对互斥型信号量进行PV操作。
 */
#define OS_ERRNO_SEM_MUTEX_POST_INTERR OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x11)

/*
 * 尚未实现该功能
 */
#define OS_ERRNO_SEM_FUNC_NOT_SUPPORT OS_ERRNO_BUILD_ERROR(OS_MID_SEM, 0x12)

/*
 * 信号量等待时间设定：表示不等待。
 */
#define OS_NO_WAIT 0

/*
 * 信号量等待时间设定：表示永久等待。
 */
#define OS_WAIT_FOREVER 0xFFFFFFFF

/*
 * 二进制信号量空闲状态，互斥型信号量初始计数值。
 */
#define OS_SEM_FULL  1

/*
 * 二进制信号量占用状态，同步型信号量初始计数值。
 */
#define OS_SEM_EMPTY 0

/*
 * 计数型信号量最大计数值。
 */
#define OS_SEM_COUNT_MAX 0xFFFFFFFE

/*
 * 信号量不被任何任务持有，处于空闲状态。
 */
#define OS_INVALID_OWNER_ID 0xFFFFFFFE

/*
 * 信号量句柄类型定义。
 */
typedef U16 SemHandle;

/*
 * 信号量类型。
 */
/* 计数型信号量 */
#define SEM_TYPE_COUNT 0
/* 二进制信号量 */
#define SEM_TYPE_BIN 1

/*
 * 信号量模块被阻塞线程唤醒方式。
 */
enum SemMode {
    SEM_MODE_FIFO,  // 信号量FIFO唤醒模式
    SEM_MODE_PRIOR, // 信号量优先级唤醒模式
    SEM_MODE_BUTT   // 信号量非法唤醒方式
};

/*
 * 信号量模块配置信息的结构体定义。
 */
struct SemModInfo {
    /* 最大支持的信号量数 */
    U16 maxNum;
    /* 保留 */
    U16 reserved;
};

/*
 * 信号量模块获取信号量详细信息时的信息结构体。
 */
struct SemInfo {
    /* 信号量计数 */
    U32 count;
    /* 信号量占用者，对于计数型信号量，记录的是最后一次信号量获得者；如果没有被任务获得，则为OS_THREAD_ID_INVALID */
    U32 owner;
    /* 信号量唤醒方式，为SEM_MODE_FIFO或SEM_MODE_PRIOR */
    enum SemMode mode;
    /* 信号量类型，为SEM_TYPE_COUNT（计数型）或SEM_TYPE_BIN（互斥型） */
    U32 type;
};

/*
 * @brief 创建一个计数型信号量。
 *
 * @par 描述
 * 根据用户指定的计数值，创建一个计数型信号量，设定初始计数器数值，唤醒方式为FIFO。
 * @attention
 * <ul><li>创建是否成功会受到"核内信号量裁剪开关"和"最大支持信号量"配置项的限制。</li></ul>
 *
 * @param count     [IN]  类型#U32，计数器初始值，取值范围为[0, 0xFFFFFFFE]。
 * @param semHandle [OUT] 类型#SemHandle *，输出信号量句柄。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemDelete
 */
extern U32 PRT_SemCreate(U32 count, SemHandle *semHandle);

/*
 * @brief 删除一个信号量。
 *
 * @par 描述
 * 删除用户传入的信号量句柄指定的信号量，如果有任务阻塞于该信号量，则删除失败。
 * @attention  无
 *
 * @param semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemCreate
 */
extern U32 PRT_SemDelete(SemHandle semHandle);

/*
 * @brief 获取信号量计数器数值。
 *
 * @par 描述
 * 根据用户输入信号量句柄和计数值，获取信号量计数器数值。
 * @attention 无
 *
 * @param semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
 * @param semCnt    [OUT] 类型#U32 *，保存信号量计数值指针。
 *
 * @retval #OS_OK  0x00000000，获取信号量计数器值成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemCreate | PRT_SemGetInfo
 */
extern U32 PRT_SemGetCount(SemHandle semHandle, U32 *semCnt);

/*
 * @brief 等待一个信号量。
 *
 * @par 描述
 * 等待用户传入信号量句柄指定的信号量，若其计数器值大于0，则直接减1返回成功。否则任务阻塞，
 * 等待其他线程发布该信号量，等待超时时间可设定。
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>等待时间可以选择零等待、等待特定时间、永久等待。</li>
 * <li>该接口只能被任务调用。</li>
 * <li>在锁任务情况下，用户要PEND信号量，要保证当前有可用信号量资源。</li>
 * </ul>
 *
 * @param semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
 * @param timeout   [IN]  类型#U32，等待时间限制，单位为tick，取值范围为[0, 0xffffffff]。#OS_NO_WAIT或0表示不等待，
 * #OS_WAIT_FOREVER或0xffffffff表示永久等待。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemPost
 */
extern U32 PRT_SemPend(SemHandle semHandle, U32 timeout);

/*
 * @brief 发布指定的信号量。
 *
 * @par 描述
 * 发布指定的信号量，若没有任务等待该信号量，则直接将计数器加1返回。
 * 否则根据唤醒方式唤醒相应的阻塞任务，FIFO方式唤醒最早阻塞的任务，优先级方式唤醒阻塞在此信号量的最高优先级任务。
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>在未锁任务的情况下，如果唤醒的任务优先级高于当前任务，则会立刻发生任务切换。</li>
 * <li>发生任务切换时，如果支持优先级唤醒方式，且创建信号量时指定唤醒方式为优先级，
 * 则唤醒阻塞在该信号量上的最高优先级任务。</li>
 * </ul>
 *
 * @param semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemPend
 */
extern U32 PRT_SemPost(SemHandle semHandle);

/*
 * @brief 获取阻塞在指定核内信号量上的任务PID清单。
 *
 * @par 描述
 * 根据用户指定的核内信号量句柄，获取阻塞在指定核内信号量上的任务PID清单。
 * 若有任务阻塞于该核内信号量，则返回阻塞于该核内信号量的任务数目，以及相应任务的PID。
 * 若没有任务阻塞于该核内信号量，则返回阻塞于该核内信号量的任务数目为0。
 * @attention
 * <ul><li>用户应保证存储任务PID清单的内存空间足够大，建议将bufLen配置为(#OS_TSK_MAX_SUPPORT_NUM + 1)
 * 4bytes。</li></ul>
 *
 * @param semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
 * @param tskCnt    [OUT] 类型#U32 *，阻塞于该核内信号量的任务数。
 * @param pidBuf    [OUT] 类型#U32 *，由用户指定的内存区域首地址，用于存储阻塞于指定核内信号量的任务PID。
 * @param bufLen    [IN]  类型#U32，用户指定的内存区域的长度（单位：字节）。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskGetPendSem | PRT_SemGetInfo
 */
extern U32 PRT_SemGetPendList(SemHandle semHandle, U32 *tskCnt, U32 *pidBuf, U32 bufLen);

/*
 * @brief 获取信号量详细信息:信号量当前计数值，信号量持有者(最后一次pend成功的线程ID)，信号量唤醒方式，信号量类型。
 *
 * @par 描述
 * 根据用户输入信号量句柄获取信号量详细信息。
 * @attention 无
 *
 * @param semHandle [IN]  类型#SemHandle，信号量句柄，来源于信号量创建成功的输出值。
 * @param semInfo   [OUT] 类型#struct SemInfo *，信号量详细信息:count--信号量计数，owner--信号量占用者，
 * mode--信号量唤醒方式，type--信号量类型。
 *
 * @retval #OS_OK  0x00000000，获取信号量计数器值成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_sem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemGetCount | PRT_SemGetPendList
 */
extern U32 PRT_SemGetInfo(SemHandle semHandle, struct SemInfo *semInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* PRT_SEM_H */
