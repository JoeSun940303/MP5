    #include <linux/uaccess.h>
    #include <linux/proc_fs.h>
    #include <linux/seq_file.h>
    #include <linux/module.h>
    #include <linux/kernel.h>
    #include <linux/vmalloc.h>
    #include <linux/slab.h>
    #include <linux/mutex.h>
    #include "mp2_given.h"

    MODULE_LICENSE("GPL");
    MODULE_AUTHOR("Group_ID");
    MODULE_DESCRIPTION("CS-423 MP2");

    //predefine some variable
    #define USER_ROOT_DIR "mp2"
    #define USER_ENTRY1   "status"
    #define DEBUG 1
    static struct proc_dir_entry *pt_root;
    static struct proc_dir_entry *pt_entry1;
    static struct timer_list my_timer;
    static struct workqueue_struct* wq;
    static DEFINE_MUTEX(process_lock);
    static int check =1;
    struct my_list{
         int pid;
         struct list_head list;
         unsigned long cpu_time;
    } ;

    //The macro defination of the list head
    LIST_HEAD(mylist);


    static void update_cpu_time (void){
	    int flag;
	    unsigned long cpu_time=0;
        struct my_list *tmp;
        struct list_head *q;
        struct list_head *list_pos;
        //avoid the different processes to change the list at the same time
        mutex_lock(&process_lock);
        //printk(KERN_ALERT "ready to update CPU time");
        //transverse all the list node and then update the cup_time
        list_for_each_safe(list_pos,q, &(mylist)){
                tmp= list_entry(list_pos, struct my_list, list);
		        //flag = get_cpu_use(tmp->pid,&cpu_time);

		        if(flag<0){
		            list_del(list_pos);
		            kfree(tmp);
		        }
                //transform the cpu time from tricks to seconds
                //tmp->cpu_time =jiffies_to_msecs( cpu_time)/1000;
		        cpu_time =0;
        }
        //printk(KERN_INFO "CPU time updated, can be shown on next cat operation");
        mutex_unlock(&process_lock);
        return;
    }
