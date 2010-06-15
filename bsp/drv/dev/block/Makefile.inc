
SRCS-$(CONFIG_RAMDISK)+=	dev/block/ramdisk.c
SRCS-$(CONFIG_FDD)+=		dev/block/fdd.c
SRCS-$(CONFIG_CROMDISK)+=   dev/block/cromdisk/cromdisk.c \
                            dev/block/cromdisk/puff.c
                            