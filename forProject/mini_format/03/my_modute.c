// 05_init_and_exit_macros.c
#include <linux/init.h>     // მაკროსებისთვის
#include <linux/module.h>
#include <linux/kernel.h>
// მოდულის ინიციალიზაციის ფუნქცია
static int __init my_module_init(void) {
    printk(KERN_INFO "Hello world!\n");
    return 0;
}
// მოდულის დეინიციალიზაციის ფუნქცია
static void __exit my_module_exit(void) {
    printk(KERN_INFO "Goodbye world!\n");
}
/*
აქ module_init მაკრისის დახმარებით ვუთითებს,
რომელია მოდულის ინიციალიზაციის ფუნქცია
*/
module_init(my_module_init);
/*
აქ module_exit მაკრისის დახმარებით ვუთითებს,
რომელია მოდულის ინიციალიზაციის ფუნქცია
*/
module_exit(my_module_exit);
//
MODULE_LICENSE("GPL");
