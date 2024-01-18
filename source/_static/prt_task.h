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
 * Description: 任务模块的对外头文件。
 */
#ifndef PRT_TASK_H
#define PRT_TASK_H

// #include "prt_buildef.h"
#include "os_cpu_armv8.h"
#include "prt_module.h"
#include "prt_errno.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


// src/core/kernel/task/prt_task_internal.h
/* 任务栈初始化魔术字，默认是0xCA，只支持配置一个字节 */
#define OS_TSK_STACK_MAGIC                         0xCA
#define OS_TSK_STACK_CFG_BY_USER 1
#define OS_TSK_STACK_CFG_BY_SYS 0

/*
 * 模块内全局变量声明
 */
extern struct TagListObject g_tskCbFreeList;
extern struct TagListObject g_tskRecyleList;

/* OS系统状态标志位 */
extern  U32 g_uniFlag;
#define UNI_FLAG g_uniFlag

// src/core/kernel/include/prt_sys_external.h
#define OS_INT_ACTIVE_MASK \
    (OS_FLG_HWI_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)
#define OS_INT_ACTIVE ((UNI_FLAG & OS_INT_ACTIVE_MASK) != 0)
#define OS_INT_INACTIVE (!(OS_INT_ACTIVE))
/*
 * 任务名的最大长度。
 *
 * 任务名的最大长度，包括结尾符'\0'。
 */
#define OS_TSK_NAME_LEN 16

/*
 * U32类型的PID中，TCB Index占用的比特数
 */
#define OS_TSK_TCB_INDEX_BITS ((4 - OS_TSK_CORE_BYTES_IN_PID) * 8)

/*
 * 从线程PID获取核内线程handle号
 */
#define GET_HANDLE(pid) ((pid) & ((1U << OS_TSK_TCB_INDEX_BITS) - 1))

/*
 * 硬中断核内线程handle号
 */
#define OS_HWI_HANDLE ((1U << OS_TSK_TCB_INDEX_BITS) - 1)

/*
 * 从线程PID获取核号
 */
#define GET_COREID(pid) ((U8)((pid) >> OS_TSK_TCB_INDEX_BITS))

/*
 * 将coreid与handle组成PID,  coreid:[0, OS_MAX_CORE_NUM); handle:[0, 255]
 */
#define COMPOSE_PID(coreid, handle) \
    ((((U32)(coreid)) << OS_TSK_TCB_INDEX_BITS) + ((U8)(handle))) /* 将(coreid)与(handle)组成PID,UIPC不使用该接口 */

/*
 * 任务或任务控制块状态标志。
 *
 * 任务控制块未被使用。
 */
#define OS_TSK_UNUSED 0x0000

/*
 * 任务或任务控制块状态标志。
 *
 * 任务控制块被使用,任务被创建。
 */
#define OS_TSK_INUSE 0x0001

/*
 * 任务或任务控制块状态标志。
 *
 * 任务被阻塞（等待信号）。
 */
#define OS_TSK_WAIT_SIGNAL 0x0002

/*
 * 任务或任务控制块状态标志。
 *
 * 任务被挂起。
 */
#define OS_TSK_SUSPEND 0x0004

/*
 * 任务或任务控制块状态标志。
 *
 * 任务被阻塞（等待信号量）。
 */
#define OS_TSK_PEND 0x0008

/*
 * 任务或任务控制块状态标志。
 *
 * 任务在等待信号量或者事件的标志。
 */
#define OS_TSK_TIMEOUT 0x0010

/*
 * 任务或任务控制块状态标志。
 *
 * 任务被延时。
 */
#define OS_TSK_DELAY 0x0020

/*
 * 任务或任务控制块状态标志。
 *
 * 任务已就绪，已加入就绪队列。
 */
#define OS_TSK_READY 0x0040

/*
 * 任务或任务控制块状态标志。
 *
 * 任务正运行，仍在就绪队列。
 */
