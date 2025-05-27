实验十 Shell 
=====================


新建 src/include/prt_shell.h 头文件

.. code-block:: c
    :linenos:

    #ifndef _HWLITEOS_SHELL_H
    #define _HWLITEOS_SHELL_H

    #include "prt_typedef.h"

    #define SHELL_SHOW_MAX_LEN    272
    #define PATH_MAX        1024

    typedef struct {
        U32   consoleID;
        U32   shellTaskHandle;
        U32   shellEntryHandle;
        void     *cmdKeyLink;
        void     *cmdHistoryKeyLink;
        void     *cmdMaskKeyLink;
        U32   shellBufOffset;
        U32   shellBufReadOffset;
        U32   shellKeyType;
        char     shellBuf[SHELL_SHOW_MAX_LEN];
        char     shellWorkingDirectory[PATH_MAX];
    } ShellCB;

    #endif /* _HWLITEOS_SHELL_H */

接收输入
--------------------------
QEMU的virt机器默认没有键盘作为输入设备，但当我们执行QEMU使用 -nographic 参数（disable graphical output and redirect serial I/Os to console）时QEMU会将串口重定向到控制台，因此我们可以使用UART作为输入设备。

在 src/bsp/print.c 中的 PRT_UartInit 添加初始化代码，使其支持接收数据中断。 同时定义了用于串口接收的信号量 sem_uart_rx。

.. code-block:: C
    :linenos:

    #include "prt_sem.h"
    #include "prt_shell.h"


    #define UARTCR_UARTEN (1 << 0)
    #define UARTCR_TXE (1 << 8)
    #define UARTCR_RXE (1 << 9)

    #define UARTICR_ALL (1 << 0)

    #define UARTIMSC_RXIM (1 << 4)

    #define UARTIBRD_IBRD_MASK 0xFFFF
    #define UARTFBRD_FBRD_MASK 0x3F

    #define UARTLCR_H_WLEN_MASK (3 << 5)
    #define UARTLCR_H_PEN (1 << 1)
    #define UARTLCR_H_STP1 (0 << 3)

    SemHandle sem_uart_rx;

    extern void OsGicIntSetConfig(uint32_t interrupt, uint32_t config);
    extern void OsGicIntSetPriority(uint32_t interrupt, uint32_t priority);
    extern void OsGicEnableInt(U32 intId);
    extern void OsGicClearIntPending(uint32_t interrupt);
    extern U32 PRT_Printf(const char *format, ...);
    U32 PRT_UartInit(void)
    {
        U32 result = 0;
        U32 reg_base = UART_0_REG_BASE;

        UART_REG_WRITE(0, (unsigned long)(reg_base + 0x30));// 禁用pl011
        UART_REG_WRITE(0x7ff, (unsigned long)(reg_base + 0x44));// 清空中断状态
        UART_REG_WRITE(UARTIMSC_RXIM, (unsigned long)(reg_base + 0x38));// 设定中断mask，需要使能的中断
        UART_REG_WRITE(13, (unsigned long)(reg_base + 0x24));
        UART_REG_WRITE(1, (unsigned long)(reg_base + 0x28));

        // https://developer.arm.com/documentation/ddi0183/g/programmers-model/register-descriptions/line-control-register--uartlcr-h?lang=en
        result = UART_REG_READ((unsigned long)(reg_base + DW_UART_LCR_HR));
        result = result | UARTLCR_H_WLEN_MASK | UARTLCR_H_PEN | UARTLCR_H_STP1 | DW_FIFO_ENABLE;
        UART_REG_WRITE(result, (unsigned long)(reg_base + DW_UART_LCR_HR)); // 8N1 FIFO enable

        UART_REG_WRITE(UARTCR_UARTEN | UARTCR_RXE | UARTCR_TXE, (unsigned long)(reg_base + 0x30));// 启用pl011


        // 启用UART 接收中断
        OsGicIntSetConfig(33, 0); //可省略
        OsGicIntSetPriority(33, 0);
        OsGicClearIntPending(33); //可省略
        OsGicEnableInt(33);

        // 创建uart数据接收信号量
        U32 ret;
        ret = PRT_SemCreate(0, &sem_uart_rx);
        if (ret != OS_OK) {
            PRT_Printf("failed to create uart_rx sem\n");
            return 1;
        }

        return OS_OK;
    }

