实验六 任务调度
=====================

任务调度是操作系统的核心功能之一。 UniProton实现的是一个单进程支持多线程的操作系统。在UniProton中，一个任务表示一个线程。UniProton中的任务为抢占式调度机制，而非时间片轮转调度方式。高优先级的任务可打断低优先级任务，低优先级任务必须在高优先级任务挂起或阻塞后才能得到调度。

基础数据结构：双向链表
--------------------------

双向链表结构在 src/include/list_types.h 中定义。

.. code-block:: c
    :linenos:

    #ifndef _LIST_TYPES_H
    #define _LIST_TYPES_H

    struct TagListObject {
        struct TagListObject *prev;
        struct TagListObject *next;
    };

    #endif  /* end _LIST_TYPES_H */

此外，在 src/include/prt_list_external.h 中定义了链表各种相关操作。

.. code-block:: c
    :linenos:

    #ifndef PRT_LIST_EXTERNAL_H
    #define PRT_LIST_EXTERNAL_H

    #include "prt_typedef.h"
    #include "list_types.h"

    #define LIST_OBJECT_INIT(object) { \
            &(object), &(object)       \
        }

    #define INIT_LIST_OBJECT(object)   \
        do {                           \
            (object)->next = (object); \
            (object)->prev = (object); \
        } while (0)

    #define LIST_LAST(object) ((object)->prev)
    #define LIST_FIRST(object) ((object)->next)
    #define OS_LIST_FIRST(object) ((object)->next)

    /* list action low level add */
    OS_SEC_ALW_INLINE INLINE void ListLowLevelAdd(struct TagListObject *newNode, struct TagListObject *prev,
                                                struct TagListObject *next)
    {
        newNode->next = next;
        newNode->prev = prev;
        next->prev = newNode;
        prev->next = newNode;
    }

    /* list action add */
    OS_SEC_ALW_INLINE INLINE void ListAdd(struct TagListObject *newNode, struct TagListObject *listObject)
    {
        ListLowLevelAdd(newNode, listObject, listObject->next);
    }

    /* list action tail add */
    OS_SEC_ALW_INLINE INLINE void ListTailAdd(struct TagListObject *newNode, struct TagListObject *listObject)
    {
        ListLowLevelAdd(newNode, listObject->prev, listObject);
    }

    /* list action lowel delete */
    OS_SEC_ALW_INLINE INLINE void ListLowLevelDelete(struct TagListObject *prevNode, struct TagListObject *nextNode)
    {
        nextNode->prev = prevNode;
        prevNode->next = nextNode;
    }

    /* list action delete */
    OS_SEC_ALW_INLINE INLINE void ListDelete(struct TagListObject *node)
    {
        ListLowLevelDelete(node->prev, node->next);

        node->next = NULL;
        node->prev = NULL;
    }

    /* list action empty */
    OS_SEC_ALW_INLINE INLINE bool ListEmpty(const struct TagListObject *listObject)
    {
        return (bool)((listObject->next == listObject) && (listObject->prev == listObject));
    }

    #define OFFSET_OF_FIELD(type, field) ((uintptr_t)((uintptr_t)(&((type *)0x10)->field) - (uintptr_t)0x10))

    #define COMPLEX_OF(ptr, type, field) ((type *)((uintptr_t)(ptr) - OFFSET_OF_FIELD(type, field)))

    /* 根据成员地址得到控制块首地址, ptr成员地址, type控制块结构, field成员名 */
    #define LIST_COMPONENT(ptrOfList, typeOfList, fieldOfList) COMPLEX_OF(ptrOfList, typeOfList, fieldOfList)

    #define LIST_FOR_EACH(posOfList, listObject, typeOfList, field)                                                    \
        for ((posOfList) = LIST_COMPONENT((listObject)->next, typeOfList, field); &(posOfList)->field != (listObject); \
            (posOfList) = LIST_COMPONENT((posOfList)->field.next, typeOfList, field))

    #define LIST_FOR_EACH_SAFE(posOfList, listObject, typeOfList, field)                \
        for ((posOfList) = LIST_COMPONENT((listObject)->next, typeOfList, field);       \
            (&(posOfList)->field != (listObject))&&((posOfList)->field.next != NULL);  \
            (posOfList) = LIST_COMPONENT((posOfList)->field.next, typeOfList, field))

    #endif /* PRT_LIST_EXTERNAL_H */

这里面最有意思的是 LIST_COMPONENT 宏，其作用是根据成员地址得到控制块首地址, ptr成员地址, type控制块结构, field成员名。

LIST_FOR_EACH 和 LIST_FOR_EACH_SAFE 用于遍历链表，主要是简化代码编写。

任务控制块
--------------------------

任务相关的头文件主要包括 src/include/prt_task.h [`下载 <../\_static/prt_task.h>`_] 和 src/include/prt_task_external.h [`下载 <../\_static/prt_task_external.h>`_]两个头文件。此外还会用到 src/include/prt_module.h [`下载 <../\_static/prt_module.h>`_] 和 src/include/prt_errno.h [`下载 <../\_static/prt_errno.h>`_] 两个头文件。 prt_module.h中主要是一些模块ID的定义，而 prt_errno.h 主要是错误类型的相关定义，引入这两个头文件主要是为了保持接口与原版 UniProton 相一致。

prt_task.h 中除了一些相关宏定义外，还定义了任务创建时参数传递的结构体： struct TskInitParam。

