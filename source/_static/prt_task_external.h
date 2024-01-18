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
 * Description: 任务模块的内部头文件
 */
#ifndef PRT_TASK_EXTERNAL_H
#define PRT_TASK_EXTERNAL_H

#include "prt_typedef.h"

#include "prt_task.h"
#include "list_types.h"
#include "prt_list_external.h"

// struct TagOsRunQue {
//     U32 taskReadyListBitMap;
//     /* 优先级bit位表 */
//     U32 tskChildBitMap[OS_GET_WORD_NUM_BY_PRIONUM(OS_TSK_NUM_OF_PRIORITIES)];
//     struct TagListObject readyList[OS_TSK_NUM_OF_PRIORITIES];
// };
#define TagOsRunQue TagListObject //简单实现

/*
 * 任务线程及进程控制块的结构体统一定义。
 */
struct TagTskCb {
    /* 当前任务的SP */
    void *stackPointer;
    /* 任务状态,后续内部全改成U32 */
    U32 taskStatus;
    /* 任务的运行优先级 */
    TskPrior priority;
    /* 任务栈配置标记 */
    U16 stackCfgFlg;
    /* 任务栈大小 */
    U32 stackSize;
    TskHandle taskPid;

    /* 任务栈顶 */
    uintptr_t topOfStack;

    /* 任务入口函数 */
    TskEntryFunc taskEntry;
    /* 任务Pend的信号量指针 */
    void *taskSem;

    /* 任务的参数 */
    uintptr_t args[4];
#if (defined(OS_OPTION_TASK_INFO))
    /* 存放任务名 */
    char name[OS_TSK_NAME_LEN];
#endif
    /* 信号量链表指针 */
    struct TagListObject pendList;
    /* 任务延时链表指针 */
    struct TagListObject timerList;
    /* 持有互斥信号量链表 */
    struct TagListObject semBList;
    /* 记录条件变量的等待线程 */
    struct TagListObject condNode;
#if defined(OS_OPTION_LINUX)
    /* 等待队列指针 */
    struct TagListObject waitList;
#endif
#if defined(OS_OPTION_EVENT)
    /* 任务事件 */
    U32 event;
    /* 任务事件掩码 */
    U32 eventMask;
#endif
    /* 任务记录的最后一个错误码 */
    U32 lastErr;
    /* 任务恢复的时间点(单位Tick) */
    U64 expirationTick;

#if defined(OS_OPTION_NUTTX_VFS)
    struct filelist tskFileList;
#if defined(CONFIG_FILE_STREAM)
    struct streamlist ta_streamlist;
#endif
#endif
};

typedef void (*TskCoresleep)(void);
typedef void (*TaskNameGetFunc)(U32 taskId, char **taskName);
typedef U32 (*TaskNameAddFunc)(U32 taskId, const char *name);

extern U16 g_uniTaskLock;
extern TskHandle g_idleTaskId;
extern struct TagOsRunQue g_runQueue;
extern struct TagTskCb *g_runningTask;
extern struct TagTskCb *g_highestTask;
extern struct TagOsTskSortedDelayList g_tskSortedDelay;

extern U32 g_tskMaxNum;
extern U32 g_tskBaseId;
extern struct TagTskCb *g_tskCbArray;
extern struct TskModInfo g_tskModInfo;
extern struct TagTskMonNode *g_tskMonList;

extern TskEntryFunc g_tskIdleEntry;
extern TaskNameAddFunc g_taskNameAdd;
extern TaskNameGetFunc g_taskNameGet;

extern volatile TskCoresleep g_taskCoreSleep;

#if defined(OS_OPTION_POWEROFF)
typedef void (*PowerOffFuncT)(void);
extern void OsPowerOffSetFlag(void);
extern void OsPowerOffFuncHook(PowerOffFuncT powerOffFunc);
extern void OsCpuPowerOff(void); /* hook之前异常, 需实现该函数 */
#endif

#define OS_TSK_PARA_0   0
#define OS_TSK_PARA_1   1
#define OS_TSK_PARA_2   2
#define OS_TSK_PARA_3   3

/* 定义任务的缺省优先级 */
#define OS_TSK_DEFAULT_PRIORITY 20
#define OS_TSK_PRIO_BIT_MAP_POW 5
#define OS_TSK_STACK_TOP_MAGIC  0xAAAAAAAA

#define OS_TSK_PRIO_RDY_BIT  0x80000000U

#define OS_TASK_LOCK_DATA          g_uniTaskLock
#define IDLE_TASK_ID               g_idleTaskId
#define RUNNING_TASK               g_runningTask
#define TSK_GET_INDEX(taskId)      ((taskId) - g_tskBaseId)

/* 内核进程的进程及线程调度控制块使用同一类型 */
#define OS_MAX_TCB_NUM             (g_tskMaxNum + 1 + 1)  // 1个IDLE，1个无效任务

#define OS_TSK_DELAY_LOCKED_DETACH(task)            ListDelete(&(task)->timerList)
#define CHECK_TSK_PID_OVERFLOW(taskId)              (TSK_GET_INDEX(taskId) >= (g_tskMaxNum + 1))

