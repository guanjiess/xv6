### trap原因

导致CPU中断当前指令的执行，转而去处理当前事件的情况，统称为trap。

- 系统调用：system call, when a user program executes the ecall instruction to ask the kernel to do something for it.
- 异常处理：exception, an instruction does something illegal, such as divide by 0 or use an invalid address.
- 设备中断：when a device signals that it needs attention



###trap思路流程

1. A trap forces a transfer of control into the kernel . 当有用户态切换至内核态进入supervisor mode时，你可以：读写SATP寄存器，也就是page table的指针；STVEC，也就是处理trap的内核指令地址；SEPC，保存当发生trap时的程序计数器；SSCRATCH等等。在supervisor mode你可以读写这些寄存器，而用户代码不能做这样的操作。 
2. the kernel saves registers and other state (pc, satp, k stack, mode...)so that execution can be resumed
3. the kernel executes appropriate handler code (e.g., a system call implementation or device driver)
4. the kernel restores the saved state and returns from the trap.
5. the original code resumes where it left off.



###Xv6 trap处理阶段

1. RISC-V CPU执行硬件操作，保存\更改一些寄存器等等
2. 汇编指令为C内核代码做准备（uservec  /  kernelvec）
3. C函数内执行trap作（  usertrap(), kerneltrap()  ）
4. 针对触发trap的类型，执行需要的操作，如system call, device-driver service routine or just kill the process.



以用户程序shell 执行write的系统调用为例，其经历的系统调用过程如下

> 1. Shell在用户空间执行了ecall指令。ecall会完成几件事情
>    - ecall指令会设置当前为supervisor mode
>    - 保存程序计数器到SEPC寄存器
>    - 将程序计数器设置成控制寄存器STVEC的内容。STVEC是内核在进入到用户空间之前设置好的众多数据之一，内核会将其设置成trampoline page的起始位置。所以，当ecall指令执行时，ecall会将STVEC拷贝到程序计数器。之后程序继续执行，但是却会在当前程序计数器所指的地址，也就是trampoline page的起始地址执行。 
>
> 2. 进入trampoline.S代码，执行uservec。该步会保存用户程序的相关寄存器内容到trapframe里，并且把trapframe里预先存好的kernel pagetable， kernel stack等寄存器的值写入 satp , tp 等等寄存器内。
> 3. 执行usertrap()，该函数在trap.c中。会确定发生trap的原因，并且执行相关的操作。如果是系统调用，会执行syscall()函数。
> 4. 执行usertrapret()，保存kernep pagetable, kernel stack, usertrap等内容，以供下一次从用户空间转换到内核空间时可以用到这些数据。 恢复此前在SEPC存储的pc值，调转到userret函数中等等操作。
> 5. 执行userret。



trapframe中包含用户进程进入、离开内核时的数据。





###xv6在kernel处理trap的原因

- handling traps in kernel is natural for system calls
- it **makes sense for interrupts** since isolation demands that only the kernel be allowed to use devices, and kernel is a convenient mechanism with which to share devices among multiple process.
-  It also makes sense for exceptions since xv6 responds to all exceptions from user space by killing the offending program. 
-  isolation



###重要的寄存器

