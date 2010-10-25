# Conversation Locker Makefile
# borrowed template from pidign-facebookchat plugins's Makefile

# Being new to makefiles and in general development, it took me a really long time to figure out exact libraries and paths to include for this project.
# So in the end, I took makefile arguments generated by Linux native build of pidgin and also makefile.mingw arguments used for cross compilation
# and came up with this working Makefile (+referred to other plugin makefiles).
# I am going to keep all that stuff, in the hope that some other fellow programmer might find this makefile template useful for pidgin plugin development.

#Customizable Stuff here
LINUX32_COMPILER = gcc
LINUX64_COMPILER = gcc
WIN32_COMPILER = /usr/bin/i586-mingw32msvc-gcc
CFLAGS = -g -g -O2
CFLAGS64 = -g -g -O2 -m64

PIDGIN_CFLAGS = -I/usr/include/pidgin -I/usr/local/include/pidgin
LIBPURPLE_CFLAGS = -I/usr/include/libpurple -I/usr/local/include/libpurple -DPURPLE_PLUGINS -DENABLE_NLS -DHAVE_ZLIB
GTK_CFLAGS = -D_REENTRANT -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/directfb -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include

WIN32_DEV_DIR = /home/enli/development/sources/pidgin/win32/win32-dev
WIN32_PIDGIN_DIR = /home/enli/development/sources/pidgin/win32/pidgin-2.6.6
WIN32_INCLUDE_PATHS = 	-I/usr/i586-mingw32msvc/include -I$(WIN32_DEV_DIR)/gtk_2_0/include \
					-I$(WIN32_DEV_DIR)/gtk_2_0/include/gtk-2.0 \
					-I$(WIN32_DEV_DIR)/gtk_2_0/include/glib-2.0 \
					-I$(WIN32_DEV_DIR)/gtk_2_0/include/pango-1.0 \
					-I$(WIN32_DEV_DIR)/gtk_2_0/include/atk-1.0 \
					-I$(WIN32_DEV_DIR)/gtk_2_0/include/cairo \
					-I$(WIN32_DEV_DIR)/gtk_2_0/lib/glib-2.0/include \
					-I$(WIN32_DEV_DIR)/gtk_2_0/lib/gtk-2.0/include \
					-I$(WIN32_PIDGIN_DIR)/libpurple \
					-I$(WIN32_PIDGIN_DIR)/libpurple/win32 \
					-I$(WIN32_PIDGIN_DIR)/pidgin \
					-I$(WIN32_PIDGIN_DIR)/pidgin/win32 \
					-I$(WIN32_PIDGIN_DIR)

WIN32_LIB_PATHS = 	-L$(WIN32_DEV_DIR)/gtk_2_0/lib \
				-L$(WIN32_PIDGIN_DIR)/libpurple \
				-L$(WIN32_PIDGIN_DIR)/pidgin

WIN32_LIBS = 	-lgtk-win32-2.0 \
			-lglib-2.0 \
			-lgdk-win32-2.0 \
			-lgobject-2.0 \
			-lgmodule-2.0 \
			-lgdk_pixbuf-2.0 \
			-lpango-1.0 \
			-lintl \
			-lws2_32 \
			-lpurple \
			-lpidgin

WIN32_ARGS = -O2 -Wall -Waggregate-return -Wcast-align -Wdeclaration-after-statement -Werror-implicit-function-declaration -Wextra -Wno-sign-compare -Wno-unused-parameter -Winit-self -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wundef -pipe -mno-cygwin -mms-bitfields -g -DHAVE_CYRUS_SASL -DHAVE_CONFIG_H -DENABLE_NLS -DHAVE_ZLIB
#-DPURPLE_PLUGINS

CONVLOCKER_SOURCES = 	conversationlocker.c \
					conversationlocker-icons.h

CONVLOCKER_MAIN = conversationlocker.c
CONVLOCKER_VERSION = 1.1

#Standard stuff here
.PHONY:	all clean install installers sourcepackage

all:	conversationlocker.so conversationlocker64.so conversationlocker.dll conversationlocker.zip conversationlocker.exe sourcepackage linux-bin \
		conversationlocker.deb conversationlocker64.deb

install:
	cp conversationlocker.so /usr/lib/pidgin/

installers :	conversationlocker.exe linux-bin

uninstall:
	rm -f $(DESTDIR)/usr/lib/pidgin/conversationlocker.so