.. code-block:: c
    :linenos:

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

prt_task_external.h 中定义了任务调度中最重要的数据结构——任务控制块 struct TagTskCb。

.. code-block:: c
    :linenos:

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

简单起见，我们还将任务运行队列结构 TagOsRunQue 直接定义为双向链表 TagListObject（见上面代码）。 

最后我们引入了 src/include/prt_amp_task_internal.h 

.. code-block:: C
    :linenos:

    #ifndef PRT_AMP_TASK_INTERNAL_H
    #define PRT_AMP_TASK_INTERNAL_H

    #include "prt_task_external.h"
    #include "prt_list_external.h"

    #define OS_TSK_EN_QUE(runQue, tsk, flags) OsEnqueueTaskAmp((runQue), (tsk))
    #define OS_TSK_EN_QUE_HEAD(runQue, tsk, flags) OsEnqueueTaskHeadAmp((runQue), (tsk))
    #define OS_TSK_DE_QUE(runQue, tsk, flags) OsDequeueTaskAmp((runQue), (tsk))

    extern U32 OsTskAMPInit(void);
    extern U32 OsIdleTskAMPCreate(void);

    OS_SEC_ALW_INLINE INLINE void OsEnqueueTaskAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
    {
        ListTailAdd(&tsk->pendList, runQue);
        return;
    }

    OS_SEC_ALW_INLINE INLINE void OsEnqueueTaskHeadAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
    {
        ListAdd(&tsk->pendList, runQue);
        return;
    }

    OS_SEC_ALW_INLINE INLINE void OsDequeueTaskAmp(struct TagOsRunQue *runQue, struct TagTskCb *tsk)
    {
        ListDelete(&tsk->pendList);
        return;
    }

    #endif /* PRT_AMP_TASK_INTERNAL_H */

该头文件中主要是定义了三个内联函数，用于将任务控制块加入运行队列或从运行队列中移除任务控制块。

任务创建
--------------------------

任务创建代码主要在 src/kernel/task/prt_task_init.c 中。 代码比较多，我们分几个部分分别介绍。

相关变量与函数声明
^^^^^^^^^^^^^^^^^^^^^^^^^^

首先是引入必要的头文件。

然后声明了 1 个全局双向链表，并通过 LIST_OBJECT_INIT 宏进行初始化。 g_tskCbFreeList 链表是空闲的任务控制块链表。

最后声明了3个外部函数。

.. code-block:: c
    :linenos:

    #include "list_types.h"
    #include "os_attr_armv8_external.h"
    #include "prt_list_external.h"
    #include "prt_task.h"
    #include "prt_task_external.h"
    #include "prt_asm_cpu_external.h"
    #include "os_cpu_armv8_external.h"
    #include "prt_config.h"


    /* Unused TCBs and ECBs that can be allocated. */
    OS_SEC_DATA struct TagListObject g_tskCbFreeList = LIST_OBJECT_INIT(g_tskCbFreeList);

    extern U32 OsTskAMPInit(void);
    extern U32 OsIdleTskAMPCreate(void);
    extern void OsFirstTimeSwitch(void);

其中头文件 src/include/prt_asm_cpu_external.h [`下载 <../\_static/prt_asm_cpu_external.h>`_] 包含内核相关的一些状态定义。

极简内存空间管理
^^^^^^^^^^^^^^^^^^^^^^^^^^

内核运行过程中需要动态分配内存。我们实现了一种极简的内存管理，该内存管理方法仅支持4K大小，最多256字节对齐空间的分配。

.. code-block:: C
    :linenos:
   
    //简单实现OsMemAllocAlign
    /*
    * 描述：分配任务栈空间
    * 仅支持4K大小，最多256字节对齐空间的分配
    */
    uint8_t stackMem[20][4096] __attribute__((aligned(256))); // 256 字节对齐，20 个 4K 大小的空间
    uint8_t stackMemUsed[20] = {0}; // 记录对应 4K 空间是否已被分配
    OS_SEC_TEXT void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, U8 alignPow)
    {
        // 最多支持256字节对齐
        if (alignPow > 8)
            return NULL;
        if (size != 4096)
            return NULL;
        for(int i = 0; i < 20; i++){
            if (stackMemUsed[i] == 0){
                stackMemUsed[i] = 1; // 记录对应 4K 空间已被分配
                return &(stackMem[i][0]); // 返回 4K 空间起始地址
            }
        }
        return NULL;
    }

    /*
    * 描述：分配任务栈空间
    */
    OS_SEC_L4_TEXT void *OsTskMemAlloc(U32 size)
    {
        void *stackAddr = NULL;
            stackAddr = OsMemAllocAlign((U32)OS_MID_TSK, (U8)0, size,
                                    /* 内存已按16字节大小对齐 */
                                    OS_TSK_STACK_SIZE_ALLOC_ALIGN);
        return stackAddr;
    }

任务栈初始化
^^^^^^^^^^^^^^^^^^^^^^^^^^

在理论课程中，我们知道当发生任务切换时会首先保存前一个任务的上下文到栈里，然后从栈中恢复下一个将运行任务的上下文。可是当任务第一次运行的时候怎么恢复上下文，之前从来没有保存过上下文？

