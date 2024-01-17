#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "parson.h"

#define DALI_drv "/dev/dali"

static u_int8_t seq = 0;
static u_int8_t ShortAddr = 0;
FILE *fptr;
static bool hasFile = false;

struct stat filestatus;
char *filename = "config.json";
int file_size;
char *file_contents;
JSON_Value *user_data;
int fd;

void send_command(int fd, char *command)
{
    char buffer[7];
    sprintf(buffer, "%02x%4s", seq, command);
    write(fd, buffer, 6);
    seq += 1;
    if (seq == 0xff)
        seq = 0;
}

void init_search(int fd)
{
    send_command(fd, "2000");
    usleep(90000);

    send_command(fd, "2000");
    usleep(90000);

    send_command(fd, "A500");
    usleep(90000);

    send_command(fd, "A500");
    usleep(90000);
}

int main(int argc, char *argv[])
{
    loadConfig();
    struct timeval timeout;
    char buffer[7];
    int len = 7;
    int ret;
    u_int32_t t;
    fd = open(DALI_drv, O_RDWR);

    if (fd < 0)
    {
        perror("Cannot open device \t");
        printf(" fd = %d \n", fd);
        return 0;
    }

    init_search(fd);
}