clean:
	@rm -f conversationlocker.so
	@rm -f conversationlocker64.so
	@rm -f conversationlocker.dll
	@rm -f conversationlocker.o
	@rm -f *.exe
	@rm -f *.zip
	@rm -f pidgin-conversation-locker-$(CONVLOCKER_VERSION)-src.tar.bz2
	@rm -f pidgin-conversation-locker-$(CONVLOCKER_VERSION)-linux-i386.tar.bz2
	@rm -f *.deb
	@echo "Cleanup performed!"

conversationlocker.so:		$(CONVLOCKER_SOURCES)
	$(LINUX32_COMPILER) $(CFLAGS) $(LIBPURPLE_CFLAGS) $(PIDGIN_CFLAGS) $(GTK_CFLAGS) -Wall -pthread $(GLIB_CFLAGS) -I. -pipe conversationlocker.c -o $@ -shared -fPIC -DPIC

conversationlocker64.so:		$(CONVLOCKER_SOURCES)
	$(LINUX64_COMPILER) $(CFLAGS64) $(LIBPURPLE_CFLAGS) $(PIDGIN_CFLAGS) $(GTK_CFLAGS) -Wall -pthread $(GLIB_CFLAGS) -I. -pipe conversationlocker.c -o $@ -shared -fPIC -DPIC

conversationlocker.dll:	$(CONVLOCKER_SOURCES)
	${WIN32_COMPILER} $(WIN32_ARGS) ${WIN32_INCLUDE_PATHS} -o conversationlocker.o -c conversationlocker.c
	${WIN32_COMPILER} -shared conversationlocker.o ${WIN32_LIB_PATHS} $(WIN32_LIBS) -Wl,--enable-auto-image-base -o $@
#-Wall -I. -g -O2 -pipe conversationlocker.c -o conversationlocker.dll -shared -mno-cygwin -DSKYPENET -Wl,--strip-all
	upx conversationlocker.dll

conversationlocker.zip : conversationlocker.dll
	@zip conversationlocker_$(CONVLOCKER_VERSION)_portable.zip conversationlocker.dll > /dev/null

conversationlocker.deb :	conversationlocker.so
	@echo "\nDont forget to update version number"
	@mkdir -p debdir/DEBIAN
	@mkdir -p debdir/usr/lib/pidgin/
	@chmod -x conversationlocker.so
	@cp conversationlocker.so debdir/usr/lib/pidgin/
	@cp packaging/deb/DEBIAN/control debdir/DEBIAN/control
	@dpkg-deb --build debdir pidgin-conversation-locker_$(CONVLOCKER_VERSION)_i386.deb
	@rm -r debdir
	@echo "\ndebi386 package created."

conversationlocker64.deb :	conversationlocker64.so
	@echo "\nDont forget to update version number"
	@mkdir -p debdir/DEBIAN
	@mkdir -p debdir/usr/lib/pidgin/
	@chmod -x conversationlocker64.so
	@cp conversationlocker64.so debdir/usr/lib/pidgin/
	@cp packaging/deb/DEBIAN/control64 debdir/DEBIAN/control
	@dpkg-deb --build debdir pidgin-conversation-locker_$(CONVLOCKER_VERSION)_amd64.deb
	@rm -r debdir
	@echo "\ndeb64 package created."

conversationlocker.exe:	conversationlocker.dll
	@cp packaging/windows/conversationlocker.nsi .
	@makensis conversationlocker.nsi > /dev/null
	@rm conversationlocker.nsi
	@echo "\n >>> Did you update the version? <<<"
	@echo "\n.exe installer generated."

sourcepackage :	$(CONVLOCKER_SOURCES) img-src packaging AUTHORS ChangeLog COPYING Makefile Makefile.mingw NEWS README VERSION
	@mkdir -p pidgin-conversation-locker
	@cp -r $^ pidgin-conversation-locker
	@tar --bzip2 -cf pidgin-conversation-locker-$(CONVLOCKER_VERSION)-src.tar.bz2 pidgin-conversation-locker
	@rm -rf pidgin-conversation-locker
	@echo "\nSource package generated."

linux-bin :	packaging/linux/install.sh conversationlocker.so
	@echo "Don't forget to update version number in /packaging/linux/README"
	@mkdir -p pidgin-conversation-locker-$(CONVLOCKER_VERSION)
	@cp packaging/linux/install.sh packaging/linux/README conversationlocker.so pidgin-conversation-locker-$(CONVLOCKER_VERSION)
	@tar --bzip2 -cf pidgin-conversation-locker-$(CONVLOCKER_VERSION)-linux-i386.tar.bz2 pidgin-conversation-locker-$(CONVLOCKER_VERSION)
	@rm -r pidgin-conversation-locker-$(CONVLOCKER_VERSION)
	@echo "\nLinux binary installer generated."

