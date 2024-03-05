#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x38f431f2, "module_layout" },
	{ 0x48a84465, "cdev_del" },
	{ 0x695bf5e9, "hrtimer_cancel" },
	{ 0xfe990052, "gpio_free" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0x42160169, "flush_workqueue" },
	{ 0x870d5a1c, "__init_swait_queue_head" },
	{ 0x49cd25ed, "alloc_workqueue" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x4bcfdc97, "device_destroy" },
	{ 0xab51152b, "gpiod_to_irq" },
	{ 0xa362bf8f, "hrtimer_init" },
	{ 0x5bbe49f4, "__init_waitqueue_head" },
	{ 0xe023ffe0, "cdev_add" },
	{ 0x285cb869, "cdev_init" },
	{ 0xec6c724b, "class_destroy" },
	{ 0x761d47c4, "device_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x11ddfbc6, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x7fc95440, "gpiod_direction_input" },
	{ 0x57c53bd2, "gpiod_export" },
	{ 0x2030d859, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x97255bdf, "strlen" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x49970de8, "finish_wait" },
	{ 0x647af474, "prepare_to_wait_event" },
	{ 0x1000e51, "schedule" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x800473f, "__cond_resched" },
	{ 0x3376ec94, "gpiod_get_raw_value" },
	{ 0x3dcf1ffa, "__wake_up" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x5f754e5a, "memset" },
	{ 0xc3055d20, "usleep_range_state" },
	{ 0xb2d48a2e, "queue_work_on" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x92997ed8, "_printk" },
	{ 0xf64bf255, "wait_for_completion" },
	{ 0xc37335b0, "complete" },
	{ 0x4fef3ef4, "completion_done" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e1e487f, "gpiod_set_raw_value" },
	{ 0xeaf06972, "gpio_to_desc" },
	{ 0x5cc2a511, "hrtimer_forward" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xec523f88, "hrtimer_start_range_ns" },
	{ 0xa12d6e96, "kmem_cache_alloc_trace" },
	{ 0x6d0b5a7f, "kmalloc_caches" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B9C2BF32F6E8F36AC75AC56");
