实验七 信号量与同步 
=====================

信号量结构初始化
--------------------------

新建 lab7/src/include/prt_sem_external.h 头文件

.. code-block:: C
    :linenos:

    #ifndef PRT_SEM_EXTERNAL_H
    #define PRT_SEM_EXTERNAL_H

    #include "prt_sem.h"
    #include "prt_task_external.h"
    #if defined(OS_OPTION_POSIX)
    #include "bits/semaphore_types.h"
    #endif

    #define OS_SEM_UNUSED 0
    #define OS_SEM_USED   1

    #define SEM_PROTOCOL_PRIO_INHERIT 1
    #define SEM_TYPE_BIT_WIDTH        0x4U
    #define SEM_PROTOCOL_BIT_WIDTH    0x8U

    #define OS_SEM_WITH_LOCK_FLAG    1
    #define OS_SEM_WITHOUT_LOCK_FLAG 0

    #define MAX_POSIX_SEMAPHORE_NAME_LEN    31

    #define GET_SEM_LIST(ptr) LIST_COMPONENT(ptr, struct TagSemCb, semList)
    #define GET_SEM(semid) (((struct TagSemCb *)g_allSem) + (semid))
    #define GET_SEM_TSK(semid) (((SEM_TSK_S *)g_semTsk) + (semid))
    #define GET_TSK_SEM(tskid) (((TSK_SEM_S *)g_tskSem) + (tskid))
    #define GET_SEM_TYPE(semType) (U32)((semType) & ((1U << SEM_TYPE_BIT_WIDTH) - 1))
    #define GET_MUTEX_TYPE(semType) (U32)(((semType) >> SEM_TYPE_BIT_WIDTH) & ((1U << SEM_TYPE_BIT_WIDTH) - 1))
    #define GET_SEM_PROTOCOL(semType) (U32)((semType) >> SEM_PROTOCOL_BIT_WIDTH)

    struct TagSemCb {
        /* 是否使用 OS_SEM_UNUSED/OS_SEM_USED */
        U16 semStat;
        /* 核内信号量索引号 */
        U16 semId;
    #if defined(OS_OPTION_SEM_RECUR_PV)
        /* 二进制互斥信号量递归P计数，计数型信号量和二进制同步模式信号量无效 */
        U32 recurCount;
    #endif
        /* 当该信号量已用时，其信号量计数 */
        U32 semCount;
        /* 挂接阻塞于该信号量的任务 */
        struct TagListObject semList;
        /* 挂接任务持有的互斥信号量，计数型信号量信号量无效 */
        struct TagListObject semBList;

        /* Pend到该信号量的线程ID */
        U32 semOwner;
        /* 信号量唤醒阻塞任务的方式 */
        enum SemMode semMode;
        /* 信号量，计数型或二进制 */
        U32 semType;
    #if defined(OS_OPTION_POSIX)
        /* 信号量名称 */
        char name[MAX_POSIX_SEMAPHORE_NAME_LEN + 1]; // + \0
        /* sem_open 句柄 */
        sem_t handle;
    #endif
    };

    /* 模块间全局变量声明 */
    extern U16 g_maxSem;

    /* 指向核内信号量控制块 */
    extern struct TagSemCb *g_allSem;

    extern U32 OsSemCreate(U32 count, U32 semType, enum SemMode semMode, SemHandle *semHandle, U32 cookie);
    extern bool OsSemBusy(SemHandle semHandle);

    #endif /* PRT_SEM_EXTERNAL_H */


新建 src/kernel/sem/prt_sem_init.c 文件。