#define OS_TSK_RUNNING 0x0080

/*
 * 任务有信号在准备处理。
 *
 * OS_TSK_HOLD_SIGNAL
 */
#define OS_TSK_HOLD_SIGNAL 0x0400

/*
 * 任务或任务控制块状态标志。
 *
 * OS_TSK_EVENT_PEND      --- 任务阻塞于等待读事件。
 */
#define OS_TSK_EVENT_PEND 0x0800

/*
 * 任务或任务控制块状态标志。
 *
 * OS_TSK_EVENT_TYPE    --- 任务读事件类型，0:ANY; 1:ALL。
 */
#define OS_TSK_EVENT_TYPE 0x1000

/*
 * 任务或任务控制块状态标志。
 *
 * OS_TSK_QUEUE_PEND      --- 任务阻塞与等待队列。
 */
#define OS_TSK_QUEUE_PEND 0x2000

/*
 * 任务或任务控制块状态标志。
 *
 * OS_TSK_QUEUE_BUSY      --- 队列正在读写数据。
 */
#define OS_TSK_QUEUE_BUSY 0x4000

/*
 * 任务或任务控制块状态标志。
 *
 * 任务被延时，可被唤醒。
 */
#define OS_TSK_DELAY_INTERRUPTIBLE 0x8000

/*
 * 任务或任务控制块状态标志。
 *
 * 任务在等待队列中
 */
#define OS_TSK_WAITQUEUE_PEND 0x10000

/*
 * 任务或任务控制块状态标志。
 *
 * 任务暂停，等待任何信号处理完成
 */
#define OS_TSK_SIG_PAUSE 0x20000



/*
 * 任务模块的错误码定义。
 */
/*
 * 任务错误码：申请内存失败。
 *
 * 值: 0x02000301
 *
 * 解决方案: 分配更大的私有FSC内存分区
 *
 */
#define OS_ERRNO_TSK_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x01)

/*
 * 任务错误码：指针参数为空。
 *
 * 值: 0x02000302
 *
 * 解决方案: 检查参数指针是否为NUL。
 */
#define OS_ERRNO_TSK_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x02)

/*
 * 任务错误码：任务栈大小未按16字节大小对齐。
 *
 * 值: 0x02000303
 *
 * 解决方案: 检查入参任务栈大小是否按16字节对齐。
 */
#define OS_ERRNO_TSK_STKSZ_NOT_ALIGN OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x03)

/*
 * 任务错误码：任务优先级非法。
 *
 * 值: 0x02000304
 *
 * 解决方案: 检查入参任务优先级不能大于63，其他平台不能大于31。
 */
#define OS_ERRNO_TSK_PRIOR_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x04)

/*
 * 任务错误码：任务入口函数为空。
 *
 * 值: 0x02000305
 *
 * 解决方案: 检查入参任务入口函数是否为NULL。
 */
#define OS_ERRNO_TSK_ENTRY_NULL OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x05)

/*
 * 任务错误码：任务名的指针为空或任务名为空字符串。
 *
 * 值: 0x02000306
 *
 * 解决方案: 检查任务名指针和任务名。
 */
#define OS_ERRNO_TSK_NAME_EMPTY OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x06)

/*
 * 任务错误码：指定的任务栈空间太小。
 *
 * 值: 0x02000307
 *
 * 解决方案: 检查任务栈是否小于OS_TSK_MIN_STACK_SIZE。
 */
#define OS_ERRNO_TSK_STKSZ_TOO_SMALL OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x07)

/*
 * 任务错误码：任务ID非法。
 *
 * 值: 0x02000308
 *
 * 解决方案: 检查当前运行任务的PID是否超过任务最大数或检查用户入参任务PID是否合法。
 */
#define OS_ERRNO_TSK_ID_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x08)

/*
 * 任务错误码：任务已被挂起。
 *
 * 值: 0x02000309
 *
 * 解决方案: 检查所挂起任务是否为已挂起任务。
 */