答案就是我们手工制造一个就可以了。下面代码中 stack->x01 到 stack->x29 被初始化成很有标志性意义的值，其他他们的值不重要。比较重要的是 stack->x30 和 stack->spsr 等处的值。

struct TskContext 表示任务上下文，放在 src/bsp/os_cpu_armv8.h 中定义。在我们的实现上它与中断上下文 struct ExcRegInfo (在 src/bsp/os_exc_armv8.h 中定义)没有区别。在UniProton中，它们的定义有一些差别。

.. code-block:: c
    :linenos:

    /*
    * 描述: 初始化任务栈的上下文
    */
    void *OsTskContextInit(U32 taskID, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
    {
        (void)taskID;
        struct TskContext *stack = (struct TskContext *)((uintptr_t)topStack + stackSize);

        stack -= 1; // 指针减，减去一个TskContext大小

        stack->x00 = 0;
        stack->x01 = 0x01010101;
        stack->x02 = 0x02020202;
        stack->x03 = 0x03030303;
        stack->x04 = 0x04040404;
        stack->x05 = 0x05050505;
        stack->x06 = 0x06060606;
        stack->x07 = 0x07070707;
        stack->x08 = 0x08080808;
        stack->x09 = 0x09090909;
        stack->x10 = 0x10101010;
        stack->x11 = 0x11111111;
        stack->x12 = 0x12121212;
        stack->x13 = 0x13131313;
        stack->x14 = 0x14141414;
        stack->x15 = 0x15151515;
        stack->x16 = 0x16161616;
        stack->x17 = 0x17171717;
        stack->x18 = 0x18181818;
        stack->x19 = 0x19191919;
        stack->x20 = 0x20202020;
        stack->x21 = 0x21212121;
        stack->x22 = 0x22222222;
        stack->x23 = 0x23232323;
        stack->x24 = 0x24242424;
        stack->x25 = 0x25252525;
        stack->x26 = 0x26262626;
        stack->x27 = 0x27272727;
        stack->x28 = 0x28282828;
        stack->x29 = 0x29292929;
        stack->x30 = funcTskEntry;   // x30： lr(link register)
        stack->xzr = 0;

        stack->elr = funcTskEntry;
        stack->esr = 0;
        stack->far = 0;
        stack->spsr = 0x305;    // EL1_SP1 | D | A | I | F
        return stack;
    }

在 src/bsp/os_cpu_armv8.h 中加入 struct TskContext 定义。

.. code-block:: C
    :linenos:

    /*
    * 任务上下文的结构体定义。
    */
    struct TskContext {
        /* *< 当前物理寄存器R0-R12 */
        uintptr_t elr;               // 返回地址
        uintptr_t spsr;
        uintptr_t far;
        uintptr_t esr;
        uintptr_t xzr;
        uintptr_t x30;
        uintptr_t x29;
        uintptr_t x28;
        uintptr_t x27;
        uintptr_t x26;
        uintptr_t x25;
        uintptr_t x24;
        uintptr_t x23;
        uintptr_t x22;
        uintptr_t x21;
        uintptr_t x20;
        uintptr_t x19;
        uintptr_t x18;
        uintptr_t x17;
        uintptr_t x16;
        uintptr_t x15;
        uintptr_t x14;
        uintptr_t x13;
        uintptr_t x12;
        uintptr_t x11;
        uintptr_t x10;
        uintptr_t x09;
        uintptr_t x08;
        uintptr_t x07;
        uintptr_t x06;
        uintptr_t x05;
        uintptr_t x04;
        uintptr_t x03;
        uintptr_t x02;
        uintptr_t x01;
        uintptr_t x00;
    };
任务入口函数
^^^^^^^^^^^^^^^^^^^^^^^^^^

这个函数有几个有趣的地方。（1）你找不到类似 OsTskEntry(taskId); 这样的对 OsTskEntry 的函数调用。这实际上是在通过 OsTskContextInit 函数进行栈初始化时传入的，也就意味着当任务第一次就绪运行时会进入 OsTskEntry 执行。（2）用户指定的 taskcb->taskEntry 不一定要求是 4 参数的，可以是 0~4 参数之间任意选定，这个需要你在汇编层面去理解。

采用 OsTskEntry 的好处是在用户提供的 taskCb->taskEntry 函数的基础上进行了一层封装，比如可以确保调用taskCb->taskEntry执行完后调用 OsTaskExit。

.. code-block:: c
    :linenos:

    /*
    * 描述：所有任务入口
    */
    OS_SEC_L4_TEXT void OsTskEntry(TskHandle taskId)
    {
        struct TagTskCb *taskCb;
        uintptr_t intSave;

        (void)taskId;

        taskCb = RUNNING_TASK;

        taskCb->taskEntry(taskCb->args[OS_TSK_PARA_0], taskCb->args[OS_TSK_PARA_1], taskCb->args[OS_TSK_PARA_2],
                        taskCb->args[OS_TSK_PARA_3]);

        // 调度结束后会开中断，所以不需要自己添加开中断
        intSave = OsIntLock();

        OS_TASK_LOCK_DATA = 0;

        /* PRT_TaskDelete不能关中断操作，否则可能会导致它核发SGI等待本核响应时死等 */
        OsIntRestore(intSave);

        OsTaskExit(taskCb);
    }


创建任务
^^^^^^^^^^^^^^^^^^^^^^^^^^

创建任务的代码看上去还是比较多，但已经不是很复杂了。我们从后面的代码往前面看，首先是接口函数 PRT_TaskCreate 函数根据传入的 initParam 参数创建任务返回任务句柄 taskPid。

PRT_TaskCreate 函数会直接调用 OsTaskCreateOnly 函数实际进行任务创建。OsTaskCreateOnly 函数将：

    - 通过 OsTaskCreateChkAndGetTcb 函数从空闲链表 g_tskCbFreeList 中取一个任务控制块；
    - 在 OsTaskCreateRsrcInit 函数中，如果用户未提供堆栈空间，则通过 OsTskMemAlloc 为新建的任务分配堆栈空间；
    - OsTskContextInit 函数负责将栈初始化成刚刚发生过中断一样；
    - OsTskCreateTcbInit 函数负责用 initParam 参数等初始化任务控制块，包括栈指针、入口函数、优先级和参数等；
    - 最后将任务的状态设置为挂起 Suspend 状态。这意味着 PRT_TaskCreate 创建任务后处于 Suspend 状态，而不是就绪状态。



.. code-block:: c
    :linenos:

    // src/core/kernel/task/prt_task_internal.h
    OS_SEC_ALW_INLINE INLINE U32 OsTaskCreateChkAndGetTcb(struct TagTskCb **taskCb)
    {
        if (ListEmpty(&g_tskCbFreeList)) {
            return OS_ERRNO_TSK_TCB_UNAVAILABLE;
        }

        // 先获取到该控制块
        *taskCb = GET_TCB_PEND(OS_LIST_FIRST(&g_tskCbFreeList));
        // 成功，从空闲列表中移除
        ListDelete(OS_LIST_FIRST(&g_tskCbFreeList));

        return OS_OK;
    }

    OS_SEC_ALW_INLINE INLINE bool OsCheckAddrOffsetOverflow(uintptr_t base, size_t size)
    {
        return (base + size) < base;
    }

    OS_SEC_L4_TEXT U32 OsTaskCreateRsrcInit(U32 taskId, struct TskInitParam *initParam, struct TagTskCb *taskCb,
                                                    uintptr_t **topStackOut, uintptr_t *curStackSize)
    {
        U32 ret = OS_OK;
        uintptr_t *topStack = NULL;

        /* 查看用户是否配置了任务栈，如没有，则进行内存申请，并标记为系统配置，如有，则标记为用户配置。 */
        if (initParam->stackAddr != 0) {
            topStack = (void *)(initParam->stackAddr);
            taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_USER;
        } else {
            topStack = OsTskMemAlloc(initParam->stackSize);
            if (topStack == NULL) {
                ret = OS_ERRNO_TSK_NO_MEMORY;
            } else {
                taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_SYS;
            }
        }
        *curStackSize = initParam->stackSize;
        if (ret != OS_OK) {
            return ret;
        }

        *topStackOut = topStack;
        return OS_OK;
    }

    OS_SEC_L4_TEXT void OsTskCreateTcbInit(uintptr_t stackPtr, struct TskInitParam *initParam,
        uintptr_t topStackAddr, uintptr_t curStackSize, struct TagTskCb *taskCb)
    {
        /* Initialize the task's stack */
        taskCb->stackPointer = (void *)stackPtr;
        taskCb->args[OS_TSK_PARA_0] = (uintptr_t)initParam->args[OS_TSK_PARA_0];
        taskCb->args[OS_TSK_PARA_1] = (uintptr_t)initParam->args[OS_TSK_PARA_1];
        taskCb->args[OS_TSK_PARA_2] = (uintptr_t)initParam->args[OS_TSK_PARA_2];
        taskCb->args[OS_TSK_PARA_3] = (uintptr_t)initParam->args[OS_TSK_PARA_3];
        taskCb->topOfStack = topStackAddr;
        taskCb->stackSize = curStackSize;
        taskCb->taskSem = NULL;
        taskCb->priority = initParam->taskPrio;
        taskCb->taskEntry = initParam->taskEntry;
    #if defined(OS_OPTION_EVENT)
        taskCb->event = 0;
        taskCb->eventMask = 0;
    #endif
        taskCb->lastErr = 0;

        INIT_LIST_OBJECT(&taskCb->semBList);
        INIT_LIST_OBJECT(&taskCb->pendList);
        INIT_LIST_OBJECT(&taskCb->timerList);

        return;
    }

    /*
    * 描述：创建一个任务但不进行激活
    */
    OS_SEC_L4_TEXT U32 OsTaskCreateOnly(TskHandle *taskPid, struct TskInitParam *initParam)
    {
        U32 ret;
        U32 taskId;
        uintptr_t intSave;
        uintptr_t *topStack = NULL;
        void *stackPtr = NULL;
        struct TagTskCb *taskCb = NULL;
        uintptr_t curStackSize = 0;

        intSave = OsIntLock();
        // 获取一个空闲的任务控制块
        ret = OsTaskCreateChkAndGetTcb(&taskCb);
        if (ret != OS_OK) {
            OsIntRestore(intSave);
            return ret;
        }

        taskId = taskCb->taskPid;
        // 分配堆栈空间资源
        ret = OsTaskCreateRsrcInit(taskId, initParam, taskCb, &topStack, &curStackSize);
        if (ret != OS_OK) {
            ListAdd(&taskCb->pendList, &g_tskCbFreeList);
            OsIntRestore(intSave);
            return ret;
        }
        // 栈初始化，就像刚发生过中断一样
        stackPtr = OsTskContextInit(taskId, curStackSize, topStack, (uintptr_t)OsTskEntry);
        // 任务控制块初始化
        OsTskCreateTcbInit((uintptr_t)stackPtr, initParam, (uintptr_t)topStack, curStackSize, taskCb);

        taskCb->taskStatus = OS_TSK_SUSPEND | OS_TSK_INUSE;
        // 出参ID传出
        *taskPid = taskId;
        OsIntRestore(intSave);
        return OS_OK;
    }

    /*
    * 描述：创建一个任务但不进行激活
    */
    OS_SEC_L4_TEXT U32 PRT_TaskCreate(TskHandle *taskPid, struct TskInitParam *initParam)
    {
        return OsTaskCreateOnly(taskPid, initParam);
    }


解挂任务
^^^^^^^^^^^^^^^^^^^^^^^^^^

PRT_TaskResume 函数负责解挂任务，即将 Suspend 状态的任务转换到就绪状态。PRT_TaskResume 首先检查当前任务是否已创建且处于 Suspend 状态，如果处于 Suspend 状态，则清除 Suspend 位，然后调用 OsMoveTaskToReady 将任务控制块移到就绪队列中。

OsMoveTaskToReady 函数将任务加入就绪队列 g_runQueue，然后通过 OsTskSchedule 进行任务调度和切换（稍后描述）。 由于有新的任务就绪，所以需要通过OsTskSchedule 进行调度。这个位置一般称为调度点。对于优先级调度来说，找到所有的调度点并进行调度非常重要。

.. code-block:: C
    :linenos:
    
    // src/core/kernel/task/prt_task_internal.h
    OS_SEC_ALW_INLINE INLINE void OsMoveTaskToReady(struct TagTskCb *taskCb)
    {
        if (TSK_STATUS_TST(taskCb, OS_TSK_DELAY_INTERRUPTIBLE)) {
            /* 可中断delay, 属于定时等待的任务时候，去掉其定时等待标志位*/
            if (TSK_STATUS_TST(taskCb, OS_TSK_TIMEOUT)) {
                OS_TSK_DELAY_LOCKED_DETACH(taskCb);
            }
            TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT | OS_TSK_DELAY_INTERRUPTIBLE);
        }

        /* If task is not blocked then move it to ready list */
        if ((taskCb->taskStatus & OS_TSK_BLOCK) == 0) {
            OsTskReadyAdd(taskCb);
    
            if ((OS_FLG_BGD_ACTIVE & UNI_FLAG) != 0) {
                OsTskSchedule();
                return;
            }
        }
    }

    /*
    * 描述解挂任务
    */
    OS_SEC_L2_TEXT U32 PRT_TaskResume(TskHandle taskPid)
    {
        uintptr_t intSave;
        struct TagTskCb *taskCb = NULL;

        // 获取 taskPid 对应的任务控制块
        taskCb = GET_TCB_HANDLE(taskPid);

        intSave = OsIntLock();

        if (TSK_IS_UNUSED(taskCb)) {
            OsIntRestore(intSave);
            return OS_ERRNO_TSK_NOT_CREATED;
        }

        if (((OS_TSK_RUNNING & taskCb->taskStatus) != 0) && (g_uniTaskLock != 0)) {
            OsIntRestore(intSave);
            return OS_ERRNO_TSK_ACTIVE_FAILED;
        }

        /* If task is not suspended and not in interruptible delay then return */
        if (((OS_TSK_SUSPEND | OS_TSK_DELAY_INTERRUPTIBLE) & taskCb->taskStatus) == 0) {
            OsIntRestore(intSave);
            return OS_ERRNO_TSK_NOT_SUSPENDED;
        }

        TSK_STATUS_CLEAR(taskCb, OS_TSK_SUSPEND);

        /* If task is not blocked then move it to ready list */
        OsMoveTaskToReady(taskCb);
        OsIntRestore(intSave);

        return OS_OK;
    }


