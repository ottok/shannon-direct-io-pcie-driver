# -*- makefile -*-

SHANNON_DRIVER_NAME ?= shannon
SHANNON_EMU_NAME ?= shannon_nand_emu

$(SHANNON_DRIVER_NAME)-y := shannon_main.o
$(SHANNON_DRIVER_NAME)-y += shannon_ftl.o
$(SHANNON_DRIVER_NAME)-y += shannon_prefetch.o
$(SHANNON_DRIVER_NAME)-y += shannon_epilog.o
$(SHANNON_DRIVER_NAME)-y += shannon_err_handler.o
$(SHANNON_DRIVER_NAME)-y += shannon_ns.o
$(SHANNON_DRIVER_NAME)-y += shannon_ioctl.o
$(SHANNON_DRIVER_NAME)-y += shannon_scsi_cmd.o
$(SHANNON_DRIVER_NAME)-y += shannon_kcore.o
$(SHANNON_DRIVER_NAME)-y += shannon_time.o
$(SHANNON_DRIVER_NAME)-y += shannon_workqueue.o
$(SHANNON_DRIVER_NAME)-y += shannon_waitqueue.o
$(SHANNON_DRIVER_NAME)-y += shannon_file.o
$(SHANNON_DRIVER_NAME)-y += shannon_pci.o
$(SHANNON_DRIVER_NAME)-y += shannon_scatter.o
$(SHANNON_DRIVER_NAME)-y += shannon_device.o
$(SHANNON_DRIVER_NAME)-y += shannon_block.o
$(SHANNON_DRIVER_NAME)-y += shannon_sched.o
$(SHANNON_DRIVER_NAME)-y += shannon_module_init.o
$(SHANNON_DRIVER_NAME)-y += shannon_sysfs.o
$(SHANNON_DRIVER_NAME)-y += shannon_scsi.o


ifneq ($(CONFIG_SHANNON_EMU),)
$(SHANNON_EMU_NAME)-y := shannon_emu.o
$(SHANNON_EMU_NAME)-y += shannon_cdev.o
obj-$(CONFIG_SHANNON_EMU) += $(SHANNON_EMU_NAME).o
else
$(SHANNON_DRIVER_NAME)-y += shannon_cdev.o
$(SHANNON_DRIVER_NAME)-y += shannon_dma.o
endif

obj-$(CONFIG_BLK_DEV_SHANNON) += $(SHANNON_DRIVER_NAME).o

INSTALL_MOD_DIR ?= extra/shannon