#define OS_ERRNO_TSK_ALREADY_SUSPENDED OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x09)

/*
 * 任务错误码：任务未被挂起。
 *
 * 值: 0x0200030a
 *
 * 解决方案: 检查所恢复任务是否未挂起。
 */
#define OS_ERRNO_TSK_NOT_SUSPENDED OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x0a)

/*
 * 任务错误码：任务未创建。
 *
 * 值: 0x0200030b
 *
 * 解决方案: 检查任务是否创建。
 */
#define OS_ERRNO_TSK_NOT_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x0b)

/*
 * 任务错误码：在锁任务的状态下删除当前任务。
 *
 * 值: 0x0300030c
 *
 * 解决方案: 用户确保删除任务时，将任务解锁。
 *
 */
#define OS_ERRNO_TSK_DELETE_LOCKED OS_ERRNO_BUILD_FATAL(OS_MID_TSK, 0x0c)

/*
 * 任务错误码：在硬中断的处理中进行延时操作。
 *
 * 值: 0x0300030d
 *
 * 解决方案: 此操作禁止在中断中进行调度。
 *
 */
#define OS_ERRNO_TSK_DELAY_IN_INT OS_ERRNO_BUILD_FATAL(OS_MID_TSK, 0x0d)

/*
 * 任务错误码：在锁任务的状态下进行延时操作。
 *
 * 值: 0x0200030e
 *
 * 解决方案: 检查是否锁任务。
 */
#define OS_ERRNO_TSK_DELAY_IN_LOCK OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x0e)

/*
 * 任务错误码：任务ID不在Yield操作指定的优先级队列中。
 *
 * 值: 0x0200030f
 *
 * 解决方案: 检查操作的任务的优先级。
 */
#define OS_ERRNO_TSK_YIELD_INVALID_TASK OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x0f)

/*
 * 任务错误码：Yield操作指定的优先级队列中，就绪任务数小于2。
 *
 * 值: 0x02000310
 *
 * 解决方案: 检查指定优先级就绪任务，确保就绪任务数大于1。
 */
#define OS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x10)

/*
 * 任务错误码：没有可用的任务控制块资源或配置项中任务裁剪关闭。
 *
 * 值: 0x02000311
 *
 * 解决方案: 打开配置项中任务裁剪开关，并配置足够大的任务资源数。
 */
#define OS_ERRNO_TSK_TCB_UNAVAILABLE OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x11)

/*
 * 任务错误码：操作IDLE任务。
 *
 * 值: 0x02000312
 *
 * 解决方案: 检查是否操作IDLE任务。
 */
#define OS_ERRNO_TSK_OPERATE_IDLE OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x12)

/*
 * 任务错误码：在锁任务的状态下挂起当前任务。
 *
 * 值: 0x03000313
 *
 * 解决方案: 确保任务挂起的时候，任务已经解锁。
 *
 */
#define OS_ERRNO_TSK_SUSPEND_LOCKED OS_ERRNO_BUILD_FATAL(OS_MID_TSK, 0x13)

/*
 * 任务错误码：系统初始化任务激活失败。
 *
 * 值: 0x02000314
 *
 * 解决方案: 查看任务栈是否配置错误。
 *
 */
#define OS_ERRNO_TSK_ACTIVE_FAILED OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x14)

/*
 * 任务错误码：配置的任务数量太多，配置的最大任务个数不能大于254，
 * 总任务个数不包括Idle任务且不能为0。
 *
 * 值: 0x02000315
 *
 * 解决方案: 在任务配置项中将最大任务数改为小于等于254且大于0。
 */
#define OS_ERRNO_TSK_MAX_NUM_NOT_SUITED OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x15)

/*
 * 任务错误码：获取任务信息时，用户实际欲获取任务数为0。
 *
 * 值: 0x02000316
 *
 * 解决方案: 获取任务信息时，用户实际输入的欲获取任务数不为0。
 */
