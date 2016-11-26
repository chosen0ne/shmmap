shmmap
======

*This is written 5 years ago, the code quality is not quite good because of readability, structure and so on*

shmmap is key-value map in shared memory which can be used in multi-process. As no locks are used, so no more than one process can write the map, and others can read it. It can be used as inter-process communication which can transmit data from one to many.

##Features
* Map operations are supported, such as put, get, iteration, contains...
* Support node bindings which can use shmmap in nodejs.

##Compile
Just make it.

    > make

After *make*, a runnable binary named 'shmmap_test' in *src* directory is generated. You can run it to experience. A simple console prompt is used, like this:

    > -INFO- [m_init]Init memory pool, address start at 0x10c2fa014, end at 0x10cc83694, size is 10000000
    > map size 0, operation: 1:put, 2:get, 3:iter, 4:free list, 5:free size, 6:exit
    > 1
    > key:
    > aaa
    > value:
    > bbb
    > map size 1, operation: 1:put, 2:get, 3:iter, 4:free list, 5:free size, 6:exit
    > 1
    > key:
    > rrr
    > value:
    > yyy
    > map size 2, operation: 1:put, 2:get, 3:iter, 4:free list, 5:free size, 6:exit
    > 3
    > (rrr)=>(yyy)
    > (aaa)=>(bbb)
    > map size 2, operation: 1:put, 2:get, 3:iter, 4:free list, 5:free size, 6:exit

You can also run it in multi processes meanwhile, to see the magic power of the *shmmap* (Write maded in one process can be see by other processes).
The example code can be found in src/shmmap_test.c

##Node Binding

    > make node

In *ext/node/build/Release*, 'shmmap.node' is generated which is node module. And the example code 'shmmap_test.js' is also is *ext/node*.

*Enjoy it*
