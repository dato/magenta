// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "netprotocol.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdint.h>

#include <magenta/netboot.h>

static const char* appname;

int main(int argc, char** argv) {
    appname = argv[0];

    if (argc < 3) {
        fprintf(stderr, "usage: %s <hostname> <command>\n", appname);
        return -1;
    }

    const char* hostname = argv[1];
    if (!strcmp(hostname, "-") || !strcmp(hostname, ":")) {
        hostname = "*";
    }


    char cmd[MAXSIZE];
    size_t cmd_len = 0;
    while (argc > 2) {
        size_t len = strlen(argv[2]);
        if (len > (MAXSIZE - cmd_len - 1)) {
            fprintf(stderr, "%s: command too long\n", appname);
            return -1;
        }
        memcpy(cmd + cmd_len, argv[2], len);
        cmd_len += len;
        cmd[cmd_len++] = ' ';
        argc--;
        argv++;
    }
    cmd[cmd_len - 1] = 0;

    int s;
    if ((s = netboot_open(hostname, NB_SERVER_PORT, NULL)) < 0) {
        if (errno == ETIMEDOUT) {
            fprintf(stderr, "%s: lookup timed out\n", appname);
        }
        return -1;
    }

    msg m;
    m.hdr.magic = NB_MAGIC;
    m.hdr.cookie = 0x11224455;
    m.hdr.cmd = NB_SHELL_CMD;
    m.hdr.arg = 0;
    memcpy(m.data, cmd, cmd_len);

    write(s, &m, sizeof(nbmsg) + cmd_len);
    close(s);

    return 0;
}