#define OS_ERRNO_TSK_INPUT_NUM_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x16)

/*
 * 任务错误码：用户配置的任务栈首地址未16字节对齐。
 *
 * 值: 0x02000317
 *
 * 解决方案: 配置进来任务栈首地址需16字节对齐。
 */
#define OS_ERRNO_TSK_STACKADDR_NOT_ALIGN OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x17)

/*
 * 任务错误码：任务正在操作队列。
 *
 * 值: 0x02000318
 *
 * 解决方案: 让被删除的任务得到调度读取完队列数据，即可删除。
 */
#define OS_ERRNO_TSK_QUEUE_DOING OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x18)

/*
 * 任务错误码：任务退出时没有完全释放资源。
 *
 * 值: 0x03000319
 *
 * 解决方案: 任务退出前确保完全释放其占有的资源(如消息，互斥信号量等)。
 *
 */
#define OS_ERRNO_TSK_EXIT_WITH_RESOURCE OS_ERRNO_BUILD_FATAL(OS_MID_TSK, 0x19)

/*
 * 任务错误码：解锁任务之前并未上锁。
 *
 * 值: 0x0200031a
 *
 * 解决方案: 任务上锁解锁必须配对使用，不能不加锁，直接解锁，可能导致误解锁。
 */
#define OS_ERRNO_TSK_UNLOCK_NO_LOCK OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x1a)

/*
 * 任务错误码：指定的任务栈地址太大导致任务栈初始化的时候整形溢出。
 *
 * 值: 0x0200031b
 *
 * 解决方案: 限制任务栈地址大小，确保任务栈初始化地址不大于0xFFFFFFFF。
 */
#define OS_ERRNO_TSK_STACKADDR_TOO_BIG OS_ERRNO_BUILD_ERROR(OS_MID_TSK, 0x1b)

/*
 * 任务ID的类型定义。
 */
typedef U32 TskHandle;

/*
 * 任务优先级的类型定义。
 */
typedef U16 TskPrior;

/*
 * 任务状态的类型定义。
 */
typedef U16 TskStatus;

/*
 * 任务信息结构体
 */
struct TskInfo {
    /* 任务切换时的SP */
    uintptr_t sp;
    /* 任务切换时的PC */
    uintptr_t pc;
    /* 任务状态 */
    TskStatus taskStatus;
    /* 任务优先级 */
    TskPrior taskPrio;
    /* 任务栈的大小 */
    U32 stackSize;
    /* 任务栈的栈顶 */
    uintptr_t topOfStack;
    /* 任务名 */
    char name[OS_TSK_NAME_LEN];
    /* 任务入口函数 */
    void *entry;
    /* 任务控制块地址 */
    void *tcbAddr;
    /* 栈底 */
    uintptr_t bottom;
    /* 栈当前使用的大小 */
    U32 currUsed;
    /* 栈使用的历史峰值 */
    U32 peakUsed;
    /* 栈是否溢出 */
    bool ovf;
    /* 任务上下文 */
    struct TskContext context;
};

/*
 * 任务模块配置信息的结构体定义。
 *
 * 保存任务模块的配置项信息。
 */
struct TskModInfo {
    /* 最大支持的任务数 */
    U32 maxNum;
    /* 缺省的任务栈大小 */
    U32 defaultSize;
    /* Idle任务的任务栈大小 */
    U32 idleStackSize;
    /* 任务栈初始化魔术字 */
    U32 magicWord;
};

