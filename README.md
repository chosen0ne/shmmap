shmmap
======

shmmap is key-value map in shared memory which can be used in multi-process. As no locks are used, so no more than one process can write the map, and others can read it. It can be used as inter-process communication which can transmit data from one to many.
