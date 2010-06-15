# Rules to create OS image

include $(SRCDIR)/mk/own.mk

TARGET:=	$(SRCDIR)/prexos
LOADER:=	$(SRCDIR)/bsp/boot/bootldr
DRIVER:=	$(SRCDIR)/bsp/drv/drv.ko
KERNEL:=	$(SRCDIR)/sys/prex

include $(SRCDIR)/conf/etc/tasks.mk
include $(SRCDIR)/conf/etc/files.mk
include $(SRCDIR)/mk/common.mk
-include $(SRCDIR)/bsp/boot/$(ARCH)/$(PLATFORM)/Makefile.sysgen

$(SRCDIR)/tools/mkcromdisk: $(SRCDIR)/tools/mkcromdisk.c
	cc -o $@ $< -lz
	
ifdef CONFIG_CROMDISK
$(TARGET): $(SRCDIR)/tools/mkcromdisk
else
$(TARGET):
endif
	$(call echo-file,PACK   ,$@)
ifdef FILES
	$(AR) rcS bootdisk.a $(FILES)
ifdef CONFIG_CROMDISK
	$(SRCDIR)/tools/mkcromdisk bootdisk.a bootdisk.crom
	$(MV) bootdisk.crom bootdisk.a
endif
	$(AR) rcS tmp.a $(KERNEL) $(DRIVER) $(TASKS) bootdisk.a
	$(RM) bootdisk.a
else
	$(AR) rcS tmp.a $(KERNEL) $(DRIVER) $(TASKS)
endif
	$(CAT) $(LOADER) tmp.a > $@
	$(RM) tmp.a
	$(call sysgen)
	@echo 'Done.'