简单起见，在 src/bsp/print.c 中实现  OsUartRxHandle() 处理接收中断。

.. code-block:: c
    :linenos:

    extern ShellCB g_shellCB;
    void OsUartRxHandle(void)
    {
        U32 flag = 0;
        U32 result = 0;
        U32 reg_base = UART_0_REG_BASE;

        flag = UART_REG_READ((unsigned long)(reg_base + 0x18));
        while((flag & (1<<4)) == 0)
        {
            result = UART_REG_READ((unsigned long)(reg_base + 0x0));
            // PRT_Printf("%c", result);

            // 将收到的字符存到g_shellCB的缓冲区
            g_shellCB.shellBuf[g_shellCB.shellBufOffset] = (char) result;
            g_shellCB.shellBufOffset++;
            if (g_shellCB.shellBufOffset == SHELL_SHOW_MAX_LEN)
                g_shellCB.shellBufOffset = 0;

            PRT_SemPost(sem_uart_rx);
            flag = UART_REG_READ((unsigned long)(reg_base + 0x18));
        }
        return;
    }

在 src/bsp/prt_exc.c 中OsHwiHandleActive() 链接中断和处理函数OsUartRxHandle()

.. code-block:: c
    :linenos:

    extern void OsTickDispatcher(void);
    extern void OsUartRxHandle(void);
    OS_SEC_ALW_INLINE INLINE void OsHwiHandleActive(U32 irqNum)
    {
        switch(irqNum){
            case 30: 
                OsTickDispatcher();
                // PRT_Printf(".");
                break;
            case 33:
                OsUartRxHandle();
            default:
                break;
        }
    }


在 src/kernel/task/prt_task.c 中加入函数

.. code-block:: c
    :linenos:

    extern U32 PRT_Printf(const char *format, ...);
    OS_SEC_TEXT void OsDisplayTasksInfo(void)
    {
        struct TagTskCb *taskCb = NULL;
        U32 cnt = 0;

        PRT_Printf("PID\t\tPriority\tStack Size\n");
        // 遍历g_runQueue队列，查找优先级最高的任务
        LIST_FOR_EACH(taskCb, &g_runQueue, struct TagTskCb, pendList) {
            cnt++;
            PRT_Printf("%d\t\t%d\t\t%d\n", taskCb->taskPid, taskCb->priority, taskCb->stackSize);
        }
        PRT_Printf("Total %d tasks\n", cnt);

    }

在 src/kernel/tick/prt_tick.c 中加入函数

.. code-block:: c
    :linenos:

    extern U32 PRT_Printf(const char *format, ...);
    OS_SEC_TEXT void OsDisplayCurTick(void)
    {
        PRT_Printf("Current Tick: %d\n", PRT_TickGetCount());
    }


shell 处理
--------------------------

新建 src/shell/shmsg.c 文件。

