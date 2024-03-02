前言 
=====================

- 零拾（010）计划：操作系统理论课程解决从0到1的问题，课程实验解决从1到10的问题。希望同学们可以以此为起点从10再到100，甚至1000。
    - 010的名字是受教育部 `101计划 <http://101.pku.edu.cn/index>`_ 的启发，教育部101计划是指本科教育教学改革试点工作计划。
- 从象牙塔到产业界，我们拆解的是 `UniProton <https://gitee.com/openeuler/UniProton>`_ 操作系统内核，这是华为 OpenEuler 旗下的嵌入式实时内核，未来也将融入鸿蒙生态。
- 我们的目标不是为了构建一个完整的操作系统，而且通过对一款商业内核的拆解、简化，并从0开始构建来了解操作系统中的关键技术和算法等。
- 每个实验需理解的关键代码为数十行到数百行之间，同学们需要自行撰写的代码在数行到数十行之间，完全在同学们可接受范围内。
- 最终我们构建操作系统如下，该系统支持：

    .. image:: ./miniEuler.gif

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

- 让我们从0开始吧😄💪🏻

- 实验设计与指导书编写： 湖南大学 李蕊[`个人页面 <http://csee.hnu.edu.cn/people/lirui>`_]
- 贡献者列表（排名不分先后）
    - vastina （湖南大学） 