.. code-block:: c
    :linenos:

    #include "prt_sem_external.h"
    #include "os_attr_armv8_external.h"
    #include "os_cpu_armv8_external.h"

    OS_SEC_BSS struct TagListObject g_unusedSemList;
    OS_SEC_BSS struct TagSemCb *g_allSem;

    extern void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, U8 alignPow);
    /*
    * 描述：信号量初始化
    */
    OS_SEC_L4_TEXT U32 OsSemInit(void)
    {
        struct TagSemCb *semNode = NULL;
        U32 idx;
        U32 ret = OS_OK;

        g_allSem = (struct TagSemCb *)OsMemAllocAlign((U32)OS_MID_SEM,
                                                    0,
                                                    4096,
                                                    OS_SEM_ADDR_ALLOC_ALIGN);

        if (g_allSem == NULL) {
            return OS_ERRNO_SEM_NO_MEMORY;
        }

        g_maxSem = 4096 / sizeof(struct TagSemCb);

        char *cg_allSem = (char *)g_allSem;
        for(int i = 0; i < 4096; i++)
            cg_allSem[i] = 0;

        INIT_LIST_OBJECT(&g_unusedSemList);
        for (idx = 0; idx < g_maxSem; idx++) {
            semNode = ((struct TagSemCb *)g_allSem) + idx; //指针操作
            semNode->semId = (U16)idx;
            ListTailAdd(&semNode->semList, &g_unusedSemList);
        }

        return ret;
    }

    /*
    * 描述：创建一个信号量
    */
    OS_SEC_L4_TEXT U32 OsSemCreate(U32 count, U32 semType, enum SemMode semMode,
                                SemHandle *semHandle, U32 cookie)
    {
        uintptr_t intSave;
        struct TagSemCb *semCreated = NULL;
        struct TagListObject *unusedSem = NULL;
        (void)cookie;

        if (semHandle == NULL) {
            return OS_ERRNO_SEM_PTR_NULL;
        }

        intSave = OsIntLock();

        if (ListEmpty(&g_unusedSemList)) {
            OsIntRestore(intSave);
            return OS_ERRNO_SEM_ALL_BUSY;
        }

        /* 在空闲链表中取走一个控制节点 */
        unusedSem = OS_LIST_FIRST(&(g_unusedSemList));
        ListDelete(unusedSem);

        /* 获取到空闲节点对应的信号量控制块，并开始填充控制块 */
        semCreated = (GET_SEM_LIST(unusedSem));
        semCreated->semCount = count;
        semCreated->semStat = OS_SEM_USED;
        semCreated->semMode = semMode;
        semCreated->semType = semType;
        semCreated->semOwner = OS_INVALID_OWNER_ID;
        if (GET_SEM_TYPE(semType) == SEM_TYPE_BIN) {
            INIT_LIST_OBJECT(&semCreated->semBList);
    #if defined(OS_OPTION_SEM_RECUR_PV)
            if (GET_MUTEX_TYPE(semType) == PTHREAD_MUTEX_RECURSIVE) {
                semCreated->recurCount = 0;
            }
    #endif
        }

        INIT_LIST_OBJECT(&semCreated->semList);
        *semHandle = (SemHandle)semCreated->semId;

        OsIntRestore(intSave);
        return OS_OK;
    }

    /*
    * 描述：创建一个信号量
    */
    OS_SEC_L4_TEXT U32 PRT_SemCreate(U32 count, SemHandle *semHandle)
    {
        U32 ret;

        if (count > OS_SEM_COUNT_MAX) {
            return OS_ERRNO_SEM_OVERFLOW;
        }

        ret = OsSemCreate(count, SEM_TYPE_COUNT, SEM_MODE_FIFO, semHandle, (U32)(uintptr_t)semHandle);
        return ret;
    }

在 src/bsp/os_cpu_armv8_external.h 加入 定义

.. code-block:: c
    :linenos:

    #define OS_SEM_ADDR_ALLOC_ALIGN 2U //按2的幂对齐，即2^2=4字节


新建 src/kernel/sem/prt_sem.c 文件。