任务管理系统初始化与启动
^^^^^^^^^^^^^^^^^^^^^^^^^^

OsTskInit 函数通过调用 OsTskAMPInit 函数完成任务管理系统的初始化。主要包括：

    - 为任务控制块分配空间，由于我们只实现了简单的内存分配算法，所以支持的任务控制块数目为：4096 / sizeof(struct TagTskCb) - 2; 减去2是因为预留了 1 个空闲任务， 1 个无效任务。
    - 将所有分配的任务控制块加入空闲任务控制块链表 g_tskCbFreeList， 并对所有控制块进行初始化。
    - 任务就绪链表 g_runQueue 通过 INIT_LIST_OBJECT 初始化为空。
    - RUNNING_TASK 目前指向无效任务。

OsActivate 启动多任务系统。

    - 首先通过 OsIdleTskAMPCreate 函数创建空闲任务，这样当系统中没有其他任务就绪时就可以执行空闲任务了。
    - OsTskHighestSet 函数在就绪队列中查找最高优先级任务并将 g_highestTask 指针指向该任务。
    - UNI_FLAG 设置好内核状态
    - OsFirstTimeSwitch 函数将会加载 g_highestTask 的上下文后执行（稍后描述）。

.. code-block:: c
    :linenos:

    /*
    * 描述：AMP任务初始化
    */
    extern U32 g_threadNum;
    extern void *OsMemAllocAlign(U32 mid, U8 ptNo, U32 size, U8 alignPow);
    OS_SEC_L4_TEXT U32 OsTskAMPInit(void)
    {
        uintptr_t size;
        U32 idx;

        // 简单处理，分配4096,存OS_MAX_TCB_NUM个任务。#define OS_MAX_TCB_NUM  (g_tskMaxNum + 1 + 1)  // 1个IDLE，1个无效任务
        g_tskCbArray = (struct TagTskCb *)OsMemAllocAlign((U32)OS_MID_TSK, 0,
                                                        4096, OS_TSK_STACK_SIZE_ALLOC_ALIGN);
        if (g_tskCbArray == NULL) {
            return OS_ERRNO_TSK_NO_MEMORY;
        }

        g_tskMaxNum = 4096 / sizeof(struct TagTskCb) - 2;
        

        // 1为Idle任务
        g_threadNum += (g_tskMaxNum + 1);

        // 初始化为全0
        for(int i = 0; i < OS_MAX_TCB_NUM - 1; i++)
            g_tskCbArray[i] = {0};

        g_tskBaseId = 0;

        // 将所有控制块加入g_tskCbFreeList链表，且设置控制块的初始状态和任务id
        INIT_LIST_OBJECT(&g_tskCbFreeList);
        for (idx = 0; idx < OS_MAX_TCB_NUM - 1; idx++) {
            g_tskCbArray[idx].taskStatus = OS_TSK_UNUSED;
            g_tskCbArray[idx].taskPid = (idx + g_tskBaseId);
            ListTailAdd(&g_tskCbArray[idx].pendList, &g_tskCbFreeList);
        }

        /* 在初始化时给RUNNING_TASK的PID赋一个合法的无效值，放置在Trace使用时出现异常 */
        RUNNING_TASK = OS_PST_ZOMBIE_TASK;

        /* 在初始化时给RUNNING_TASK的PID赋一个合法的无效值，放置在Trace使用时出现异常 */
        RUNNING_TASK->taskPid = idx + g_tskBaseId;

        INIT_LIST_OBJECT(&g_runQueue);

        /* 增加OS_TSK_INUSE状态，使得在Trace记录的第一条信息状态为OS_TSK_INUSE(创建状态) */
        RUNNING_TASK->taskStatus = (OS_TSK_INUSE | OS_TSK_RUNNING);
        RUNNING_TASK->priority = OS_TSK_PRIORITY_LOWEST + 1;

        return OS_OK;
    }

    /*
    * 描述：任务初始化
    */
    OS_SEC_L4_TEXT U32 OsTskInit(void)
    {
        U32 ret;
        ret = OsTskAMPInit();
        if (ret != OS_OK) {
            return ret;
        }

        return OS_OK;
    }

    /*
    * 描述：Idle背景任务
    */
    OS_SEC_TEXT void OsTskIdleBgd(void)
    {
        while (TRUE);   
    }

    /*
    * 描述：ilde任务创建.
    */
    OS_SEC_L4_TEXT U32 OsIdleTskAMPCreate(void)
    {
        U32 ret;
        TskHandle taskHdl;
        struct TskInitParam taskInitParam = {0};
        char tskName[OS_TSK_NAME_LEN] = "IdleTask";

        /* Create background task. */
        taskInitParam.taskEntry = (TskEntryFunc)OsTskIdleBgd;
        taskInitParam.stackSize = 4096;
        // taskInitParam.name = tskName;
        taskInitParam.taskPrio = OS_TSK_PRIORITY_LOWEST;
        taskInitParam.stackAddr = 0;

        /* 任务调度的必要条件就是有背景任务，此时背景任务还没有创建，因此不会发生任务切换 */
        ret = PRT_TaskCreate(&taskHdl, &taskInitParam);
        if (ret != OS_OK) {
            return ret;
        }
        ret = PRT_TaskResume(taskHdl);
        if (ret != OS_OK) {
            return ret;
        }
        IDLE_TASK_ID = taskHdl;

        return ret;
    }

    /*
    * 描述：激活任务管理
    */
    OS_SEC_L4_TEXT U32 OsActivate(void)
    {
        U32 ret;
        ret = OsIdleTskAMPCreate();
        if (ret != OS_OK) {
            return ret;
        }

        OsTskHighestSet();

        /* Indicate that background task is running. */
        UNI_FLAG |= OS_FLG_BGD_ACTIVE | OS_FLG_TSK_REQ;

        /* Start Multitasking. */
        OsFirstTimeSwitch();
        // 正常情况不应执行到此
        return OS_ERRNO_TSK_ACTIVE_FAILED;
    }


