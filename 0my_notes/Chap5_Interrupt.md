预备知识

> Chapter5, kernel/kernelvec.S, plic.c, console.c, uart.c, printf.c

###中断

每个设备都需要一个驱动，常见的外设：console, keyboard, mouse, display等等。在Xv6里的console用uart进行数据传输。

**中断和系统调用的区别**

- Asynchronous，外设的中断和CPU上运行的进程没有任何关联，是异步的。对于系统调用而言，其发生在进程的背景下。
- Concurrency，并发。中断的执行和CPU的执行是并行的。 

**中断类型**

- software interrupt
- timer interrupt
- external interrupt（**重点讨论**）

### PLIC

设备中断控制器，platform level interrupt controller. 用来给CPU分发中断。其工作流程如下

> 1.PLIC会通知当前存在一个中断
>
> 2.某个CPU会确认收到中断，则其他CPU不会处理该中断
>
> 3.CPU处理完中断后会给PLIC返回complete信息
>
> 4.PLIC清除中断信息

![1701864166691](.\Chap5_Interrupt.assets\1701864166691.png)





###xv6的中断配置



###Top-Bottom

- 驱动程序用来控制设备行为，通常包括top和bottom两部分。通过驱动程序的top和bottom对驱动程序内的缓存队列进行读写，可以实现并行设备和CPU的解耦==？不太理解==。
- top部分是用户进程or内核的其他调用接口，用来和用户进程进行数据交互。

- bottom部分是interrupt handler，即中断处理程序，用来处理中断。在本课程中用qemu模拟16550芯片设备（键盘），来与console进行交互。一般而言硬件设备会暴露出一些寄存器，CPU通过load/store等指令来操作寄存器，进而控制设备。

> 设备上的 I/O 端口通常映射到寄存器。在计算机体系结构中，每个外设都会被分配一个或多个 I/O 端口，这些端口实际上是与外设相关联的寄存器的地址。
>
> 这些寄存器用于在 CPU 和外设之间传递数据、状态信息和控制信息。通过特定的 I/O 指令，CPU 可以通过地址总线和数据总线与这些寄存器进行通信。CPU通过读取或写入这些寄存器中的内容，与外设进行数据交换、控制外设的操作，并获取外设的状态信息。
>
> 通常，与外设相关的寄存器会被映射到系统的内存地址空间中，使得 CPU 可以通过访问内存的方式来与外设进行通信。这种映射是由计算机体系结构和操作系统的设计规定的。



==v6启动后，console显示出提示符$的过程：top--->bottom，自上而下的视角==

shell输出到console的每一个字符都会触发write系统调用。大概流程如下

**uart 驱动的top**

> - 第一个用户进程init.c执行sh用户程序-->sh调用fprintf函数-->frintf函数引用了系统调用的用户接口write()，继而转入内核态
> - 进入内核态，开始执行系统调用，sys_write--->file_write调用console的write函数(devsw[f->major].write)--->uartputc--->uartstart，最终将数据写入uart的THR寄存器（transmit holding register）--->系统调用返回，返回机制同trap的返回机制。

**uart驱动的bottom**

> - 键盘生成中断并发向PLIC，PLIC将中断分发给某个CPU进行处理。如果该CPU的SIE寄存器有效（可处理设备中断），那么会进行如下操作、
>   1. 清除SIE寄存器相应的bit，这样可以阻止CPU核被其他中断打扰，该CPU核可以专心处理当前中断。处理完成之后，可以再次恢复SIE寄存器相应的bit。 
>   2. 保存当前的PC到SEPC寄存器。
>   3. 保存当前的工作模式，并将工作模式设为Supervisor mode.
>   4. 将PC的值设为STVEC的值（stvec保存trap handler，中断处理函数的地址，以运行在用户空间的shell为例子，其stvec保存的值为uservec函数地址（在trampoline里），uservec会调用usertrap函数） 
>
> - 进入usertrap函数--->进入devintr()    (trap.c，第68行)
> - devintr()根据scause判断其是否为外设中断，如果是，由plic_claim获取其中断号irq，根据irq选择相应的中断执行函数，这里进入uartintr()
> - 由uartintr()跳转至uartstart()，==细节问题==，这里uartstart会把getcmd初始化时$后的空格输出，但是为什么？



==从键盘输入到console显示的过程：bottom-->top，自底向上的视角==

流程

> 0.在sh初始化getcmd输出`$  `
>
> 1.sh的getcmd调用gets()，gets()调用read
>
> 2.read()是系统调用sys_read的用户接口，切换至内核态后执行系统调用sys_read
>
> 3.sys_read()调用fileread()
>
> 4.fileread()根据文件类型调用consoleread()，在没有字符输入时，进入sleep状态。直至键盘输入被保存到buffer队列。
>
> 5.当键盘中断输入时，生成中断并发向PLIC，PLIC将中断分发给某个CPU进行处理。
>
> 6.进入usertrap函数--->进入devintr() --->进入uartintr()
>
> 7.进入consoleintr()，随后调用consputc(c)，将输入的字符显示在console。



（和设备中断有关）

键盘每输入一个字符都会产生一个中断，输入一个字符后会产生一个transmit complete中断

uartintr-->consoleintr-->consoleputc（回显函数）



###并发 concurrency 

计算机运行时，CPU和设备之间时并行的。

驱动程序的top部分和bottom部分也是并行的。



###RISC-V 中断相关寄存器

RISC-V有许多与中断相关的寄存器：

- SIE（Supervisor Interrupt Enable）寄存器。这个寄存器中有一个bit（E）专门针对例如UART的外部设备的中断；有一个bit（S）专门针对软件中断，软件中断可能由一个CPU核触发给另一个CPU核；还有一个bit（T）专门针对定时器中断。
- SSTATUS（Supervisor Status）寄存器。这个寄存器中有一个bit来打开或者关闭中断。
- 每一个CPU核都有独立的SIE和SSTATUS寄存器，除了通过SIE寄存器来单独控制特定的中断，还可以通过SSTATUS寄存器中的一个bit来控制所有的中断。
- SIP（Supervisor Interrupt Pending）寄存器。当发生中断时，处理器可以通过查看这个寄存器知道当前是什么类型的中断。
- SCAUSE寄存器，查看当前trap状态的原因是否为中断。
- STVEC寄存器，它会保存当trap，page fault或者中断发生时，CPU运行的用户程序的程序计数器，这样才能在稍后恢复程序的运行。



###问题

**2023.12.06**

console是什么？

Shell是什么？

在sh输入字符时，Shell把字符存储到哪了？uart输出的缓存队列的字符是怎么保存进来的？

shell怎么调用read？在哪调用的？

**有时间去gdb调试看一看。**

完整地处理中断需要：外设产生中断、PLIC传递中断到CPU、CPU处理中断