/* 定义任务的缺省任务栈大小 */
#define OS_PST_ZOMBIE_TASK             (&g_tskCbArray[OS_MAX_TCB_NUM - 1])
#define TSK_IS_UNUSED(tsk)             ((tsk)->taskStatus == OS_TSK_UNUSED)
#define TSK_STATUS_TST(tsk, statBit)   (((tsk)->taskStatus & (statBit)) != 0)
#define TSK_STATUS_CLEAR(tsk, statBit) ((tsk)->taskStatus &= ~(statBit))
#define TSK_STATUS_SET(tsk, statBit)   ((tsk)->taskStatus |= (statBit))
#define OS_TSK_LOCK()                  (OS_TASK_LOCK_DATA++)
#define GET_TCB_PEND(ptr)              LIST_COMPONENT(ptr, struct TagTskCb, pendList)
#define GET_TCB_HANDLE(taskPid)        (((struct TagTskCb *)g_tskCbArray) + TSK_GET_INDEX(taskPid))
// 保留一个idle task。最大任务handle为FE，FF表示硬中断线程。
#define MAX_TASK_NUM                   ((1U << OS_TSK_TCB_INDEX_BITS) - 2)  // 254
#define OS_TSK_BLOCK                   (OS_TSK_DELAY | OS_TSK_PEND | OS_TSK_SUSPEND  | OS_TSK_QUEUE_PEND | \
        OS_TSK_EVENT_PEND | OS_TSK_WAITQUEUE_PEND)

#if defined(OS_OPTION_LINUX)
#define KTHREAD_TSK_STATE_TST(tsk, tskState)   (((tsk)->kthreadTsk != NULL) && ((tsk)->kthreadTsk->state == (tskState)))
#define KTHREAD_TSK_STATE_SET(tsk, tskState)   ((tsk)->kthreadTsk != NULL ? (tsk)->kthreadTsk->state = (tskState) : 0)
#endif

#define OS_TSK_SUSPEND_READY_BLOCK (OS_TSK_SUSPEND)
// 设置任务优先级就绪链表主BitMap中Bit位，每32个优先级对应一个BIT位，即Bit0(优先级0~31),Bit1(优先级32~63),依次类推。
#define OS_SET_RDY_TSK_BIT_MAP(priority) \
        (OS_TSK_PRIO_RDY_BIT >> ((priority) >> OS_TSK_PRIO_BIT_MAP_POW))
// 清除任务优先级就绪链表主BitMap中Bit位，每32个优先级对应一个BIT位，即Bit0(优先级0~31),Bit1(优先级32~63),依次类推。
#define OS_CLR_RDY_TSK_BIT_MAP(priority) \
        (~(OS_TSK_PRIO_RDY_BIT >> ((priority) >> OS_TSK_PRIO_BIT_MAP_POW)))

// 设置任务优先级就绪链表子BitMap中Bit位，每个优先级对应一个BIT位。
#define OS_SET_CHILD_BIT_MAP(priority) \
        (OS_TSK_PRIO_RDY_BIT >> ((priority) % OS_WORD_BIT_NUM))
// 清除任务优先级就绪链表子BitMap中Bit位，每个优先级对应一个BIT位。
#define OS_CLR_CHILD_BIT_MAP(priority) \
        (~(OS_TSK_PRIO_RDY_BIT >> ((priority) % OS_WORD_BIT_NUM)))

extern void OsTaskScan(void);
extern void OsTskSchedule(void);
extern void OsTskEntry(TskHandle taskId);
extern void OsTaskExit(struct TagTskCb *tsk);
extern void OsTskReadyAdd(struct TagTskCb *task);
extern void OsTskReadyDel(struct TagTskCb *taskCb);
extern void OsTskSwitchHookCaller(U32 prevPid, U32 nextPid);
extern void OsTskTimerAdd(struct TagTskCb *taskCb, uintptr_t timeout);

extern U32 OsTskMaxNumGet(void);
extern U32 OsTaskDelete(TskHandle taskPid);
extern U32 OsTaskCreateOnly(TskHandle *taskPid, struct TskInitParam *initParam);

extern void OsTskScheduleFast(void);
extern void OsTskScheduleFastPs(uintptr_t intSave);

/*
 * 模块内内联函数定义
 */
OS_SEC_ALW_INLINE INLINE void OsTskHighestSet(void)
{
    struct TagTskCb *taskCb = NULL;
    struct TagTskCb *savedTaskCb = NULL;

    // 遍历g_runQueue队列，查找优先级最高的任务
    LIST_FOR_EACH(taskCb, &g_runQueue, struct TagTskCb, pendList) {
        // 第一个任务，直接保存到savedTaskCb
        if(savedTaskCb == NULL) {
            savedTaskCb = taskCb;
            continue;
        }
        // 比较优先级，值越小优先级越高
        if(taskCb->priority < savedTaskCb->priority){
            savedTaskCb = taskCb;
        }
    }

    g_highestTask = savedTaskCb;
}

#endif /* PRT_TASK_EXTERNAL_H */
