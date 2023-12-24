obj-m += netlink.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -Wall -Werror -Wextra user.c -o user
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf user