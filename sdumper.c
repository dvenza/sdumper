#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <zlib.h>
#include <time.h>

#define SOCKET_PATH "/tmp/sdumper.sock"

struct gzFile_s* out_fp = NULL;

void gen_filename(char* name, size_t len) {
    struct timeval tv;
    struct tm *now_tm;

    gettimeofday(&tv, NULL);
    now_tm = gmtime(&tv.tv_sec);
    snprintf(name, len, "dump-%d-%02d-%02d-%02d%02d.json.gz", now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min);
}

int today() {
    struct timeval tv;
    struct tm *now_tm;

    gettimeofday(&tv, NULL);
    now_tm = gmtime(&tv.tv_sec);
    return now_tm->tm_mday;
}

void sig_handler(int signal_num) {
    if (out_fp != NULL) {
        gzclose(out_fp);
        out_fp = NULL;
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    struct sockaddr_un addr;
    int fd, ret, day, count = 0;
    char buf[131072];
    char fname[256];

    fd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        exit (1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    unlink(SOCKET_PATH);
    ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret) {
        perror("bind");
        exit(1);
    }

    listen(fd, 5);

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGQUIT, sig_handler);

    gen_filename(fname, 256);
    day = today();
    out_fp = gzopen(fname, "wb");
    if (out_fp == NULL) {
        perror("fopen");
        exit(1);
    }

    while (1) {
        int ret, ret2;

        ret = recv(fd, buf, sizeof(buf), 0);
        if (ret == -1) {
            perror("read");
            continue;
        }
        
        ret2 = gzwrite(out_fp, buf, ret);
        printf("read %u bytes, wrote %u bytes\n", ret, ret2);
        count++;
        if (count > 100) {
            count = 0;
            if (day != today()) {
                printf("day changed, rotating file\n");
                gzclose(out_fp);
                gen_filename(fname, 256);
                day = today();
                out_fp = gzopen(fname, "wb");
            }
        }
    }
}
