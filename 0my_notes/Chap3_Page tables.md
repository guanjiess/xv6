目的是实现物理地址上的隔离，isolation



### 地址空间

每个程序都运行在自己的地址空间，并且这些地址空间彼此之间相互独立，以实现程序之间相互隔离。

基本想法是每个应用程序都有自己独立的表单（页表），并且这个表单定义了应用程序的地址空间。所以当操作系统将CPU从一个应用程序切换到另一个应用程序时，同时也需要切换SATP寄存器（保存当前根页表）中的内容，从而指向新的进程保存在物理内存中的地址对应表单。这样的话，cat程序和Shell程序中相同的虚拟内存地址，就可以翻译到不同的物理内存地址，因为每个应用程序都有属于自己的不同的地址对应表单。 

**virtual address**

RISC-V指令操作的是64位虚拟地址，而计算机的RAM使用的是物理地址，运行程序的时候需要把虚拟地址翻译成物理地址。起到这个翻译作用的桥梁是页表，page table. 即，我们需要用pagetable访问内存。

==虚拟内存的好处==

- 第一个是Isolation，隔离性。虚拟内存使得操作系统可以为每个应用程序提供属于它们自己的地址空间。所以一个应用程序不可能有意或者无意的修改另一个应用程序的内存数据。虚拟内存同时也提供了用户空间和内核空间的隔离性。

- 另一个好处是level of indirection，提供了一层抽象。处理器和所有的指令都可以使用虚拟地址，而内核会定义从虚拟地址到物理地址的映射关系。



### Paging hardware

页表并不是作为一个包含了个PTE的大列表存储在物理内存中的，而是采用了三级树状的形式（page directory）进行存储，这样可以让页表分散存储。每个页表就是一页。第一级页表是一个4096字节的页，包含了512个PTE（因为每个PTE需要8字节），每个PTE存储了下级页表的页物理地址，第二级列表由512个页构成，第三级列表由512*512个页构成。因为每个进程虚拟地址的高27位用来确定PTE，对应到3级页表就是最高的9位确定一级页表PTE的位置，中间9位确定二级页表PTE的位置，最低9位确定三级页表PTE的位置。如下图所示。第一级根页表的物理页地址存储在`satp`寄存器中，每个CPU拥有自己独立的`satp`

- Xv6采用三级页表。每个页表都是一页，有4096个字节和512个PTEs（页表项）
- 第一级页表（根页表）的物理地址存储在`satp`寄存器中，根页表存储了包含512个PTE，每个PTE中包含了第二级页表的页物理地址
- 第二级列表由512个页构成，每一页包含512个PTE，每个PTE中包含一个三级页表的物理地址
- 第三级页表由512*512个页构成，每个PTE确定VA的物理页地址。

 <img src=".\Chap3_Page tables.assets\1700710994936.png" alt="1700710994936" style="zoom:67%;" />

- 第一个标志位是Valid。如果Valid bit位为1，那么表明这是一条合法的PTE，你可以用它来做地址翻译。对于刚刚举得那个小例子（应用程序只用了1个page的例子），我们只使用了3个page directory，每个page directory中只有第0个PTE被使用了，所以只有第0个PTE的Valid bit位会被设置成1，其他的511个PTE的Valid bit为0。这个标志位告诉MMU，你不能使用这条PTE，因为这条PTE并不包含有用的信息。

- 下两个标志位分别是Readable和Writable。表明你是否可以读/写这个page。
- Executable表明你可以从这个page执行指令。
- User表明这个page可以被运行在用户空间的进程访问。

> 为什么是PPN存在这些page directory中？为什么不是一个虚拟内存地址？
>
> 某学生回答：因为我们需要在物理内存中查找下一个page directory的地址。
>
> Frans教授：是的，我们不能让我们的地址翻译依赖于另一个翻译，否则我们可能会陷入递归的无限循环中。所以page directory必须存物理地址。那SATP呢？它存的是物理地址还是虚拟地址？

其他标志位并不是那么重要，他们偶尔会出现，前面5个是重要的标志位

###virtual address configuration in Xv6





###kernel address space



###Process address space

1. One page table per process

   Each process has a separate page table, and when xv6 switches between processes, it also changes page tables. 

2. 



VA: Virtual address，VPN和offset的组合

PA: physical address，PFN/PPN和offset的组合

offset:偏移量的位宽决定了页的大小，以xv6为例，12位宽对应着4KB

VPN: virtual page number

PTE: page table entry 

PPN: physical page number 

Page table，页表，记录着从VPN到PPN的映射关系

va到pa的转换过程使用了三级的页表。









主要涉及的函数和文件

> kalloc.c
>
> vm.c
>
> exec.c
>
> memlayout.h
>
> exec(), walk() 、：

可以看一看OSTEP这本书



==问题==

readelf命令，elf文件

makefile是什么？

kinit()

freerange: 用来挂载内存

kvminit()  

kernel.ld

gdb命令：focus cmd





听课的感想，MIT的学生会有很多的问题想问出来，讲课的老师也很乐于去回答这些问题；再者就是能看出来MIT 的学生在上课之前是做了充分的准备的，所以他们有问题能问。再然后，这本小册子和代码，看一遍是绝对不够的。