.. code-block:: c
    :linenos:

    #include "prt_sem_external.h"
    #include "prt_asm_cpu_external.h"
    #include "os_attr_armv8_external.h"
    #include "os_cpu_armv8_external.h"

    /* 核内信号量最大个数 */
    OS_SEC_BSS U16 g_maxSem;


    OS_SEC_ALW_INLINE INLINE U32 OsSemPostErrorCheck(struct TagSemCb *semPosted, SemHandle semHandle)
    {
        (void)semHandle;
        /* 检查信号量控制块是否UNUSED，排除大部分错误场景 */
        if (semPosted->semStat == OS_SEM_UNUSED) {
            return OS_ERRNO_SEM_INVALID;
        }

        /* post计数型信号量的错误场景, 释放计数型信号量且信号量计数大于最大计数 */
        if ((semPosted)->semCount >= OS_SEM_COUNT_MAX) {
            return OS_ERRNO_SEM_OVERFLOW;
        }

        return OS_OK;
    }


    /*
    * 描述：把当前运行任务挂接到信号量链表上
    */
    OS_SEC_L0_TEXT void OsSemPendListPut(struct TagSemCb *semPended, U32 timeOut)
    {
        struct TagTskCb *curTskCb = NULL;
        struct TagTskCb *runTsk = RUNNING_TASK;
        struct TagListObject *pendObj = &runTsk->pendList;

        OsTskReadyDel((struct TagTskCb *)runTsk);

        runTsk->taskSem = (void *)semPended;

        TSK_STATUS_SET(runTsk, OS_TSK_PEND);
        /* 根据唤醒方式挂接此链表，同优先级再按FIFO子顺序插入 */
        if (semPended->semMode == SEM_MODE_PRIOR) {
            LIST_FOR_EACH(curTskCb, &semPended->semList, struct TagTskCb, pendList) {
                if (curTskCb->priority > runTsk->priority) {
                    ListTailAdd(pendObj, &curTskCb->pendList);
                    // goto TIMER_ADD;
                    return;
                }
            }
        }
        /* 如果到这里，说明是FIFO方式；或者是优先级方式且挂接首个节点或者挂接尾节点 */
        ListTailAdd(pendObj, &semPended->semList);

    }

    /*
    * 描述：从非空信号量链表上摘首个任务放入到ready队列
    */
    OS_SEC_L0_TEXT struct TagTskCb *OsSemPendListGet(struct TagSemCb *semPended)
    {
        struct TagTskCb *taskCb = GET_TCB_PEND(LIST_FIRST(&(semPended->semList)));

        ListDelete(LIST_FIRST(&(semPended->semList)));
        /* 如果阻塞的任务属于定时等待的任务时候，去掉其定时等待标志位，并将其从去除 */
        if (TSK_STATUS_TST(taskCb, OS_TSK_TIMEOUT)) {
            OS_TSK_DELAY_LOCKED_DETACH(taskCb);
        }

        /* 必须先去除 OS_TSK_TIMEOUT 态，再入队[睡眠时是先出ready队，再置OS_TSK_TIMEOUT态] */
        TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT | OS_TSK_PEND);
        taskCb->taskSem = NULL;
        /* 如果去除信号量阻塞位后，该任务不处于阻塞态则将该任务挂入就绪队列并触发任务调度 */
        if (!TSK_STATUS_TST(taskCb, OS_TSK_SUSPEND)) {
            OsTskReadyAddBgd(taskCb);
        }

        return taskCb;
    }

    OS_SEC_L0_TEXT U32 OsSemPendParaCheck(U32 timeout)
    {
        if (timeout == 0) {
            return OS_ERRNO_SEM_UNAVAILABLE;
        }

        if (OS_TASK_LOCK_DATA != 0) {
            return OS_ERRNO_SEM_PEND_IN_LOCK;
        }
        return OS_OK;
    }

    OS_SEC_L0_TEXT bool OsSemPendNotNeedSche(struct TagSemCb *semPended, struct TagTskCb *runTsk)
    {
        if (semPended->semCount > 0) {
            semPended->semCount--;
            semPended->semOwner = runTsk->taskPid;

            return TRUE;
        }
        return FALSE;
    }

    /*
    * 描述：指定信号量的P操作
    */
    OS_SEC_L0_TEXT U32 PRT_SemPend(SemHandle semHandle, U32 timeout)
    {
        uintptr_t intSave;
        U32 ret;
        struct TagTskCb *runTsk = NULL;
        struct TagSemCb *semPended = NULL;

        if (semHandle >= (SemHandle)g_maxSem) {
            return OS_ERRNO_SEM_INVALID;
        }

        semPended = GET_SEM(semHandle);

        intSave = OsIntLock();
        if (semPended->semStat == OS_SEM_UNUSED) {
            OsIntRestore(intSave);
            return OS_ERRNO_SEM_INVALID;
        }

        if (OS_INT_ACTIVE) {
            OsIntRestore(intSave);
            return OS_ERRNO_SEM_PEND_INTERR;
        }

        runTsk = (struct TagTskCb *)RUNNING_TASK;

        if (OsSemPendNotNeedSche(semPended, runTsk) == TRUE) {
            OsIntRestore(intSave);
            return OS_OK;
        }

        ret = OsSemPendParaCheck(timeout);
        if (ret != OS_OK) {
            OsIntRestore(intSave);
            return ret;
        }
        /* 把当前任务挂接在信号量链表上 */
        OsSemPendListPut(semPended, timeout);
        if (timeout != OS_WAIT_FOREVER) {
            OsIntRestore(intSave);
            return OS_ERRNO_SEM_FUNC_NOT_SUPPORT;
        } else {
            /* 恢复ps的快速切换 */
            OsTskScheduleFastPs(intSave);
            
        }

        OsIntRestore(intSave);
        return OS_OK;
    }

    OS_SEC_ALW_INLINE INLINE void OsSemPostSchePre(struct TagSemCb *semPosted)
    {
        struct TagTskCb *resumedTask = NULL;

        resumedTask = OsSemPendListGet(semPosted);
        semPosted->semOwner = resumedTask->taskPid;
    }

    /*
    * 描述：判断信号量post是否有效
    * 备注：以下情况表示post无效，返回TRUE: post互斥二进制信号量，若该信号量被嵌套pend或者已处于空闲状态
    */
    OS_SEC_ALW_INLINE INLINE bool OsSemPostIsInvalid(struct TagSemCb *semPosted)
    {
        if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
            /* 释放互斥二进制信号量且信号量已处于空闲状态 */
            if ((semPosted)->semCount == OS_SEM_FULL) {
                return TRUE;
            }
        }
        return FALSE;
    }

    /*
    * 描述：指定信号量的V操作
    */
    OS_SEC_L0_TEXT U32 PRT_SemPost(SemHandle semHandle)
    {
        U32 ret;
        uintptr_t intSave;
        struct TagSemCb *semPosted = NULL;

        if (semHandle >= (SemHandle)g_maxSem) {
            return OS_ERRNO_SEM_INVALID;
        }

        semPosted = GET_SEM(semHandle);
        intSave = OsIntLock();

        ret = OsSemPostErrorCheck(semPosted, semHandle);
        if (ret != OS_OK) {
            OsIntRestore(intSave);
            return ret;
        }

        /* 信号量post无效，不需要调度 */
        if (OsSemPostIsInvalid(semPosted) == TRUE) {
            OsIntRestore(intSave);
            return OS_OK;
        }

        /* 如果有任务阻塞在信号量上，就激活信号量阻塞队列上的首个任务 */
        if (!ListEmpty(&semPosted->semList)) {
            OsSemPostSchePre(semPosted);
            /* 相当于快速切换+中断恢复 */
            OsTskScheduleFastPs(intSave);
        } else {
            semPosted->semCount++;
            semPosted->semOwner = OS_INVALID_OWNER_ID;

        }

        OsIntRestore(intSave);
        return OS_OK;
    }


