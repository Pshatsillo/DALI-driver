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
#include <math.h>

#define DALI_drv "/dev/dali"

char receiveBuffer[7];
int fd;
static u_int8_t seq = 0;
char bitToStringArr[10];

void send_command(int fd, char *command, u_int8_t sequence)
{
    char buffer[7];
    sprintf(buffer, "%02x%4s", sequence, command);
    write(fd, buffer, 6);
    seq += 1;
    if (seq == 0xff)
        seq = 0;
}

int waitForAnswer()
{
    struct pollfd fds;
    int ret;
    int res = 0;
    fds.fd = fd;
    fds.events = POLLIN;

    ret = poll(&fds, 1, 34);
    switch (ret)
    {
    case -1:
        break;
    case 0:
        break;
    default:
        read(fd, receiveBuffer, 7);
        res = 1;
        break;
    }
    return res;
}
int count_bits(unsigned x)
{
    int bits = 0;
    while (x)
    {
        if (x & 1U)
            bits++;
        x >>= 1;
    }
    return bits;
}
int int_bits(void)
{
    return count_bits(~0U);
}

void print_nbits(int x, int n)
{
    memset(&bitToStringArr[0], 0, 10);
    int i = int_bits();
    int bit;
    i = (n < i) ? n - 1 : i - 1;
    for (; i >= 0; i--)
    {
        if ((x >> i) & 1U)
        {
            bit = (n - 1) - i;
            bitToStringArr[(n - 1) - i] = '1';
        }
        else
        {
            bit = (n - 1) - i;
            bitToStringArr[(n - 1) - i] = '0';
        }
    }
}

long getCommand(int shortAddr, int cmd)
{
    char buffer[7];
    print_nbits(shortAddr, 6);
    char devID[6];
    memcpy(devID, bitToStringArr, 6);
    print_nbits(cmd, 8);
    char command[8];
    memcpy(command, bitToStringArr, 8);
    char *longAddr = NULL;
    asprintf(&longAddr, "0%c%c%c%c%c%c1%c%c%c%c%c%c%c%c", devID[0], devID[1], devID[2], devID[3], devID[4], devID[5], command[0], command[1], command[2], command[3], command[4], command[5], command[6], command[7]);
    // printf("Bytestring: %s\n", longAddr);
    long hexval = strtol(longAddr, NULL, 2);
    return hexval;
}
long getGrpCommand(int shortAddr, int cmd)
{
    char buffer[7];
    print_nbits(shortAddr, 6);
    char devID[6];
    memcpy(devID, bitToStringArr, 6);
    print_nbits(cmd, 8);
    char command[8];
    memcpy(command, bitToStringArr, 8);
    char *longAddr = NULL;
    asprintf(&longAddr, "1%c%c%c%c%c%c1%c%c%c%c%c%c%c%c", devID[0], devID[1], devID[2], devID[3], devID[4], devID[5], command[0], command[1], command[2], command[3], command[4], command[5], command[6], command[7]);
    // printf("Bytestring: %s\n", longAddr);
    long hexval = strtol(longAddr, NULL, 2);
    return hexval;
}

long setGrpDim(int shortAddr, int cmd)
{
    char buffer[7];
    print_nbits(shortAddr, 6);
    char devID[6];
    memcpy(devID, bitToStringArr, 6);
    print_nbits(cmd, 8);
    char command[8];
    memcpy(command, bitToStringArr, 8);
    char *longAddr = NULL;
    asprintf(&longAddr, "1%c%c%c%c%c%c0%c%c%c%c%c%c%c%c", devID[0], devID[1], devID[2], devID[3], devID[4], devID[5], command[0], command[1], command[2], command[3], command[4], command[5], command[6], command[7]);
    // printf("Bytestring: %s\n", longAddr);
    long hexval = strtol(longAddr, NULL, 2);
    return hexval;
}

long setDevDim(int shortAddr, int cmd)
{
    char buffer[7];
    print_nbits(shortAddr, 6);
    char devID[6];
    memcpy(devID, bitToStringArr, 6);
    print_nbits(cmd, 8);
    char command[8];
    memcpy(command, bitToStringArr, 8);
    char *longAddr = NULL;
    asprintf(&longAddr, "0%c%c%c%c%c%c0%c%c%c%c%c%c%c%c", devID[0], devID[1], devID[2], devID[3], devID[4], devID[5], command[0], command[1], command[2], command[3], command[4], command[5], command[6], command[7]);
    // printf("Bytestring: %s\n", longAddr);
    long hexval = strtol(longAddr, NULL, 2);
    return hexval;
}