/*
 * @brief 任务入口函数类型定义。
 *
 * @par 描述
 * 用户通过任务入口函数类型定义任务入口函数，在任务创建触发之后调用该函数进行任务执行。
 * @attention 无。
 *
 * @param param1 [IN]  类型#uintptr_t，传递给任务处理函数的第一个参数。
 * @param param2 [IN]  类型#uintptr_t，传递给任务处理函数的第二个参数。
 * @param param3 [IN]  类型#uintptr_t，传递给任务处理函数的第三个参数。
 * @param param4 [IN]  类型#uintptr_t，传递给任务处理函数的第四个参数。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef void (*TskEntryFunc)(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

/*
 * @brief 任务创建钩子函数类型定义。
 *
 * @par 描述
 * 用户通过任务创建钩子函数类型定义任务创建钩子，在系统创建任务时，调用该钩子。
 * @attention 无。
 *
 * @param taskPid [IN]  类型#TskHandle，新创建任务的PID。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef U32(*TskCreateHook)(TskHandle taskPid);

/*
 * @brief 任务删除钩子函数类型定义。
 *
 * @par 描述
 * 用户通过任务删除钩子函数类型定义任务删除钩子，在系统对任务进行删除时，调用该钩子。
 * @attention 无。
 *
 * @param taskPid [IN]  类型#TskHandle，删除任务的PID。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef U32(*TskDeleteHook)(TskHandle taskPid);

/*
 * @brief 任务切换钩子函数类型定义。
 *
 * @par 描述
 * 用户通过任务切换钩子函数类型定义任务切换钩子，在系统对任务进行切换时，调用该钩子。
 * @attention 无。
 *
 * @param lastTaskPid [IN]  类型#TskHandle，上一个任务的PID。
 * @param nextTaskPid [IN]  类型#TskHandle，下一个任务的PID。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef U32(*TskSwitchHook)(TskHandle lastTaskPid, TskHandle nextTaskPid);

/*
 * 空任务ID。
 *
 * 调用PRT_TaskYield时，使用OS_TSK_NULL_ID，由OS选择就绪队列中的第一个任务。
 */
#define OS_TSK_NULL_ID 0xFFFFFFFF

/*
 * 任务栈信息的结构体定义。
 *
 * 保存任务栈的信息。
 */
struct TskStackInfo {
    /* 栈顶 */
    uintptr_t top;
    /* 栈底 */
    uintptr_t bottom;
    /* 栈当前SP指针值 */
    uintptr_t sp;
    /* 栈当前使用的大小 */
    U32 currUsed;
    /* 栈使用的历史峰值 */
    U32 peakUsed;
    /* 栈是否溢出 */
    bool ovf;
};

/*
 * 任务创建参数的结构体定义。
 *
 * 传递任务创建时指定的参数信息。
 */
struct TskInitParam {
    /* 任务入口函数 */
    TskEntryFunc taskEntry;
    /* 任务优先级 */
    TskPrior taskPrio;
    U16 reserved;
    /* 任务参数，最多4个 */
    uintptr_t args[4];
    /* 任务栈的大小 */
    U32 stackSize;
    /* 任务名 */
    char *name;
    /*
     * 本任务的任务栈独立配置起始地址，用户必须对该成员进行初始化，
     * 若配置为0表示从系统内部空间分配，否则用户指定栈起始地址
     */
    uintptr_t stackAddr;
};

