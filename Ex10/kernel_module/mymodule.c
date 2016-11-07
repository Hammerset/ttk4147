#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

struct proc_dir_entry *proc;

int procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
if (offset > 0)
{
	return 0;
}
else
{
	return sprintf(buffer, "Hello world\n");
}
}



static __init my_module_init(void) {
	
	printk("Init\n");
	
	proc = create_proc_entry(“myproc”, 0644, NULL);
	proc->read_proc = procfile_read;
	return 0;
}


void __exit my_module_exit(void) {

	/*cleanup code goes here*/
	printk("exit\n");
	remove_proc_entry("myproc", NULL);

}
module_init(my_module_init);
module_exit(my_module_exit);