int main(int argc, char *argv[])
{
    char *p;
    char buffer[7];
    int len = 7;
    int ret;
    struct pollfd fds;
    int t;

    fd = open(DALI_drv, O_RDWR);

    if (fd < 0)
    {
        perror("Cannot open device \t");
        printf(" fd = %d \n", fd);
        return 0;
    }

    if (argc >= 4)
    {
        if (strcmp("device", argv[1]) == 0)
        {
            // if (strcmp("on", argv[3]) == 0)
            // {
            // }
            if (strcmp("off", argv[3]) == 0)
            {
                long arg2 = strtol(argv[2], &p, 10);
                long hexval = getCommand(arg2, 0);
                sprintf(buffer, "%04x\n", hexval);
                send_command(fd, &buffer[0], arg2);
            }
            if (strcmp("dim", argv[3]) == 0)
            {
                long arg2 = strtol(argv[2], &p, 10);
                long arg4 = strtod(argv[4], &p);
                if (arg4 == 0 || arg4 <= 100)
                {
                    long hexval = setDevDim(arg2, round(arg4 * 2.54));
                    sprintf(buffer, "%04x\n", hexval);
                    send_command(fd, &buffer[0], arg2);
                }
            }
            if (strcmp("dimraw", argv[3]) == 0)
            {
                long arg2 = strtol(argv[2], &p, 10);
                long arg4 = strtol(argv[4], &p, 10);
                long hexval = setDevDim(arg2, arg4);
                sprintf(buffer, "%04x\n", hexval);
                send_command(fd, &buffer[0], arg2);
            }
            if (strcmp("status", argv[3]) == 0)
            {
                long arg3 = strtol(argv[2], &p, 10);
                // printf("Device ID: %d\n", arg3);
                long hexval = getCommand(arg3, 160);
                sprintf(buffer, "%04x\n", hexval);
                // printf("Hex: %04x\n", hexval);
                send_command(fd, &buffer[0], arg3);
                int result = waitForAnswer();
                if (result == 1)
                {
                    char *hex = malloc(2);
                    sprintf(hex, "%c%c", receiveBuffer[4], receiveBuffer[5]);
                    int number = (int)strtol(hex, NULL, 16);
                    printf("%d", number);
                }
            }
        }
        if (strcmp("group", argv[1]) == 0)
        {
            // if (strcmp("on", argv[3]) == 0)
            // {
            // }
            if (strcmp("off", argv[3]) == 0)
            {
                long arg2 = strtol(argv[2], &p, 10);
                long hexval = getGrpCommand(arg2, 0);
                sprintf(buffer, "%04x\n", hexval);
                send_command(fd, &buffer[0],arg2 + 65);
            }
            if (strcmp("dim", argv[3]) == 0)
            {
                long arg2 = strtol(argv[2], &p, 10);
                long arg4 = strtod(argv[4], &p);
                if (arg4 == 0 || arg4 <= 100)
                {
                    long hexval = setGrpDim(arg2, round(arg4 * 2.54));
                    sprintf(buffer, "%04x\n", hexval);
                    send_command(fd, &buffer[0], arg2 + 65);
                }
            }
            if (strcmp("dimraw", argv[3]) == 0)
            {
                long arg2 = strtol(argv[2], &p, 10);
                long arg4 = strtol(argv[4], &p, 10);
                if (arg4 == 0 || arg4 <= 254)
                {
                    long hexval = setGrpDim(arg2, arg4);
                    sprintf(buffer, "%04x\n", hexval);
                    send_command(fd, &buffer[0], arg2 + 65);
                }
            }
            if (strcmp("status", argv[3]) == 0)
            {
                long arg3 = strtol(argv[2], &p, 10);
                // printf("Group ID: %d\n", arg3);
                long hexval = getGrpCommand(arg3, 160);
                sprintf(buffer, "%04x\n", hexval);
                // printf("Hex: %04x\n", hexval);
                send_command(fd, &buffer[0], arg3 + 65);
                int result = waitForAnswer();
                if (result == 1)
                {
                    char *hex = malloc(2);
                    sprintf(hex, "%c%c", receiveBuffer[4], receiveBuffer[5]);
                    int number = (int)strtol(hex, NULL, 16);
                    printf("%d", number);
                }
            }
        }
    }
    return 0;
}