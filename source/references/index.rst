参考资料 
=====================

- 交叉工具链可以从Arm官网的 `GNU Toolchain Downloads <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>`_ 页面下载。
- 完整的寄存器列表可参考 Arm 官网的 `AArch64 System Registers <https://developer.arm.com/documentation/ddi0601/latest/?lang=en>`_ 页面。
- 完整的指令集可参考 Arm 官网的 `Arm A-profile A64 Instruction Set Architecture <https://developer.arm.com/documentation/ddi0602/latest/?lang=en>`_ 页面。
- 详情可参考 `The GNU linker <https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html>`_。此外，这里还有一个简单的 `链接脚本基本介绍 <https://zhuanlan.zhihu.com/p/363308789>`_ 可参考。
- CMake 的命令和参数等可参考 `官网文档 <https://cmake.org/cmake/help/latest/index.html>`_。此外，这里还有一个很好的入门 `博客文章 <https://zhuanlan.zhihu.com/p/500002865>`_。


执行gdb时报错

.. code-block:: console
    
    root@e1370b14f32a:~/netdisk/os2024_exp/os2024_exp_redo/lab4# ../../aarch64-none-elf/bin/aarch64-none-elf-gdb build/miniEuler
    ../../aarch64-none-elf/bin/aarch64-none-elf-gdb: error while loading shared libraries: libncursesw.so.5: cannot open shared object file: No such file or directory
    root@e1370b14f32a:~/netdisk/os2024_exp/os2024_exp_redo/lab4# apt-get install libncursesw5

    root@e1370b14f32a:~/netdisk/os2024_exp/os2024_exp_redo/lab4# ../../aarch64-none-elf/bin/aarch64-none-elf-gdb build/miniEuler
    ../../aarch64-none-elf/bin/aarch64-none-elf-gdb: error while loading shared libraries: libpython3.6m.so.1.0: cannot open shared object file: No such file or directory
    root@e1370b14f32a:~/netdisk/os2024_exp/os2024_exp_redo/lab4# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/anaconda3/lib/
    root@e1370b14f32a:~/netdisk/os2024_exp/os2024_exp_redo/lab4# ../../aarch64-none-elf/bin/aarch64-none-elf-gdb build/miniEuler
