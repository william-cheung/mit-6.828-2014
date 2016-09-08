1. if the kernel executed the following code snippet:
   
   ```
   struct spinlock lk;
   initlock(&lk, "test lock");
   acquire(&lk);
   acquire(&lk);
   ```

   the kernel will panic on the second acquire. (acquire in spinlock.c)

2. 
