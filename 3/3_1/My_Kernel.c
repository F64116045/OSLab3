#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /* Do nothing */
	return 0;
}


static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset) {
    struct task_struct *task, *thread;
    char *p = buf;
    int remaining = BUFSIZE;
    int len;

    pid_t target_pid = current->pid;

    if (*offset > 0) {
        return 0;
    }

    for_each_process(task) {
        if (task->pid != target_pid) {
            continue;
        }


        len = snprintf(p, remaining, "PID: %d", task->pid);
        if (len >= remaining) {
            break;
        }
        p += len;
        remaining -= len;

        for_each_thread(task, thread) {
            if(thread->pid == task->pid) continue;
            len = snprintf(p, remaining, "  TID: %d, Priority: %d, State: %ld\n",
                           thread->pid, thread->prio, thread->__state);
            if (len >= remaining) {
                break;
            }
            p += len;
            remaining -= len;
        }
    }

    /*
    for_each_thread(current, thread) {
            len = snprintf(p, remaining, "PID = %d,  TID: %d, Priority: %d, State: %ld\n",
                           thread->tgid, thread->pid, thread->prio, thread->__state);
            if (len >= remaining) {
                break;
            }
            p += len;
            remaining -= len;
        }

    if (copy_to_user(ubuf, buf, p - buf)) {
        return -EFAULT;
    }*/


    if (copy_to_user(ubuf, buf, p - buf)) {
        return -EFAULT;
    }

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