src/include/prt_task_external.h 加入 OsTskReadyAddBgd()

.. code-block:: c
    :linenos:

    OS_SEC_ALW_INLINE INLINE void OsTskReadyAddBgd(struct TagTskCb *task)
    {
        OsTskReadyAdd(task);
    }

src/kernel/task/prt_task.c 加入 OsTskScheduleFastPs()

.. code-block:: c
    :linenos:

    // src/core/kernel/task/prt_amp_task.c
    /*
    * 描述：如果快速切换后只有中断恢复，使用此接口
    */
    OS_SEC_TEXT void OsTskScheduleFastPs(uintptr_t intSave)
    {
        /* Find the highest task */
        OsTskHighestSet();

        /* In case that running is not highest then reschedule */
        if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
            UNI_FLAG |= OS_FLG_TSK_REQ;

            /* only if there is not HWI or TICK the trap */
            if (OS_INT_INACTIVE) {
                OsTaskTrapFastPs(intSave);
            }
        }
    }

src/bsp/os_cpu_armv8_external.h 加入 OsTaskTrapFastPs()

.. code-block:: C
    :linenos:

    OS_SEC_ALW_INLINE INLINE void OsTaskTrapFastPs(uintptr_t intSave)
    {
        (void)intSave;
        OsTaskTrap();
    }

