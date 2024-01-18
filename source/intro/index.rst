前言 
=====================

- 零拾（010）计划，课程解决从0到1的问题，实验解决从1到10的问题，自己再从10到100，甚至1000。
- 从象牙塔到产业界，我们拆解华为的 UniProton 内核，这是 OpenEuler 旗下的嵌入式实时内核，未来也将融入鸿蒙系统。
- 我们的目标不是为了构建一个完整的操作系统。
- 每个实验需理解的关键代码为数十行到数百行之间，同学们需要自行撰写的代码在数行到数十行之间，完全在同学们可接受范围内。
- 最终的版本支持
    - shell
    - 时间Tick
    - 任务创建
    - 任务调度
    - 任务延迟
    - 信号量

- UniProton 采用木兰协议，其版权声明如下。

.. code-block:: c

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
    */