在 prt_config.h 中加入空闲任务优先级定义。

.. code-block:: C
    :linenos:

    #define OS_TSK_PRIORITY_LOWEST 63


任务状态转换
--------------------------

在 src/kernel/task/prt_task.c 中， 

    - 声明了运行队列 g_runQueue， 注意我们之前已经将其定义为双向队列。
    - 提供了将任务添加到就绪队列的 OsTskReadyAdd 函数和从就绪队列中移除就绪队列的 OsTskReadyDel 函数。
        - OsTskReadyAdd 会设置任务为就绪态
        - OsTskReadyDel 会清除任务的就绪态
    - 提供了任务结束退出 OsTaskExit 函数，注意 OsTskEntry 中会调用 OsTaskExit 函数。由于任务退出，因此需要进行调度，即存在调度点，所以调用 OsTskSchedule 函数。


.. code-block:: c
    :linenos:

    #include "prt_task_external.h"
    #include "prt_typedef.h"
    #include "os_attr_armv8_external.h"
    #include "prt_asm_cpu_external.h"
    #include "os_cpu_armv8_external.h"
    #include "prt_amp_task_internal.h"

    OS_SEC_BSS struct TagOsRunQue g_runQueue;  // 核的局部运行队列

    /*
    * 描述：将任务添加到就绪队列, 调用者确保不会换核，并锁上rq
    */
    OS_SEC_L0_TEXT void OsTskReadyAdd(struct TagTskCb *task)
    {
        struct TagOsRunQue *rq = &g_runQueue;
        TSK_STATUS_SET(task, OS_TSK_READY);

        OS_TSK_EN_QUE(rq, task, 0);
        OsTskHighestSet();

        return;
    }

    /*
    * 描述：将任务从就绪队列中移除，关中断外部保证
    */
    OS_SEC_L0_TEXT void OsTskReadyDel(struct TagTskCb *taskCb)
    {
        struct TagOsRunQue *runQue = &g_runQueue;
        TSK_STATUS_CLEAR(taskCb, OS_TSK_READY);

        OS_TSK_DE_QUE(runQue, taskCb, 0);
        OsTskHighestSet();

        return;
    }

    // src/core/kernel/task/prt_task_del.c
    /*
    * 描述：任务结束退出
    */
    OS_SEC_L4_TEXT void OsTaskExit(struct TagTskCb *tsk)
    {

        uintptr_t intSave = OsIntLock();

        OsTskReadyDel(tsk);
        OsTskSchedule();

        OsIntRestore(intSave);

    }

    