/*
 * @brief 创建任务，但不激活任务。
 *
 * @par 描述
 * 创建一个任务。该任务不加入就绪队列，只处于挂起状态，用户需要激活该任务需要通过调用PRT_TaskResume函数将其激活。
 *
 * @attention
 * <ul>
 * <li>任务创建时，会对之前自删除任务的任务控制块和任务栈进行回收，用户独立配置的任务栈除外。</li>
 * <li>任务名的最大长度为16字节，含结束符。</li>
 * <li>同一核内任务名不允许重复。</li>
 * <li>若指定的任务栈大小为0，则使用配置项#OS_TSK_DEFAULT_STACK_SIZE指定的默认的任务栈大小。</li>
 * <li>任务栈的大小必须按16字节大小对齐。确定任务栈大小的原则是，够用就行：多了浪费，少了任务栈溢出。</li>
 * <li>具体多少取决于需要消耗多少的栈内存，视情况而定：函数调用层次越深，栈内存开销越大，</li>
 * <li>局部数组越大，局部变量越多，栈内存开销也越大。</li>
 * <li>用户配置的任务栈首地址需16字节对齐。</li>
 * <li>用户配置的任务栈空间需由用户保证其合法性，即对可cache空间的地址用户需要保证其任务栈首地址及栈大小cache</li>
 * <li>line对齐，系统不做对齐处理，并在使用后需由用户进行释放。</li>
 * <li>任务创建时，任务创建参数中的任务栈大小配置建议不要使用最小任务栈大小OS_TSK_MIN_STACK_SIZE，</li>
 * <li>该项只是包含任务上下文预留的栈空间，任务调度时额外的任务开销需要由用户自行保证足够的任务栈空间配置。</li>
 * </ul>
 *
 * @param taskPid   [OUT] 类型#TskHandle *，保存任务PID。
 * @param initParam [IN]  类型#struct TskInitParam *，任务创建参数，
 * 其结构体中的成员参数stackAddr传入时必须进行初始化，若不采用用户配置的独立任务栈进行栈空间分配，
 * 该成员必须初始化为0。
 *
 * @retval #OS_OK  0x00000000，任务创建成功。
 * @retval #其它值，创建失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskDelete | PRT_TaskCreateHookAdd | PRT_TaskCreate
 */
extern U32 PRT_TaskCreate(TskHandle *taskPid, struct TskInitParam *initParam);

/*
 * @brief 恢复任务。
 *
 * @par 描述
 * 恢复挂起的任务。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>若任务仍处于延时、阻塞态，则只是取消挂起态，并不加入就绪队列。</li>
 * </ul>
 *
 * @param taskPid [IN]  类型#TskHandle，任务PID。
 *
 * @retval #OS_OK  0x00000000，恢复任务成功。
 * @retval #其它值，恢复任务失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskSuspend
 */
extern U32 PRT_TaskResume(TskHandle taskPid);

/*
 * @brief 挂起任务。
 *
 * @par 描述
 * 挂起指定的任务，任务将从就绪队列中被删除。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>若为当前任务且已锁任务，则不能被挂起。</li>
 * <li>IDLE任务不能被挂起。</li>
 * </ul>
 *
 * @param taskPid [IN]  类型#TskHandle，任务PID。
 *
 * @retval #OS_OK  0x00000000，挂起任务成功。
 * @retval #其它值，挂起任务失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskResume | PRT_TaskLock
 */
extern U32 PRT_TaskSuspend(TskHandle taskPid);

/*
 * @brief 删除任务。
 *
 * @par 描述
 * 删除指定的任务，释放任务栈和任务控制块资源。
 *
 * @attention
 * <ul>
 * <li>若为自删除，则任务控制块和任务栈在下一次创建任务时才回收。</li>
 * <li>对于任务自删除，处理该任务相关的信号量和接收到的消息会强制删除。</li>
 * <li>任务自删除时，删除钩子不允许进行pend信号量、挂起等操作。</li>
 * </ul>
 *
 * @param taskPid [IN]  类型#TskHandle，任务PID。
 *
 * @retval #OS_OK  0x00000000，删除任务成功。
 * @retval #其它值，删除任务失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskCreate | PRT_TaskDeleteHookAdd
 */
extern U32 PRT_TaskDelete(TskHandle taskPid);

/*
 * @brief 延迟正在运行的任务。
 *
 * @par 描述
 * 延迟当前运行任务的执行。任务延时等待指定的Tick数后，重新参与调度。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>硬中断处理中，或已锁任务，则延时操作失败。</li>
 * <li>若传入参数0，且未锁任务调度，则顺取同优先级队列中的下一个任务执行。如没有同级的就绪任务，</li>
 * <li>则不发生任务调度，继续执行原任务。</li>
 * </ul>
 *
 * @param tick [IN]  类型#U32，延迟的Tick数。
 *
 * @retval #OS_OK  0x00000000，任务延时成功。
 * @retval #其它值，延时任务失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskYield
 */
