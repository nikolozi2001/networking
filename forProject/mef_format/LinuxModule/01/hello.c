#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
// მოდულის მეტა მონაცემები
MODULE_LICENSE("GPL");
MODULE_AUTHOR("გიორგი მოდაბაძე");
MODULE_DESCRIPTION("მინიმალური Linux-ის ბირთვის მოდული");
MODULE_VERSION("0.1");
// მოდულის ჩატვირთვის დროს გამოიძახება ეს ფუნქცია
static int __init hello_init(void)
{
	printk(KERN_INFO "hello: მოდული ჩაიტვირთა\n");
	return 0;
}
// მოდულის ამოტვირთვის დროს გამოიძახება ეს ფუნქცია
static void __exit hello_exit(void)
{
	printk(KERN_INFO "hello: მოდული ამოიტვირთა\n");
}
// მოდულის ჩატვირთვის ფუნქციის რეგისტრაცია
module_init(hello_init);
// მოდულის ამოტვირთვის ფუნქციის რეგისტრაცია
module_exit(hello_exit);
