# KERNELRELEASE is set by the kernel build system.  This is used
# as a test to know if the build is being driven by the kernel.
ifneq ($(KERNELRELEASE),)
# Kernel build

# Older kernel scripts/Makefile.build only process Makefile so
# we include the Kbuild file here.  Newer kernel scripts/Makefile.build
# include Kbuild directly and never process Makefile (this file).
include $(SHANNON_DRIVER_DIR)/Kbuild

else

KERNELVER ?= $(shell uname -r)
KERNEL_SRC = /lib/modules/$(KERNELVER)/build

# Uncomment this to build EMU module directly in this dir.
#SHANNON_FLAGS += -DCONFIG_SHANNON_EMU_MODULE

.PHONY: all clean modules_clean modules modules_install uninstall
all: modules

clean modules_clean:
	$(MAKE) \
	    -C $(KERNEL_SRC) \
	    SHANNON_DRIVER_DIR=$(shell pwd) \
	    SUBDIRS=$(shell pwd) \
	    clean

debug:
	$(MAKE) \
	    -C $(KERNEL_SRC) \
	    SHANNON_DRIVER_DIR=$(shell pwd) \
	    SUBDIRS=$(shell pwd) \
	    CONFIG_BLK_DEV_SHANNON=m \
	    CONFIG_SHANNON_EMU= \
	    EXTRA_CFLAGS="$(SHANNON_FLAGS)" \
	    INSTALL_MOD_PATH=$(INSTALL_ROOT) \
	    modules

modules modules_install:
	$(MAKE) \
	    -C $(KERNEL_SRC) \
	    SHANNON_DRIVER_DIR=$(shell pwd) \
	    SUBDIRS=$(shell pwd) \
	    CONFIG_BLK_DEV_SHANNON=m \
	    CONFIG_SHANNON_EMU= \
	    EXTRA_CFLAGS="$(SHANNON_FLAGS) -DSHANNON_RELEASE" \
	    INSTALL_MOD_PATH=$(INSTALL_ROOT) \
	    $@
uninstall:
	@echo "DELETE /lib/modules/$(KERNELVER)/extra/shannon.ko"
	@rm -rf /lib/modules/$(KERNELVER)/extra/shannon.ko
	@echo "DEPMOD $(KERNELVER)"
	@/sbin/depmod $(KERNELVER)

endif
