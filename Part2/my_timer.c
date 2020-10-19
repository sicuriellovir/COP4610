#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/timekeeping32.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#define BUFSIZE  500
 
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SEANILU");
 
// to save last called time. 
static struct timespec preTime={0}; 

// to generate /proc/timer file
static struct proc_dir_entry *ent;

// Adding wrapper of current_kernel_time as it is Deprecated
/*
	The current_kernel_time() function got deprecated and moved to timekeeping32.h in v4.15 
	and then completely removed in v4.20.
	Newer timekeeping functions were introduced in v4.18.
	
	Ref: https://stackoverflow.com/questions/59828144/what-is-the-equivalent-of-current-kernel-time-in-linux-kernel-v5
*/
static inline struct timespec current_kernel_time(void)
{
    	struct timespec curTime;	
    	struct timespec64 lTime;
    	ktime_get_coarse_real_ts64(&lTime);
	curTime.tv_sec  = lTime.tv_sec;
	curTime.tv_nsec = lTime.tv_nsec;
   	return curTime;
} 

// Find the differnce between two time
void timediff(struct timespec *prev, struct timespec *cur,
                   struct timespec *diff)
{
    if ((cur->tv_nsec - prev->tv_nsec) < 0) {
        diff->tv_sec = cur->tv_sec - prev->tv_sec - 1;
        diff->tv_nsec = cur->tv_nsec - prev->tv_nsec + 1000000000;
    } else {
        diff->tv_sec = cur->tv_sec - prev->tv_sec;
        diff->tv_nsec = cur->tv_nsec - prev->tv_nsec;
    }

    return;
}
 
// write function: not implemented as it is not needed 
static ssize_t my_timer_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{
	printk( KERN_DEBUG "write for my_timer not suppoorted\n");
	return -1;
}
 
 // read function
static ssize_t my_timer_read(struct file *file, char __user *ubuf,size_t count, loff_t *ppos) 
{
	char buf[BUFSIZE];
	int len=0;
	struct timespec curTime;

	// error checking
	if(*ppos > 0)
	{
		return 0;
	}
 
	// get time
	curTime = current_kernel_time();
	
	// generate output string
	len += sprintf(buf,"current time: %ld.%ld \n",curTime.tv_sec,curTime.tv_nsec);
	if( (preTime.tv_sec > 0) || (preTime.tv_nsec > 0))
	{
		struct timespec diffTime = {0};

		timediff(&preTime, &curTime, &diffTime);
		len += sprintf(buf + len,"elapsed time: %ld.%ld \n",(diffTime.tv_sec), (diffTime.tv_nsec));
	}
	
	// saving current time.
	preTime.tv_sec = curTime.tv_sec;
	preTime.tv_nsec = curTime.tv_nsec;

	// copying data to user space
	if(copy_to_user(ubuf,buf,len))
	{
		return -EFAULT;
	}

	*ppos = len;
	return len;
}
 
 // file operation structure
static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = my_timer_read,
	.write = my_timer_write,
};
 
 // module init function
static int mytimer_init(void)
{
	ent=proc_create("timer",0660,NULL,&myops);
	printk(KERN_ALERT "Init my_timer...\n");
	return 0;
}

// module cleanup function 
static void mytimer_cleanup(void)
{
	proc_remove(ent);
	printk(KERN_WARNING "Cleaning my_timer ...\n");
}
 
module_init(mytimer_init);
module_exit(mytimer_cleanup);
