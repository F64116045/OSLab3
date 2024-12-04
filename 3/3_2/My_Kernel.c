#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>
#define procfs_name "Mythread_info"
#define BUFSIZE 1024

char buf[BUFSIZE];
static DEFINE_MUTEX(proc_mutex); // lock


static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset) {
    int len;
    if (buffer_len >= BUFSIZE) {
        return -EINVAL;
    }

    mutex_lock(&proc_mutex);
    //把 Thread X Say Hello 從ubuf用進buf
    if (copy_from_user(buf, ubuf, buffer_len)) {
        return -EFAULT;
    }
    
    //移動到Thread X say hello後面(p)
    char *p = buf + buffer_len;
    int remaining = BUFSIZE;
    //把thread資訊放進去
    len = snprintf(p, remaining, "PID: %d  TID: %d, Priority: %d, State: %d\n",
                current->tgid, current->pid, current->prio, current->__state);

    mutex_unlock(&proc_mutex);
    
    return buffer_len + len;
}

static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset) {
    int len;
    char *p = buf;
    int remaining = BUFSIZE;

    if (*offset > 0) {
        return 0; // EOF
    }

    mutex_lock(&proc_mutex);

    len = snprintf(p, BUFSIZE, "%s\n", buf);
    p += len;
    remaining -= len;
/*
    for_each_thread(current, thread) {
            if(thread->pid == thread->tgid)
                continue;
            len = snprintf(p, remaining, "PID: %d  TID: %d, Priority: %d, State: %ld\n",
                           thread->tgid, thread->pid, thread->prio, thread->__state);
            if (len >= remaining) {
                break;
            }
            p += len;
            remaining -= len;
        }*/
        mutex_unlock(&proc_mutex);


    if (copy_to_user(ubuf, buf, p - buf)) 
        return -EFAULT;
    *offset += p - buf;
    return p - buf;
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void){
    proc_create(procfs_name, 0644, NULL, &Myops);   
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void){
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