- **stvec**: The kernel writes the **address of its trap handler( uservec or kernelvec)** here; the RISC-V jumps to the address in stvec to handle a trap.
- **sepc**: When a trap occurs, RISC-V **saves the program counter here** (since the pc is then overwritten with the value in stvec). The sret (return from trap) instruction copies sepc to the pc. The kernel can write sepc to control where sret goes.
- **scause**: RISC-V puts a number here that **describes the reason for the trap.**
- <img src=".\Chap4_Trap.assets\1701315928707.png" alt="1701315928707" style="zoom: 67%;" />
- **sscratch**: The kernel places a value here that comes in handy at the very start of a trap handler. 一般情况下保存的是指向trapframe的地址。
- **sstatus**: The **SIE** bit in sstatus controls whether device interrupts are enabled. If the kernel clears SIE, the RISC-V will defer device interrupts until the kernel sets SIE. The **SPP** bit indicates whether a trap came from user mode or supervisor mode, and controls to what mode `sret` returns.
- **SIE（Supervisor Interrupt Enable）**寄存器：这个寄存器中有一个bit（SEIE）专门针对例如UART的外部设备的中断；有一个bit（SSIE）专门针对软件中断，软件中断可能由一个CPU核触发给另一个CPU核；还有一个bit（STIE）专门针对定时器中断。我们这节课只关注外部设备的中断。
- **SIP（Supervisor Interrupt Pending）**寄存器。当发生中断时，处理器可以通过查看这个寄存器知道当前是什么类型的中断。
- **satp**: (Supervisor Address Translation and Protection) register holds the (physical) address of the page-table root.
- **stval:** (Supervisor Trap Value) register holds the page-fault (virtual) address

###RISC-V寄存器

![1701521771798](.\Chap4_Trap.assets\1701521771798.png)

· Caller：来访者，简单来说就是打电话的，即调用函数的函数。 Caller Saved寄存器在函数调用的时候不会保存 。

· Callee：被访者，简单来说就是接电话的，即被调用函数。 Callee Saved寄存器在函数调用的时候会保存 。

==寄存器是CPU或者处理器上，预先定义的可以用来存储数据的位置。==寄存器之所以重要是因为汇编代码并不是在内存上执行，而是在寄存器上执行，寄存器是用来进行任何运算和数据读取的==最快==的方式 。RISC-V所有的寄存器都是64bit 。



**Instruction set architecture, ISA. 指令集架构**

ISA主要分为复杂指令集（Complex Instruction Set Computer，CISC）和精简指令集（Reduced Instruction Set Computer，RISC），典型代表如下：

| 类型           | 名称                    | 特点                     | 应用领域            |
| -------------- | ----------------------- | ------------------------ | ------------------- |
| 复杂指令集CISC | x86                     | 性能高 速度快 兼容性好   | PC 服务器           |
| 精简指令集RISC | ARM                     | 生态成熟 非离散 需授权   | 移动设备 嵌入式设备 |
| RISC-V         | 开源 模块化 简洁 可拓展 | 物联网 人工智能 边缘计算 |                     |

相比于x86这种CISC，RISC-V的优势在于

- RISC-V指令数量少
- 指令更简单
- 开源





###C-Assembly

处理器不能理解C语言，其能够处理的是二进制编码，需要先将C语言翻译为汇编代码。 

处理器执行一段C语言程序的流程：C-->汇编-->链接等操作-->生成二进制文件(.o 或者.obj)

ISA是底层硬件电路面向上层软件程序提供的一层**接口规范**，即机器语言程序所运行的计算机硬件和软件之间的“**桥梁**”。ISA主要定义了如下内容：

· 基本数据类型及格式（byte、int、word……）

· 指令格式，寻址方式和可访问地址空间大小

· 程序可访问的通用寄存器的个数、位数和编号

· 控制寄存器的定义

· I/O空间的编址方式

· 异常或中断的处理方式

· 机器工作状态的定义和切换



###函数和栈

每一次调用一个函数，函数都会为自己创建一个Stack Frame，并且只给自己用。函数通过移动Stack Pointer来完成Stack Frame的空间分配。 

![1701525648259](.\Chap4_Trap.assets\1701525648259.png)

Stack Frame中有两个重要的寄存器

- 第一个是SP（Stack Pointer），它指向Stack的底部并代表了当前Stack Frame的位置。
- 第二个是FP（Frame Pointer），它指向当前Stack Frame的顶部。因为Return address和指向前一个Stack Frame的的指针都在当前Stack Frame的固定位置，所以可以通过当前的FP寄存器寻址到这两个数据（如，fp - 8,  fp - 16）。 

