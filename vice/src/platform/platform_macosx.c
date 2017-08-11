/*
 * platform_macosx.c - Mac OS X Platform detection
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * Based on Code by
 *  http://www.cocoadev.com/index.pl?DeterminingOSVersion
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

/* Tested and confirmed working on:
 * - ppc MacOSX 10.4
 * - x86 MacOSX 10.4
 * - x86 MacOSX 10.5
 * - x86 MacOSX 10.6
 * - x86 MacOSX 10.7
 * - x86 MacOSX 10.8
 * - x86 MacOSX 10.9
 * - x86 MacOSX 10.10
 * - x86 MacOSX 10.11
 */

/* Binary compatibility table:

   running on |       | compiled for ->
              v       | PPC OSX 10.1-10.4 | x86 OSX 10.4-10.6 | x86 OSX 10.7-10.11
   -------------------------------------------------------------------------------
   PPC OSX 10.1-10.4  | yes                | NO               | NO
   x86 OSX 10.4-10.6  | yes (Rosetta)      | yes              | NO
   x86 OSX 10.7-10.11 | NO                 | yes              | yes
 */

#include "vice.h"


#if defined(__APPLE__) && !defined(RHAPSODY_COMPILE) && !defined(DARWIN_COMPILE)

#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ppc__
#include <sys/stat.h>
#endif

#include "CoreServices/CoreServices.h"
#include "platform_macosx.h"
#include "platform.h"
#include "lib.h"

#define MAX_OS_CPU_STR 64
#define MAX_OS_VERSION_STR  32

static char os_cpu_str[MAX_OS_CPU_STR];
static char os_version_str[MAX_OS_VERSION_STR];

#ifdef MAC_OS_X_VERSION_10_12
#define NO_GESTALT
#endif

#ifdef MAC_OS_X_VERSION_10_11
#define NO_GESTALT
#endif

#if !defined(PLATFORM_OS) && defined(MAC_OS_X_VERSION_10_10)
#define NO_GESTALT
#endif

#if !defined(PLATFORM_OS) && defined(MAC_OS_X_VERSION_10_9)
#define NO_GESTALT
#endif

/* this code was taken from: http://www.cocoadev.com/index.pl?DeterminingOSVersion
   and ported to C
*/
static void get_os_version(unsigned *major, unsigned *minor, unsigned *bugFix)
{
#ifndef NO_GESTALT
    OSErr err;
    SInt32 systemVersion, versionMajor, versionMinor, versionBugFix;

    if ((err = Gestalt(gestaltSystemVersion, &systemVersion)) != noErr) {
        goto fail;
    }


    if (systemVersion < 0x1040) {
        if (major) {
            *major = ((systemVersion & 0xF000) >> 12) * 10 +
                     ((systemVersion & 0x0F00) >> 8);
        }
        if (minor) {
            *minor = (systemVersion & 0x00F0) >> 4;
        }
        if (bugFix) {
            *bugFix = (systemVersion & 0x000F);
        }
    } else {
        if ((err = Gestalt(gestaltSystemVersionMajor, &versionMajor)) != noErr) {
            goto fail;
        }
        if ((err = Gestalt(gestaltSystemVersionMinor, &versionMinor)) != noErr) {
            goto fail;
        }
        if ((err = Gestalt(gestaltSystemVersionBugFix, &versionBugFix)) != noErr) {
            goto fail;
        }
        if (major) {
            *major = versionMajor;
        }
        if (minor) {
            *minor = versionMinor;
        }
        if (bugFix) {
            *bugFix = versionBugFix;
        }
    }

    return;
#else
    FILE *fp;
    char num[15];
    const char d[2] = ".";
    char *token;

    fp = popen("/usr/bin/defaults read /System/Library/CoreServices/SystemVersion.plist |/usr/bin/grep ProductVersion |/usr/bin/cut -c23-", "r");

    if (fp == NULL) {
        goto fail;
    }

    while (fgets(num, 15, fp) != NULL) {
        num[(int)strlen(num)-3] = '\0';
    }

    pclose(fp);
    token = strtok(num, d);

    if (token != NULL) {
        *major = strtoul(token, NULL, 0);
        token = strtok(NULL, d);
        if (token != NULL) {
            *minor = strtoul(token, NULL, 0);
            token = strtok(NULL, d);
            if (token != NULL) {
                *bugFix = strtoul(token, NULL, 0);
            }
        }
    }

    return;
#endif

fail:
    if (major) {
        *major = 10;
    }
    if (minor) {
        *minor = 0;
    }
    if (bugFix) {
        *bugFix = 0;
    }
}

char *platform_get_macosx_runtime_os(void)
{
    if (os_version_str[0] == 0) {
        unsigned major, minor, bugFix;
        get_os_version(&major, &minor, &bugFix);
        snprintf(os_version_str, MAX_OS_VERSION_STR, "%u.%u.%u", major, minor, bugFix);
    }
    return os_version_str;
}

static char *get_sysctl_hw_string(int sect)
{
    int mib[2];
    size_t len;
    char *str = NULL;

    /* determine length of string */
    mib[0] = CTL_HW;
    mib[1] = sect;
    if (sysctl(mib, 2, NULL, &len, NULL, 0) != 0) {
        return NULL;
    }

    /* retrieve string */
    str = lib_malloc(len);
    if (sysctl(mib, 2, str, &len, NULL, 0) != 0) {
        lib_free(str);
        return NULL;
    }
    return str;
}

static int get_sysctl_hw_int(int sect)
{
    int mib[2];
    int data;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = sect;
    len = sizeof(data);
    if (sysctl(mib, 2, &data, &len, NULL, 0) == 0) {
        return data;
    } else {
        return -1;
    }
}

static int64_t get_sysctl_hw_int64(int sect)
{
    int mib[2];
    int64_t data;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = sect;
    len = sizeof(data);
    if (sysctl(mib, 2, &data, &len, NULL, 0) == 0) {
        return data;
    } else {
        return -1;
    }
}

char *platform_get_macosx_runtime_cpu(void)
{
#ifdef __ppc__
    struct stat st;
#endif

    if (os_cpu_str[0] == 0) {
        char *machine = get_sysctl_hw_string(HW_MACHINE);
        char *model = get_sysctl_hw_string(HW_MODEL);
        int num_cpus = get_sysctl_hw_int(HW_NCPU);
        int64_t memsize = get_sysctl_hw_int64(HW_MEMSIZE);
        int mem_mb = (int)(memsize >> 20);

#ifdef __ppc__
        if (!stat("/usr/libexec/oah/translate", &st)) {
            snprintf(os_cpu_str, MAX_OS_CPU_STR, "%s [%s] [%d CPUs] [%d MiB RAM] [Rosetta]", machine, model, num_cpus, mem_mb);
        } else
#endif
        snprintf(os_cpu_str, MAX_OS_CPU_STR, "%s [%s] [%d CPUs] [%d MiB RAM]", machine, model, num_cpus, mem_mb);

        if (machine != NULL) {
            lib_free(machine);
        }
        if (model != NULL) {
            lib_free(model);
        }
    }
    return os_cpu_str;
}

#endif