其中，OS_TSK_EN_QUE 和 OS_TSK_DE_QUE 宏在 src/include/prt_amp_task_internal.h 定义。


调度与切换
--------------------------

src/kernel/sched/prt_sched_single.c

.. code-block:: c
    :linenos:

    #include "prt_task_external.h"
    #include "os_attr_armv8_external.h"
    #include "prt_asm_cpu_external.h"
    #include "os_cpu_armv8_external.h"

    /*
    * 描述：任务调度，切换到最高优先级任务
    */
    OS_SEC_TEXT void OsTskSchedule(void)
    {
        /* 外层已经关中断 */
        /* Find the highest task */
        OsTskHighestSet();

        /* In case that running is not highest then reschedule */
        if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
            UNI_FLAG |= OS_FLG_TSK_REQ;

            /* only if there is not HWI or TICK the trap */
            if (OS_INT_INACTIVE) { // 不在中断上下文中，否则应该在中断返回时切换
                OsTaskTrap();
                return;
            }
        }

        return;
    }
    
    /*
    * 描述: 调度的主入口
    * 备注: NA
    */
    OS_SEC_L0_TEXT void OsMainSchedule(void)
    {
        struct TagTskCb *prevTsk;
        if ((UNI_FLAG & OS_FLG_TSK_REQ) != 0) {
            prevTsk = RUNNING_TASK;

            /* 清除OS_FLG_TSK_REQ标记位 */
            UNI_FLAG &= ~OS_FLG_TSK_REQ;

            RUNNING_TASK->taskStatus &= ~OS_TSK_RUNNING;
            g_highestTask->taskStatus |= OS_TSK_RUNNING;

            RUNNING_TASK = g_highestTask;
        }
        // 如果中断没有驱动一个任务ready，直接回到被打断的任务
        OsTskContextLoad((uintptr_t)RUNNING_TASK);
    }

    /*
    * 描述: 系统启动时的首次任务调度
    * 备注: NA
    */
    OS_SEC_L4_TEXT void OsFirstTimeSwitch(void)
    {
        OsTskHighestSet();
        RUNNING_TASK = g_highestTask;
        TSK_STATUS_SET(RUNNING_TASK, OS_TSK_RUNNING);
        OsTskContextLoad((uintptr_t)RUNNING_TASK);
        // never get here
        return;
    }