加入 src/include/prt_sem.h [`下载 <../\_static/prt_sem.h>`_]，该头文件主要是信号量相关的函数声明和宏定义。

.. hint:: 将新增文件加入构建系统

验证
--------------------------

.. code-block:: c
    :linenos:

    #include "prt_typedef.h"
    #include "prt_tick.h"
    #include "prt_task.h"
    #include "prt_sem.h"

    extern U32 PRT_Printf(const char *format, ...);
    extern void PRT_UartInit(void);
    extern U32 OsActivate(void);
    extern U32 OsTskInit(void);
    extern U32 OsSemInit(void);


    static SemHandle sem_sync;


    void Test1TaskEntry()
    {
        PRT_Printf("task 1 run ...\n");
        PRT_SemPost(sem_sync);
        U32 cnt = 5;
        while (cnt > 0) {
            // PRT_TaskDelay(200);
            PRT_Printf("task 1 run ...\n");
            cnt--;
        }
    }

    void Test2TaskEntry()
    {
        PRT_Printf("task 2 run ...\n");

        PRT_SemPend(sem_sync, OS_WAIT_FOREVER);
        U32 cnt = 5;
        while (cnt > 0) {
            // PRT_TaskDelay(100);
            PRT_Printf("task 2 run ...\n");
            cnt--;
        }
    }

    S32 main(void)
    {
        // 任务模块初始化
        OsTskInit();
        OsSemInit(); // 参见demos/ascend310b/config/prt_config.c 系统初始化注册表

        PRT_UartInit();

        PRT_Printf("            _       _ _____      _             _             _   _ _   _ _   _           \n");
        PRT_Printf("  _ __ ___ (_)_ __ (_) ____|   _| | ___ _ __  | |__  _   _  | | | | \\ | | | | | ___ _ __ \n");
        PRT_Printf(" | '_ ` _ \\| | '_ \\| |  _|| | | | |/ _ \\ '__| | '_ \\| | | | | |_| |  \\| | | | |/ _ \\ '__|\n");
        PRT_Printf(" | | | | | | | | | | | |__| |_| | |  __/ |    | |_) | |_| | |  _  | |\\  | |_| |  __/ |   \n");
        PRT_Printf(" |_| |_| |_|_|_| |_|_|_____\\__,_|_|\\___|_|    |_.__/ \\__, | |_| |_|_| \\_|\\___/ \\___|_|   \n");
        PRT_Printf("                                                     |___/                               \n");

        PRT_Printf("ctr-a h: print help of qemu emulator. ctr-a x: quit emulator.\n\n");

        U32 ret;
        ret = PRT_SemCreate(0, &sem_sync);
        if (ret != OS_OK) {
            PRT_Printf("failed to create synchronization sem\n");
            return 1;
        }

        struct TskInitParam param = {0};

        // task 1
        // param.stackAddr = 0;
        param.taskEntry = (TskEntryFunc)Test1TaskEntry;
        param.taskPrio = 35;
        // param.name = "Test1Task";
        param.stackSize = 0x1000; //固定4096，参见prt_task_init.c的OsMemAllocAlign
        
        TskHandle tskHandle1;
        ret = PRT_TaskCreate(&tskHandle1, &param);
        if (ret) {
            return ret;
        }

        ret = PRT_TaskResume(tskHandle1);
        if (ret) {
            return ret;
        }

        // task 2
        // param.stackAddr = 0;
        param.taskEntry = (TskEntryFunc)Test2TaskEntry;
        param.taskPrio = 30;
        // param.name = "Test2Task";
        param.stackSize = 0x1000; //固定4096，参见prt_task_init.c的OsMemAllocAlign

        TskHandle tskHandle2;
        ret = PRT_TaskCreate(&tskHandle2, &param);
        if (ret) {
            return ret;
        }

        ret = PRT_TaskResume(tskHandle2);
        if (ret) {
            return ret;
        }

        // 启动调度
        OsActivate();

        // while(1);
        return 0;

    }



lab7 作业
--------------------------

作业1
^^^^^^^^^^^^^^^^^^^^^^^^^^

各种并发问题模拟，至少3种。