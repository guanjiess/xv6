操作系统必须具备的三大功能：Multiplexing, isolation, interaction

==本章主题==：概括介绍如何组织操作系统以满足上述的三个功能，即复用、交互、分离；同时概括性介绍了xv6的运行过程

**RISC-V**

Xv6 runs on a multi-core RISC-V microprocessor, and much of its low-level functionality (for example, its process implementation) is speciﬁc to RISC-V.

**RISC-V** is a 64-bit CPU, and xv6 is written in “LP64” C, which means long (L) and pointers (P) in the C programming language are 64 bits, but int is 32-bit. This book assumes the reader has done a bit of machine-level programming on some architecture, and will introduce RISC-V-speciﬁc ideas as they come up.

A useful reference for RISC-V is “*The RISC-V Reader: An Open Architecture Atlas*”,  *The user-level ISA* and *the privileged architecture* are the ofﬁcial speciﬁcations.

==物理资源的抽象==

> file system abstraction
>
> process abstraction
>
> memory abstraction
>
> file descriptor abstraction

Unix系统通过其设计的fork\ exit \ wait \ kill \ read \ write \ pipe 等系统调用，为程序员提供了便利和比较强的分离性 isolation。

> eg. exec isolate memory, files isolate disks.

**OS和应用之间的关系**

- OS should be defensive
- application cannot crash the OS
- application cannot break out its isolation
- Strong isolation between OS and application



###硬件对于Isolation的支持

硬件对于隔离的支持体现在两个方面

1. 处理器的user mode 和 kernel mode，通过两种模式实现kernel程序和user
2. 虚拟内存 page table，每一个进程都会有自己独立的page table，这样的话，每一个进程只能访问出现在自己page table中的物理内存。操作系统会设置page table，使得每一个进程都有不重合的物理内存，这样一个进程就不能访问其他进程的物理内存，因为其他进程的物理内存都不在它的page table中。 

==User/Supervisor mode & System calls==

**RISC-V CPU三种运行指令的模式**：machine mode, supervisor mode, user mode 

machine mode模式主要用来做开机初始化；**课程基本用不上**

**Kernel space 内核态**， The software running in *kernel space* (or in supervisor mode) is called the kernel mode, which can also execute **privileged instructions** and is said to be running in *kernel space*.  

- 特殊权限指令主要是一些直接操纵硬件的指令和设置保护的指令，例如设置page table寄存器、关闭时钟中断。

**User space 用户态**， An application can execute only user-mode instructions (e.g., adding numbers, etc. **unprivileged instructions**) and is said to be running in *user space* 

-  普通权限的指令都是一些常见指令，例如将两个寄存器相加的指令ADD、将两个寄存器相减的指令SUB、跳转指令JRC、BRANCH指令等等。 

如果一个应用(application)想去调用一个核函数，必须从用户态切换到内核态。用户态没有权限调度系统调用。

**pagetable**：虚拟内存，目的是实现进程之间的内存隔离(memory isolation)，绝大多数CPU都支持



###Syscall-Ecall

用户程序使用系统调用 (system call)来和系统内核进行数据交互，这里调用的流程如下

> 1. 在用户程序中执行系统调用，如user/echo.c中执行了write()。
>
> 2. write()系统调用在user/user.h和usys.pl中定义，makefile执行usys.pl生成汇编代码，可通过ecall指令根据系统调用号使用系统调用(ecall指令可实现从用户态到内核态的切换)
>
> 3. 执行系统调用函数
>
>    参考：https://zhuanlan.zhihu.com/p/547418974

==添加系统调用流程==

https://fanxiao.tech/posts/2021-03-02-mit-6s081-notes/#25-lab-2-system-calls

- 在`user.h`中声明需要的函数，如`int trace(int)`
- 在`user/usys.pl`注册一个ecall到kernel的kernel态的trace的入口：`entry("trace");` 
- 在`syscall.h`中声明一个系统调用号码，如` #define SYS_trace  22`
- 在`syscall.c`的系统调用表syscalls中添加系统调用号和对应的系统调用处理函数。例如

```C
extern uint64 sys_chdir(void);
extern uint64 sys_uptime(void);
extern uint64 sys_trace(void);

static uint64 (*syscalls[])(void) = {
[SYS_chdir]   sys_chdir,
[SYS_uptime]  sys_uptime,
[SYS_trace]   sys_trace,
};
```

- 在相应的源文件中实现系统调用
- 调试还不太会。。。



###内核的组织

