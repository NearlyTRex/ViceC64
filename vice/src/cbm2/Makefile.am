SUBDIRS = cart

AM_CPPFLAGS = \
	@VICE_CPPFLAGS@ \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/cbm2/cart \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/crtc \
	-I$(top_srcdir)/src/vicii \
	-I$(top_srcdir)/src/video \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/printerdrv \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/samplerdrv \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/tape \
	-I$(top_srcdir)/src/rs232drv \
	-I$(top_srcdir)/src/tapeport \
	-I$(top_srcdir)/src/datasette \
	-I$(top_srcdir)/src/core/rtc

AM_CFLAGS = @VICE_CFLAGS@

AM_CXXFLAGS = @VICE_CXXFLAGS@

AM_LDFLAGS = @VICE_LDFLAGS@


noinst_LIBRARIES = libcbm2.a libcbm2stubs.a libcbm5x0.a libcbm5x0stubs.a

libcbm2stubs_a_SOURCES = \
	cbm2-stubs.c

libcbm2_a_SOURCES = \
	cbm2-cmdline-options.c \
	cbm2-cmdline-options.h \
	cbm2-resources.c \
	cbm2-resources.h \
	cbm2-snapshot.c \
	cbm2-snapshot.h \
	cbm2.c \
	cbm2.h \
	cbm2acia.h \
	cbm2acia1.c \
	cbm2bus.c \
	cbm2cia.h \
	cbm2cia1.c \
	cbm2cpu.c \
	cbm2datasette.c \
	cbm2drive.c \
	cbm2export.c \
	cbm2iec.c \
	cbm2iec.h \
	cbm2io.c \
	cbm2mem.c \
	cbm2mem.h \
	cbm2memsnapshot.c \
	cbm2memsnapshot.h \
	cbm2model.c \
	cbm2model.h \
	cbm2printer.c \
	cbm2rom.c \
	cbm2rom.h \
	cbm2romset.c \
	cbm2sound.c \
	cbm2tpi.h \
	cbm2tpi1.c \
	cbm2tpi2.c \
	cbm2video.c

libcbm5x0stubs_a_SOURCES = \
	cbm5x0-stubs.c

libcbm5x0_a_SOURCES = \
	cbm2-cmdline-options.c \
	cbm2-cmdline-options.h \
	cbm5x0-resources.c \
	cbm2-resources.h \
	cbm5x0-snapshot.c \
	cbm2-snapshot.h \
	cbm5x0.c \
	cbm2.h \
	cbm2acia.h \
	cbm2acia1.c \
	cbm2bus.c \
	cbm2cia.h \
	cbm5x0cia1.c \
	cbm2cpu.c \
	cbm2datasette.c \
	cbm2drive.c \
	cbm2export.c \
	cbm2iec.c \
	cbm2iec.h \
	cbm2io.c \
	cbm5x0mem.c \
	cbm2mem.h \
	cbm2memsnapshot.c \
	cbm2memsnapshot.h \
	cbm2model.c \
	cbm2model.h \
	cbm5x0printer.c \
	cbm5x0rom.c \
	cbm2rom.h \
	cbm2romset.c \
	cbm2sound.c \
	cbm2tpi.h \
	cbm2tpi1.c \
	cbm2tpi2.c \
	cbm5x0video.c

EXTRA_DIST = \
	cbm2-common-resources.c

.PHONY: libcbm2cart

libcbm2cart:
	(cd cart; $(MAKE))