extern U32 PRT_TaskDelay(U32 tick);

/*
 * @brief 锁任务调度。
 *
 * @par 描述
 * 锁任务调度。若任务调度被锁，则不发生任务切换。
 *
 * @attention
 * <ul>
 * <li>只是锁任务调度，并不关中断，因此任务仍可被中断打断。</li>
 * <li>执行此函数则锁计数值加1，解锁则锁计数值减1。因此，必须与#PRT_TaskUnlock配对使用。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskUnlock
 */
extern void PRT_TaskLock(void);

/*
 * @brief 解锁任务调度。
 *
 * @par 描述
 * 任务锁计数值减1。若嵌套加锁，则只有锁计数值减为0才真正的解锁了任务调度。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>执行此函数则锁计数值加1，解锁则锁计数值减1。因此，必须与#PRT_TaskLock配对使用。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskLock
 */
extern void PRT_TaskUnlock(void);

/*
 * @brief 获取当前任务PID。
 *
 * @par 描述
 * 获取处于运行态的任务PID。
 *
 * @attention
 * <ul>
 * <li>硬中断处理中，也可获取当前任务PID，即被中断打断的任务。</li>
 * <li>在任务切换钩子处理中调用时，获取的是切入任务的ID。</li>
 * </ul>
 *
 * @param taskPid [OUT] 类型#TskHandle *，保存任务PID。
 *
 * @retval #OS_OK  0x00000000，获取成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskGetStatus | PRT_TaskGetInfo
 */
extern U32 PRT_TaskSelf(TskHandle *taskPid);

/*
 * @brief 获取任务状态。
 *
 * @par 描述
 * 获取指定任务的状态。
 *
 * @attention 无
 *
 * @param taskPid [IN]  类型#TskHandle，任务PID。
 *
 * @retval #(TskStatus)OS_INVALID    返回失败。
 * @retval #任务状态 返回成功。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskGetInfo
 */
extern TskStatus PRT_TaskGetStatus(TskHandle taskPid);

/*
 * @brief 获取任务信息。
 *
 * @par 描述
 * 获取指定任务的信息。
 *
 * @attention
 * <ul>
 * <li>若获取当前任务的信息，则只在硬中断、异常的处理中才有意义，</li>
 * <li>由于任务切换时，上下文信息也保存在任务栈中，因此任务信息中的SP是保存上下文之后的实际的SP值。</li>
 * </ul>
 *
 * @param taskPid  [IN]  类型#TskHandle，任务PID。
 * @param taskInfo [OUT] 类型#struct TskInfo *，保存任务信息。
 *
 * @retval #OS_OK  0x00000000，获取成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskGetStatus
 */
extern U32 PRT_TaskGetInfo(TskHandle taskPid, struct TskInfo *taskInfo);

/*
 * @brief 获取优先级。
 *
 * @par 描述
 * 获取指定任务的优先级。
 *
 * @attention 无
 *
 * @param taskPid  [IN]  类型#TskHandle，任务PID。
 * @param taskPrio [OUT] 类型#TskPrior *，保存任务优先级指针。
 *
 * @retval #OS_OK  0x00000000，获取成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskSetPriority
 */
extern U32 PRT_TaskGetPriority(TskHandle taskPid, TskPrior *taskPrio);

/*
 * @brief 设置优先级。
 *
 * @par 描述
 * 设置指定任务的优先级。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>若设置的优先级高于当前运行的任务，则可能引发任务调度。</li>
 * <li>若调整当前运行任务的优先级，同样可能引发任务调度。</li>
 * <li>若任务发生优先级继承，或者任务阻塞在互斥信号量或优先级唤醒模式的信号量上，不可以设置任务的优先级。</li>
 * </ul>
 *
 * @param taskPid  [IN]  类型#TskHandle，任务PID。
 * @param taskPrio [IN]  类型#TskPrior，任务优先级。
 *
 * @retval #OS_OK  0x00000000，设置成功。
 * @retval #其它值，设置失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskGetPriority
 */
