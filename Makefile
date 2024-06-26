KERN_SRC:= /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)
obj-m := DALI_driver.o
all:
	make -C $(KERN_SRC) M=$(PWD) modules
clean:
	make -C $(KERN_SRC) M=$(PWD) clean

util:
	gcc DALI_app.c -o dali_send
	gcc DALI_init.c parson.c -o dali_init
	gcc dali_config.c parson.c -o dali_config
	gcc dali_cmd.c -o dali_cmd -lm
