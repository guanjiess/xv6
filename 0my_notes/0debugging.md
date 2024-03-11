

## 工具相关

make是什么？

GNU make rule又是什么？





## 调试

记得释放端口，https://blog.csdn.net/weixin_44465434/article/details/122772336

解决：`Failed to find an available port: Address already in use`

```shell
[deadpool@localhost linux-nova]$ sudo lsof -i tcp:1234
COMMAND       PID USER   FD   TYPE  DEVICE SIZE/OFF NODE NAME
qemu-syst 1008828 root    9u  IPv4 1702471      0t0  TCP *:search-agent (LISTEN)
qemu-syst 1008828 root   10u  IPv6 1702472      0t0  TCP *:search-agent (LISTEN)
[deadpool@localhost linux-nova]$ sudo kill 1008828
```

==qemu==

==gdb==

==用root用户去调试，我也不知道为什么，以后再说==

为什么会卡住？为什么不会进程序

- GDB调试最佳实践步骤**(Debian 11.1)推荐** 
  - 首先进入xv6-labs-2021的目录中；
  - 在一个窗口运行`make CPUS=1 qemu-gdb`，调试端口号为25000
  - 在另一个窗口运行：`gdb-multiarch`→ `file kernel/kernel` → `target remote localhost:25000`

https://cactus-agenda-c84.notion.site/Lab0-Environment-Setup-5a1084d3b695416c8740977b9f1ba77e

https://www.bilibili.com/video/BV1Eb41197wb  30min开始调试

https://www.bilibili.com/video/BV19a411b7E8/  36min开始调试，涉及了很多具体操作，amazing

https://www.cnblogs.com/KatyuMarisaBlog/p/13727565.html  调试文档

问题1. 工作流程是什么？

==还不懂原理==

问题2. 怎么调试kernel代码

> 终端1：make qemu-gdb----->终端2：gdb-multiarch----->b  _entry(也可以在其他段加断点)

问题3 .怎么调试user代码？

> 终端1：make qemu-gdb----->终端2：gdb-multiarch----->file user/_sleep---->b  main(加断点)

问题4：避免出现解决不了的问题，`make CPUs=1 qemu-gdb`



> 1. 终端1：make qemu-gdb
> 2. 终端2：gdb-multiarch
> 3. 终端2：file user/_sleep
> 4. 终端2：b  main(加断点)
> 5. 终端2：c，输入后终端一系统完成启动
> 6. 终端1：输入sleep命令，进入调试界面



###远程调试

远程调试？target remote localhost:26000

 需要加载符号表。符号表是什么？

###gdb常用命令

b 打断点

n 下一行

s  si 单步操作

c continue，执行到断点

finish跳出for循环

p + 变量名，可以打印某个输出



![1700133160304](E:\master2\coding_notes\OS\0开发和调试.assets\1700133160304.png)

layout split模式

focus cmd/src可以把光标聚集在某个tab中

remote thread 1.1 1.2 1.3什么意思？

==需要关注的寄存器==

pc

a0和a1













## 策略调整

qemu是什么，怎么用

C语言从源代码到编译执行经过哪几步骤，怎么用？

ELF文件是什么？类似于_sleep的形式？



gdb是什么，怎么用？reading symbols from ...是什么意思？

focus cmd

为了让自己不被这些工具类的东西耽误时间，我决定从今天开始在linux上刷题，然后用gdb调试，工具这东西，多用就熟了。







## Debug

1. 这种问题，直接make clean

![1698898435920](E:\master2\coding_notes\OS\0debugging.assets\1698898435920.png)

2. 为什么把#include "kernel/types.h"放在前面能编译通过，放在后面就通过不了？--->因为前面的头文件包括了后面头文件需要的定义？

   ```C
   #include "kernel/types.h"
   #include "user/user.h"
   ```


3. 在终端环境下进行多窗口编程时，用vim编辑了同一个文件后出现了警告

```
E325: ATTENTION
Found a swap file by the name ".notes.swp"
          owned by: james   dated: Fri Dec  3 17:38:07 2010
         file name: ~james/school/se/project-dir/rottencucumber/doc/notes
          modified: no
         user name: james   host name: james-laptop
        process ID: 2251 (still running)
While opening file "notes"
             dated: Fri Dec  3 18:46:10 2010
      NEWER than swap file!

(1) Another program may be editing the same file.
    If this is the case, be careful not to end up with two
    different instances of the same file when making changes.
    Quit, or continue with caution.

(2) An edit session for this file crashed.
    If this is the case, use ":recover" or "vim -r notes"
    to recover the changes (see ":help recovery").
    If you did this already, delete the swap file ".notes.swp"
    to avoid this message.

Swap file ".notes.swp" already exists!
[O]pen Read-Only, (E)dit anyway, (R)ecover, (Q)uit, (A)bort:
```

解决方案

https://superuser.com/questions/218262/how-to-get-rid-of-the-warnings-when-opening-a-file-that-has-a-swp-file









## 参考资源

鸟哥的Linux私房菜







![1700205845605](E:\master2\coding_notes\OS\0开发和调试.assets\1700205845605.png)

![1700205992970](E:\master2\coding_notes\OS\0开发和调试.assets\1700205992970.png)