extern U32 PRT_TaskSetPriority(TskHandle taskPid, TskPrior taskPrio);

/*
 * @brief 查询本核指定任务正在PEND的信号量。
 *
 * @par 描述
 * 根据任务状态和任务控制块，判断任务是否在PEND信号量，以及PEND的信号量ID。
 *
 * @attention
 * <ul>
 * <li>用户应先判断PEND状态，状态为0表明任务没有被信号量阻塞。</li>
 * </ul>
 *
 * @param taskId    [IN]  类型#TskHandle，任务PID。
 * @param semId     [OUT] 类型#U16 *，任务PEND的信号量ID或者#OS_INVALID。
 * @param pendState [OUT] 类型#U16 *，任务的PEND状态：0，#OS_TSK_FSEM_PEND，#OS_TSK_PEND, #OS_TSK_MCSEM_PEND。
 *
 * @retval #OS_OK  0x00000000，查询成功。
 * @retval #其它值，查询失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_SemGetPendList
 */
extern U32 PRT_TaskGetPendSem(TskHandle taskId, U16 *semId, U16 *pendState);

/*
 * @brief 查询任务名。
 *
 * @par 描述
 * 根据任务PID，查询任务名。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>不能查询ID不合法的任务名。</li>
 * <li>若查询没有创建的任务名，查询失败。</li>
 * </ul>
 *
 * @param taskId  [IN]  类型#TskHandle，任务ID。
 * @param name [OUT] 类型#char **，保存任务名字符串的首地址。
 *
 * @retval #OS_OK  0x00000000，查询成功。
 * @retval #其它值，查询失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see
 */
extern U32 PRT_TaskGetName(TskHandle taskId, char **name);

/*
 * @brief 修改任务名。
 *
 * @par 描述
 * 根据任务PID，修改任务名。
 *
 * @attention
 * <ul>
 * <li>在osStart之前不能调用该接口。</li>
 * <li>不能修改ID不合法的任务名。</li>
 * <li>若修改没有创建的任务名，修改失败。</li>
 * </ul>
 *
 * @param taskId  [IN]  类型#TskHandle，任务ID。
 * @param name [OUT] 类型#char *，保存任务名字符串的首地址。
 *
 * @retval #OS_OK  0x00000000，修改成功。
 * @retval #其它值，修改失败。
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see
 */
extern U32 PRT_TaskSetName(TskHandle taskId, const char *name);

/*
 * @brief 注册任务切换钩子。
 *
 * @par 描述
 * 注册任务切换钩子函数。钩子函数在切入新任务前被调用。
 *
 * @attention
 * <ul>
 * <li>不同钩子函数间执行的先后顺序，不应当存在依赖关系。</li>
 * <li>不应在任务切换钩子里进行可能引起任务调度的处理，如：任务延时、P信号量等。</li>
 * </ul>
 *
 * @param hook [IN]  类型#TskSwitchHook，任务切换钩子函数。
 *
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskDelSwitchHook | PRT_TaskDeleteHookAdd | PRT_TaskCreateHookAdd
 */
extern U32 PRT_TaskAddSwitchHook(TskSwitchHook hook);

/*
 * @brief 取消任务切换钩子。
 *
 * @par 描述
 * 取消指定的任务切换钩子。钩子函数在切入新任务前被调用。
 *
 * @attention  无
 *
 * @param hook [IN]  类型#TskSwitchHook，任务切换钩子函数。
 *
 * @par 依赖
 * <ul><li>prt_task.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_TaskAddSwitchHook
 */
extern U32 PRT_TaskDelSwitchHook(TskSwitchHook hook);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_TASK_H */