其中，OsTskHighestSet 函数在 src/include/prt_task_external.h 中被定义为内联函数，提高性能。

.. code-block:: C
    :linenos:

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

在 src/bsp/prt_vector.S 实现 OsTskContextLoad，OsContextLoad 和 OsTaskTrap。

.. code-block:: asm
    :linenos:

    /*
    * 描述: void OsTskContextLoad(uintptr_t stackPointer)
    */
        .globl OsTskContextLoad
        .type OsTskContextLoad, @function
        .align 4
    OsTskContextLoad:
        ldr    X0, [X0]
        mov    SP, X0            // X0 is stackPointer

    OsContextLoad:
        ldp    x2, x3, [sp],#16
        add    sp, sp, #16        // 跳过far, esr, HCR_EL2.TRVM==1的时候，EL1不能写far, esr
        msr    spsr_el1, x3
        msr    elr_el1, x2
        dsb    sy
        isb

        RESTORE_EXC_REGS // 恢复上下文
        
        eret //从异常返回


    /*
    * 描述: Task调度处理函数。 X0 is g_runningTask
    */
        .globl OsTaskTrap
        .type OsTaskTrap, @function
        .align 4

    OsTaskTrap:
        LDR    x1, =g_runningTask /* OsTaskTrap是函数调用过来，x0 x1寄存器是caller save，此处能直接使用 */
        LDR    x0, [x1] /* x0 is the &g_pRunningTask->sp */

        SAVE_EXC_REGS

        /* TskTrap需要保存CPSR，由于不能直接访问，需要拼接获取当前CPSR入栈 */
        mrs    x3, DAIF /* CPSR：DAIF 4种事件的mask, bits[9:6] */
        mrs    x2, NZCV /* NZCV：Condition flags, bits[31:28] */
        orr    x3, x3, x2
        orr    x3, x3, #(0x1U << 2) /* 当前的 exception level,bits[3:2] 00:EL0,01:El1,10:El2,11:EL3 */
        orr    x3, x3, #(0x1U) /* 当前栈的选择,bits[0] 0:SP_EL0,1:SP_ELX */

        mov    x2, x30    // 用返回地址x30作为现场恢复点
        sub    sp, sp, #16  // 跳过esr_el1, far_el1, 异常时才有用
        stp    x2, x3, [sp,#-16]!

        // 存入SP指针到g_pRunningTask->sp
        mov    x1, sp
        str    x1, [x0]   // x0 is the &g_pRunningTask->sp

        B      OsMainSchedule
    loop1:
        B      loop1


在 src/bsp/os_cpu_armv8_external.h 加入 OsTaskTrap 和 OsTskContextLoad 的声明和关于栈地址和大小对齐宏。

.. code-block:: c
    :linenos:

    #define OS_TSK_STACK_SIZE_ALIGN  16U
    #define OS_TSK_STACK_SIZE_ALLOC_ALIGN 4U //按2的幂对齐，即2^4=16字节
    #define OS_TSK_STACK_ADDR_ALIGN  16U

    extern void OsTaskTrap(void);
    extern void OsTskContextLoad(uintptr_t stackPointer);


最后在 src/kernel/task/prt_sys.c 定义了内核的各种全局数据。 

.. code-block:: C
    :linenos:

    #include "prt_typedef.h"
    #include "os_attr_armv8_external.h"
    #include "prt_task.h"

    OS_SEC_L4_BSS U32 g_threadNum;
    
    /* Tick计数 */
    // OS_SEC_BSS U64 g_uniTicks; // 把 lab5 中在 src/kernel/tick/prt_tick.c 定义的 g_uniTicks 移到此处则取消此行的注释

    /* 系统状态标志位 */
    OS_SEC_DATA U32 g_uniFlag = 0;

    OS_SEC_DATA struct TagTskCb *g_runningTask = NULL;

    // src/core/kernel/task/prt_task_global.c
    OS_SEC_BSS TskEntryFunc g_tskIdleEntry;


    OS_SEC_BSS U32 g_tskMaxNum;
    OS_SEC_BSS struct TagTskCb *g_tskCbArray;
    OS_SEC_BSS U32 g_tskBaseId;

    OS_SEC_BSS TskHandle g_idleTaskId;
    OS_SEC_BSS U16 g_uniTaskLock;
    OS_SEC_BSS struct TagTskCb *g_highestTask;


任务调度测试
--------------------------


.. code-block:: C
    :linenos:

    #include "prt_typedef.h"
    #include "prt_tick.h"
    #include "prt_task.h"

    extern U32 PRT_Printf(const char *format, ...);
    extern void PRT_UartInit(void);
    extern void CoreTimerInit(void);
    extern U32 OsHwiInit(void);
    extern U32 OsActivate(void);
    extern U32 OsTskInit(void);

    void Test1TaskEntry()
    {
        PRT_Printf("task 1 run ...\n");

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

        U32 cnt = 5;
        while (cnt > 0) {
            // PRT_TaskDelay(100);
            PRT_Printf("task 2 run ...\n");
            cnt--;
        }
    }

    S32 main(void)
    {


        // 初始化GIC
        OsHwiInit();
        // 启用Timer
        CoreTimerInit();
        // 任务系统初始化
        OsTskInit();

        PRT_UartInit();

        PRT_Printf("            _       _ _____      _             _             _   _ _   _ _   _           \n");
        PRT_Printf("  _ __ ___ (_)_ __ (_) ____|   _| | ___ _ __  | |__  _   _  | | | | \\ | | | | | ___ _ __ \n");
        PRT_Printf(" | '_ ` _ \\| | '_ \\| |  _|| | | | |/ _ \\ '__| | '_ \\| | | | | |_| |  \\| | | | |/ _ \\ '__|\n");
        PRT_Printf(" | | | | | | | | | | | |__| |_| | |  __/ |    | |_) | |_| | |  _  | |\\  | |_| |  __/ |   \n");
        PRT_Printf(" |_| |_| |_|_|_| |_|_|_____\\__,_|_|\\___|_|    |_.__/ \\__, | |_| |_|_| \\_|\\___/ \\___|_|   \n");
        PRT_Printf("                                                     |___/                               \n");

        PRT_Printf("ctr-a h: print help of qemu emulator. ctr-a x: quit emulator.\n\n");

        U32 ret;
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


.. hint:: 将新建文件加入构建系统



lab6 作业
--------------------------

作业1 
^^^^^^^^^^^^^^^^^^^^^^^^^^^

实现分时调度。

.. hint:: 分时调度的调度点仅存在于时钟Tick中断。