DECLARE_WORK(myWork, update_cpu_time);
    //this is the work function which will put the work function into the workqueue
    void work_function(void){
   // struct work_struct myWork;//=kmalloc(sizeof(struct work_struct), GFP_ATOMIC);
    
//	    if(!myWork)
//		    return;
        queue_work(wq, &myWork);
    }



    //When the timer expires, the function will be called
    void my_timer_callback(unsigned long data){
         //printk(KERN_INFO "timer is called");
         work_function();
         //The reason why call this is that to reset the timer
         mod_timer(&my_timer, jiffies+msecs_to_jiffies(5000) );
    }

    //check whether the given pid exists in the linked list, if exists, don't add the new value
    bool if_exist( int pid){
        struct my_list *tmp;
        struct list_head *list_pos;
        list_for_each(list_pos, &(mylist)){
            tmp= list_entry(list_pos, struct my_list, list);
            if(tmp->pid == pid)
                return true;
        }
        return false;
    }


    //main function called by write callback function, will add the pid and time 0 into the list
    void add_pid_to_list( int pid){
        //first to see whether there is an existing element in list
        struct my_list * newElement;
        if(if_exist(pid)){
            printk(KERN_INFO "pid exits");
            return;
        }
        //if there is no existing one, put it in the end of the list
        newElement = (struct my_list *)kmalloc(sizeof(struct my_list),GFP_KERNEL);
        if (!newElement) return;
        //printk(KERN_INFO "read to add");
        newElement->pid = pid;
        newElement->cpu_time=0;
        list_add(&(newElement->list), &(mylist));
        //printk(KERN_INFO "element added");
    }


    //the read callback function, which will transverse the list and output the info to the buffer
    ssize_t read_callback(struct file *file, char * buffer, size_t count, loff_t *pos){

        struct my_list *tmp;
        struct list_head *list_pos;
	struct list_head * q;
        int bytes=0;
        char * buf= (char *) kmalloc(PAGE_SIZE*sizeof(char),GFP_KERNEL);

        //this function is to ensure the first time check =1, then second time ,check =0
	 if(check)
                    check =0;
            else{
                    check=1;
		    kfree(buf);
                    return 0;
                }
        mutex_lock(&process_lock);
        //printk(KERN_ALERT "The read call back function is called");
        list_for_each_safe(list_pos,q, &(mylist)){
            tmp= list_entry(list_pos, struct my_list, list);
            bytes= sprintf(buf, "%s%d: %lu\n", buf, tmp->pid, tmp->cpu_time);
        }
        mutex_unlock(&process_lock);

        printk(KERN_INFO "%s", buf);
        copy_to_user(buffer,buf,bytes);
	    //printk("the length is %d",bytes);
        kfree(buf);
	
        return bytes;

    }


    //The write call backfunction, which will take the pid in the buffer and put it into the list
    ssize_t write_callback(struct file *file, const char * buffer, size_t count, loff_t *pos){

        int pid;
        char * tem_buf = (char *) kmalloc ((count+1)*sizeof(char),GFP_KERNEL); //need to be free later
        if(copy_from_user(tem_buf, buffer, count))
                return -EFAULT;
        tem_buf[count]=0;

        //printk(KERN_INFO "The pid is received");
        if(kstrtoint(tem_buf,10,&pid)!=0){
                printk(KERN_ALERT "Invalid process id \n");
                return -EINVAL;
        }

        printk(KERN_INFO "Valid pid");

        mutex_lock(&process_lock);
        add_pid_to_list(pid);
        mutex_unlock(&process_lock);

        kfree(tem_buf);
        return count;
    }


    //release the list when exit
     void free_the_list(void){
            struct my_list *tmp;
            struct list_head *list_pos;
            struct list_head *q;

            list_for_each_safe(list_pos,q, &(mylist)){
                mutex_lock(&process_lock);
                tmp= list_entry(list_pos, struct my_list, list);
                list_del(list_pos);
                kfree(tmp);
                mutex_unlock(&process_lock);
            }
            return;
        }


    //define the proc_fops structure which is associated with the entry
    static struct file_operations proc_fops = {
        .owner = THIS_MODULE,
        .read = read_callback,
        .write = write_callback,
    };


    // mp1_init - Called when module is loaded
    int __init mp1_init(void)
    {
        #ifdef DEBUG
            printk(KERN_EMERG "MP1 MODULE LOADING\n");
        #endif
        // Insert your code here ...

        // Create user root dir under /proc
        pt_root = proc_mkdir(USER_ROOT_DIR, NULL);
        if (NULL==pt_root)
        {
            printk(KERN_ALERT "Create dir /proc/%s error!\n", USER_ROOT_DIR);
            return -1;
        }
        printk(KERN_INFO "Create dir /proc/%s\n", USER_ROOT_DIR);

        // Create a test entry under USER_ROOT_DIR
        pt_entry1 = proc_create(USER_ENTRY1,0666, pt_root,&proc_fops);
        if (NULL == pt_entry1)
        {
            printk(KERN_ALERT "Create entry %s under /proc/%s error!\n", USER_ENTRY1, USER_ROOT_DIR);
            remove_proc_entry(USER_ROOT_DIR, pt_root);
            return -1;

        }
        printk(KERN_INFO "Create /proc/%s/%s\n", USER_ROOT_DIR, USER_ENTRY1);

        wq=create_workqueue("MP1_WORKQUEUE");
	    if(!wq) {
	        printk(KERN_ALERT "workqueue creation failed, please check");
	        return -1;
	    }

        printk(KERN_INFO "Initialize the timer");
        init_timer(&my_timer);
        setup_timer(&my_timer, my_timer_callback, 0);
        mod_timer(&my_timer, jiffies+msecs_to_jiffies(5000));


        printk(KERN_ALERT "MP1 MODULE LOADED\n");
        return 0;

    }

    // mp1_exit - Called when module is unloaded
    void __exit mp1_exit(void)
    {
	    int ret;
        #ifdef DEBUG
            printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
        #endif
        // Insert your code here ...
        // Remove all entries
        remove_proc_entry(USER_ENTRY1, pt_root);
        remove_proc_entry(USER_ROOT_DIR, NULL);
        ret =  del_timer(&my_timer);
		if(ret) printk(KERN_INFO "timer is in used");
        free_the_list();
        flush_workqueue(wq);
        destroy_workqueue(wq);
        printk(KERN_INFO "All Proc Entry Removed!\n");


        printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
    }
    
    // Register init and exit funtions
    module_init(mp1_init);
    module_exit(mp1_exit);
