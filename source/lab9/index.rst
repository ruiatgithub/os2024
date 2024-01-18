实验九 Shell 
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
    extern void OsGicClearInt(uint32_t interrupt);
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
        OsGicClearInt(33); //可省略
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

        PRT_Printf("\nPID\t\tPriority\tStack Size\n");
        // 遍历g_runQueue队列，查找优先级最高的任务
        LIST_FOR_EACH(taskCb, &g_runQueue, struct TagTskCb, pendList) {
            cnt++;
            PRT_Printf("%d\t\t%d\t\t%d\n", taskCb->taskPid, taskCb->priority, taskCb->stackSize);
        }
        PRT_Printf("Total %d tasks", cnt);

    }

在 src/kernel/tick/prt_tick.c 中加入函数

.. code-block:: c
    :linenos:

    extern U32 PRT_Printf(const char *format, ...);
    OS_SEC_TEXT void OsDisplayCurTick(void)
    {
        PRT_Printf("\nCurrent Tick: %d", PRT_TickGetCount());
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
            PRT_Printf("\nminiEuler # ");
            idx = 0;
            for(int i = 0; i < SHELL_SHOW_MAX_LEN; i++)
            {
                cmd[i] = 0;
            }

            while (1){
                PRT_SemPend(sem_uart_rx, OS_WAIT_FOREVER);
                
                // 读取shellCB缓冲区的字符
                ch = shellCB->shellBuf[shellCB->shellBufReadOffset];
                cmd[idx] = ch;
                idx++;
                shellCB->shellBufReadOffset++;
                if(shellCB->shellBufReadOffset == SHELL_SHOW_MAX_LEN)
                    shellCB->shellBufReadOffset = 0;

                PRT_Printf("%c", ch); //回显
                if (ch == '\r'){
                    // PRT_Printf("\n");
                    if(cmd[0]=='t' && cmd[1]=='o' && cmd[2]=='p'){
                        OsDisplayTasksInfo();
                    } else if(cmd[0]=='t' && cmd[1]=='i' && cmd[2]=='c' && cmd[3]=='k'){
                        OsDisplayCurTick();
                    }
                    break;
                }
                    
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







.. hint:: 将新增文件加入构建系统

lab9 作业
--------------------------

作业1
^^^^^^^^^^^^^^^^^^^^^^^^^^

实现一条有用的 shell 指令。