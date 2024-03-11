进程切换；

课本更多是知识性的普及；

lecture视频的内容更多；



看起来，system call ---> page table ---> trap ---> interrupt --->lock --->schedule这几章的设计是环环相扣的，每一章都是后面的基础，而后一章又可以为之前的课程起到解释作用。

## 线程

**一、什么是线程**

线程是一个串行执行代码的单元，只占用一个CPU，且逐行地执行程序。通常情况下，我们可以通过保存线程的状态来暂停程序的运行，转而去执行其他的进程，该线程可在之后恢复运行。一个线程的状态包括了三个部分

- Program Counter，表示了当前的程序运行位置
- 保存各变量的寄存器
- 程序的栈  Stack

**二、为什么要用多线程**

主要有以下几个原因

- 希望计算机可以同时执行多个任务。
- 多线程可以简化程序，减少复杂度。
- 多线程通过并行计算的方式提高了计算机的处理速度， 常见的方式是将程序进行拆分，并通过线程在不同的CPU核上运行程序的不同部分 

**多线程的并行运行策略**

1. 在多核处理去上使用多个CPU，在每个CPU上运行一个独立的线程。
2. 在同一个CPU上，不断地进行线程的切换。xv6主要使用该策略来管理进程。

XV6结合了这两种策略，首先线程会运行在所有可用的CPU核上，其次每个CPU核会在多个线程之间切换，因为通常来说，线程数会远远多于CPU的核数。 现实中的其他操作系统也多结合两种策略管理进程。

==内存的共享==：线程之间是否共享内存是不同线程系统的一个主要区别。如果多个线程都在同一个地址空间内运行，则它们可以看到彼此的更新。比如：共享一个地址空间的线程修改了一个变量，共享地址空间的另一个线程可以看到变量的修改。所以当多个线程运行在一个共享地址空间时，**需要用锁**。 



###进程和线程





## XV6线程调度

==一、内核中线程调度的主要困难==

- 如何实现线程间的切换。 （scheduler）
- 当执行线程的切换时，保存线程的哪些信息
- 怎么处理运算密集的线程



**进程相关的数据结构**

```C
// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
};
```

- trapframe：保存用户线程寄存器
- context：保存内核线程寄存器
- kstack：保存当前进程的内核栈，进程在内核中执行时保存函数调用的位置 



**内核线程**

==CPU调度器线程==

- CPU在一个时刻只能运行一个线程。--->在xv6中，是用户进程的线程  、内核线程  、 CPU的调度器线程之一。
- 每个CPU都有一个完全不同的调度器线程，调度器线程也是一种内核线程，拥有自己的context对象， 调度器线程的context对象保存在cpu结构体中。每一个调度器线程都有自己独立的栈。
- 所有的内核线程都共享同一块内核内存。 

```C
// Per-CPU state.
struct cpu {
  struct proc *proc;          // The process running on this cpu, or null.
  struct context context;     // swtch() here to enter scheduler().
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};
```

任何运行在CPU1上的进程，当它决定出让CPU，它都会切换到CPU1对应的调度器线程，并由调度器线程切换到下一个进程。 



==切换线程时保存的Context对象==

```C
// Saved registers for kernel context switches.
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};
```



本章关注的几个线程状态

- RUNNING，正在某个CPU上运行的线程
- RUNABLE，暂未运行的线程，如果某个CPU有空闲，则立即可以运行。
- SLEEPING。等待某些I/O事件发生状态，这些I/O事件发生后，即可运行。



两类寄存器的保存

1. 用户寄存器，保存在trapframe
2. 内核线程寄存器保存在context



==线程调度的流程==

![1702629158423](.\Chap7_Scheduling.assets\1702629158423.png)

> 1. CPU的通过定时中断触发trap过程，以实现用户进程的切换。假设被中断的用户进程为进程A。
> 2. 用户进程A的用户状态（PC、各种寄存器）被保存在trapframe。代码执行到trampoline和usertrap()，用户进程进入内核，为内核线程A，usertrap判断中断类型后，执行yield。
> 3. yield调用sched，sched对进程A做必要的检查，随后调用swtch，swtch是切换进程的核心
> 4. swtch将内核线程A切换为当前CPU的调度线程。将内核线程A的context对象保存，CPU0保存的context被恢复，此后会在调度器线程的context下执行scheduler函数。
> 5. scheduler函数遍历进程表单，找到一个可执行的进程B（由某种原因被中断执行，该进程的用户状态被保存在trapframe中，以及其对应的内核寄存器也被保存在context里）。随后，scheduler通过swtch恢复了B的context，使得B返回到原来所在的中断活系统调用处理程序。
> 6. 内核线程B继续完成中断处理程序
> 7. 通过B的trapframe恢复此前的用户状态，返回到用户空间的B程序。
> 8. 恢复用户进程B的执行

所以，用户进程的切换必然包括这几步

1. 第一个用户进程进入内核，保存用户状态信息到trapframe，随后执行相应的内核进程
2. 从第一个内核进程切换到第二个内核进程
3. 第二个用户进程的内核进程恢复第二个用户进程的用户寄存器
4. 返回第二个用户进程



关键的函数：yield, sched, swtch, scheduler，核心是swtch函数



push_off 中断关闭

pop_off 中断打开