**宏内核**：操作系统直接在内核上运行，所有的系统调用直接运行在supervisor mode. 

优缺点：简单、易合作；复杂的交互

**微内核**

> **To reduce the risk of mistakes in the kernel, OS designers can** **minimize the amount of operating system code that runs in supervisor mode**, and execute the bulk of the operating system in user mode. This kernel organization is called a *microkernel*. 

两种结构各有优缺点，Unix系统采用宏内核结构。Xv6的内核结构是宏内核，即xv6的内核接口对应了系统接口，其内核实现了整个操作系统。内核组成文件如下：

![1699238715944](.\Chap2_Operating System Organization.assets\1699238715944.png)

>  一旦有了对于内核的控制能力，你可以做任何事情，因为内核控制了所有的硬件资源。 

==进程==

> The unit of isolation in xv6(as in other Unix operting systems) is a process.
>
> https://www.ruanyifeng.com/blog/2013/04/processes_and_threads.html

进程的地址空间、进程的抽象、进程的状态：`p->state   p->pagetable`等

**In summary, a process bundles two design ideas:**

- **An address space to give a process the illusion of its own memory.**
- **A thread to give the process the illusion of its own CPU.** 



==启动xv6系统的第一个进程==

可以通过在_entry加断点的方式，用gdb一步步看启动过程用了哪些代码。

看lec3  1:08之后的内容



###源码基本内容

> proc.h : 包含了操作系统内核中用于管理进程核处理系统陷阱的和核心数据结构
>
> defs.h： 这个头文件包含了一系列函数和结构体的声明，这些函数和结构体用于操作系统内核的各个组件，如文件系统、进程管理、内存管理等。这个头文件的目的是为了在代码的不同部分提供函数和结构体的声明，以便在各个模块中使用它们，而无需在每个模块中重新定义这些函数和结构体。 方便不同模块之间的交互、合作。
>
> entry.S： 汇编代码的目的是为每个CPU设置一个栈，然后跳转到C代码的入口点，启动操作系统的初始化过程。然后，它进入一个无限循环，等待其他操作系统组件的执行。 
>
> main.c： 总之，`main` 函数是操作系统内核的入口点，负责执行各种初始化操作，并最终启动进程调度，以便操作系统可以处理用户程序的执行。它还通过 `started` 变量来协调多个CPU的初始化过程。 
>
> initcode.S： 这段代码的主要任务是在用户空间中启动一个进程，该进程执行 `/init` 程序。它设置命令行参数，然后使用 `SYS_exec` 系统调用执行 `/init`，并无限循环等待。一旦 `/init` 进程执行完毕并退出，它也会通过 `SYS_exit` 系统调用退出。这段代码充当了初始化用户进程的入口，通常在操作系统启动时执行。 
>
> init.c： 总体来说，这段代码的作用是初始化操作系统，并启动一个无限循环，用于管理shell进程，以便用户可以通过该shell与操作系统进行交互。 
>
> proc.c(skim)： 是一个用于操作系统内核的一部分，负责管理进程的创建、调度、终止和资源管理等任务。这些功能是操作系统核心的一部分，用于支持多任务处理和用户程序的执行。
>
> exec.c(skim)： 操作系统中的exec系统调用的实现，用于加载并执行一个新的用户程序。
>
> kernel/syscall.h：定义了一些系统调用的常量。在操作系统或者类似的软件中，系统调用是应用程序与操作系统内核之间进行交互的一种方式。每个系统调用都有一个唯一的标识符，这些标识符通常是整数值。这里的代码定义了一些系统调用的标识符
>
> kernel/syscall.c：一个简化的操作系统内核中处理系统调用的部分，用于在操作系统内核中处理用户空间程序的系统调用请求。 
>
> user/user.h： `user.h` 提供了用户程序可以调用的系统调用**接口**的声明，`ulib.c` 中实现了一些用户库函数，为用户程序提供了一些常用的工具和功能。 
>
> usys.pl：Perl 脚本，用于自动生成一个包含系统调用桩的汇编代码文件，这些桩用于用户程序与操作系统内核之间的通信。 



###通过Makefile编译运行Kernel

1. Makefile读取C文件如proc.c，调用gcc编译器，生成汇编文件proc.S
2. 调用汇编解释器，利用汇编文件proc.S生成proc.o
3. Makefile将对所有的内核文件做相同操作，如pipe.C-->pipe.S-->pipo.o
4. 系统加载器（Loader）会收集所有的.o文件，将它们链接在一起，并生成内核文件kernel（以及kernel.asm） 