.. code-block:: c
    :linenos:

    #include "prt_typedef.h"
    #include "prt_shell.h"
    #include "os_attr_armv8_external.h"
    #include "prt_task.h"
    #include "prt_sem.h"
    #include <string.h>

    extern SemHandle sem_uart_rx;
    extern U32 PRT_Printf(const char *format, ...);
    extern void OsDisplayTasksInfo(void);
    extern void OsDisplayCurTick(void);


    OS_SEC_TEXT void ShellTask(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
    {
        U32 ret;
        char ch;
        char cmd[SHELL_SHOW_MAX_LEN];
        U32 idx;
        ShellCB *shellCB = (ShellCB *)param1;

        while (1) {
            PRT_Printf("\033[1;92mminiEuler# \033[0m");
            idx = 0;
            for(int i = 0; i < SHELL_SHOW_MAX_LEN; i++)
            {
                cmd[i] = 0;
            }

            enum {NORMAL, ESC, BRACKET} state = NORMAL;  // 状态机
            while (1){
                PRT_SemPend(sem_uart_rx, OS_WAIT_FOREVER);

                // 读取shellCB缓冲区的字符
                ch = shellCB->shellBuf[shellCB->shellBufReadOffset];
                shellCB->shellBufReadOffset++;
                if(shellCB->shellBufReadOffset == SHELL_SHOW_MAX_LEN)
                    shellCB->shellBufReadOffset = 0;
                // 状态机处理转义序列
                switch(state) {
                    case NORMAL:
                        if (ch == '\033') {
                            state = ESC;
                            continue;
                        }
                        break;
                    case ESC:
                        if (ch == '[') {
                            state = BRACKET;
                            continue;
                        }
                        state = NORMAL;
                        break;
                    case BRACKET:
                        state = NORMAL;
                        // 处理箭头键
                        switch(ch) {
                            case 'D':  // 左箭头
                                if (idx > 0) {
                                    idx--;
                                    PRT_Printf("\033[D");  // 光标左移
                                }
                                continue;
                            case 'C':  // 右箭头
                                if (idx < strlen(cmd)) {
                                    idx++;
                                    PRT_Printf("\033[C");  // 光标右移
                                }
                                continue;
                        }
                        continue;
                }
                // 检查退格键
                if (ch == 0x7F || ch == '\b') {
                    if (idx > 0) {
                        memmove(&cmd[idx-1], &cmd[idx], strlen(cmd) - idx);
                        cmd[strlen(cmd)-1] = '\0';
                        idx--;
                        PRT_Printf("\b\033[K");  // 删除当前行从光标位置到行尾
                        PRT_Printf("%s", &cmd[idx]);  // 重绘剩余字符
                        
                        if (strlen(cmd) > idx)
                            PRT_Printf("\033[%dD", strlen(cmd) - idx);  // 光标复位
                    }
                    continue;
                }
                if (ch == '\r'){
                    // PRT_Printf("\n%s", cmd); // 检查行编辑功能是否正确
                    PRT_Printf("\n");
                    // 使用strcmp代替逐个字符比较
                    if (strcmp(cmd, "help") == 0) {
                        PRT_Printf("Available commands:\n");
                        PRT_Printf("    top : print current task list.\n");
                        PRT_Printf("    tick : print current tick.\n");
                        PRT_Printf("    quit : exit shell.\n");
                    } else if (strcmp(cmd, "top") == 0) {
                        OsDisplayTasksInfo();
                    } else if (strcmp(cmd, "tick") == 0) {
                        OsDisplayCurTick();
                    }  else {
                        PRT_Printf("\033[1;91m[error]\033[0m Invalid command. Type \"help\" for a list of available commands.\n");
                    }
                    break;
                }
                if (idx >= SHELL_SHOW_MAX_LEN - 1) continue;
                memmove(&cmd[idx+1], &cmd[idx], strlen(cmd) - idx + 1);
                cmd[idx] = ch;
                PRT_Printf("\033[94m%s\033[0m", &cmd[idx]);  // 重绘剩余字符
                idx++;
                if (strlen(cmd) > idx) {
                    PRT_Printf("\033[%dD", (int)(strlen(cmd) - idx));  // 使用ANSI转义序列
                }
            }
            if (strcmp(cmd, "quit") == 0) {
                PRT_Printf("\n");
                break;
            }
        }
    }

    OS_SEC_TEXT U32 ShellTaskInit(ShellCB *shellCB)
    {
        U32 ret = 0;
        struct TskInitParam param = {0};

        // task 1
        // param.stackAddr = 0;
        param.taskEntry = (TskEntryFunc)ShellTask;
        param.taskPrio = 9;
        // param.name = "Test1Task";
        param.stackSize = 0x1000; //固定4096，参见prt_task_init.c的OsMemAllocAlign
        param.args[0] = (uintptr_t)shellCB;
        
        TskHandle tskHandle1;
        ret = PRT_TaskCreate(&tskHandle1, &param);
        if (ret) {
            return ret;
        }

        ret = PRT_TaskResume(tskHandle1);
        if (ret) {
            return ret;
        }
    }


中断处理调整
--------------------------
由于在处理接收中断函数中调用了PRT_SemPost(sem_uart_rx)，然后调用了PRT_SemPost()--OsTskScheduleFastPs(intSave)--OsTaskTrapFastPs(intSave)--OsTaskTrap()。
然而在OsTaskTrap()中，我们首先通过任务控制块获取了旧任务栈指针，重新保存了上下文，然后将新的栈指针写回任务控制块，最后调用OsMainSchedule()进行调度。
但是中断触发的时候，上下文应该由EXC_HANDLE宏保存，因此在中断切换任务时不应该调用OsTaskTrap()重复保存上下文，只需要更新旧任务栈指针即可。

在src/bsp/prt_vector.S加入以下代码，专门用来处理中断中的任务调度。

.. code-block:: c
    :linenos:

    /*
    * 中断时的调度函数
    */
        .globl OsTaskSwitchFromIrq
        .type OsTaskSwitchFromIrq, @function
        .align 4
    OsTaskSwitchFromIrq:
        LDR    x1, =g_runningTask
        LDR    x0, [x1]           // x0 = &g_runningTask->sp
        // 复用 EXC_HANDLE 保存的 x0-x30，无需补充 elr/spsr

        // 更新旧任务栈指针
        mov    x1, sp
        str    x1, [x0]

        B      OsMainSchedule

在OsTskScheduleFastPs(intSave)中有这样一段代码：

.. code-block:: c
    :linenos:

    #define OS_INT_ACTIVE_MASK \
    (OS_FLG_HWI_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)
    #define OS_INT_ACTIVE ((UNI_FLAG & OS_INT_ACTIVE_MASK) != 0)
    #define OS_INT_INACTIVE (!(OS_INT_ACTIVE))

    ......

    /* In case that running is not highest then reschedule */
    if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        /* only if there is not HWI or TICK the trap */
        if (OS_INT_INACTIVE) {
            OsTaskTrapFastPs(intSave);
        }
    }

其中OS_INT_INACTIVE与UNI_FLAG有关，但是我们从来没有设置过UNI_FLAG。
因此需要在src/bsp/prt_exc.c的OsHwiDispatch()中添加UNI_FLAG相关操作，这样在中断处理过程中调用OsTskScheduleFastPs时只会设置OS_FLG_TSK_REQ标志代表有调度请求，而不会进入OsTaskTrapFastPs(intSave)立刻开始调度。
在中断结束后调用OsTaskSwitchFromIrq()更新旧任务控制块栈指针，随后进入OsMainSchedule()进行调度。

.. code-block:: c
    :linenos:

    extern  U32 OsGicIntAcknowledge(void);
    extern void OsGicIntClear(U32 value);
    extern void OsTaskSwitchFromIrq();
    // src/arch/cpu/armv8/common/hwi/prt_hwi.c  OsHwiDispatch(),OsHwiDispatchHandle()
    /*
    * 描述: 中断处理入口, 调用处外部已关中断
    */
    OS_SEC_L0_TEXT void OsHwiDispatch( U32 excType, struct ExcRegInfo *excRegs) //src/arch/cpu/armv8/common/hwi/prt_hwi.c
    {
        UNI_FLAG |= OS_FLG_HWI_ACTIVE;  // 设置硬中断标志
        // 中断确认，相当于OsHwiNumGet()
        U32 value = OsGicIntAcknowledge();
        U32 irq_num = value & 0x1ff;
        U32 core_num = value & 0xe00;

        OsHwiHandleActive(irq_num);

        // 清除中断，相当于 OsHwiClear(hwiNum);
        OsGicIntClear(irq_num|core_num);
        UNI_FLAG &= ~OS_FLG_HWI_ACTIVE;  // 清除硬中断标志
        
        if (UNI_FLAG & OS_FLG_TSK_REQ) {
            OsTaskSwitchFromIrq();
        }
    }


.. hint:: 将新增文件加入构建系统

lab9 作业
--------------------------

作业1
^^^^^^^^^^^^^^^^^^^^^^^^^^

实现一条有用的 shell 指令。