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
#include <limits.h>

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
char bitToStringArr[10];
//char groupAsBit[4];

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

int loadConfig()
{
    if (stat(filename, &filestatus) != 0)
    {
        fptr = fopen("config.json", "w");
        JSON_Value *root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        char *serialized_string = NULL;
        json_object_set_value(root_object, "devices", json_parse_string("[]"));

        JSON_Value *device_value = json_value_init_object();
        JSON_Object *device_object = json_value_get_object(device_value);
        for (u_int8_t i = 0; i <= 15; i++)
        {
            char *name;
            asprintf(&name, "%i", i);
            json_object_set_value(device_object, name, json_parse_string("[{\"Device\":99}]"));
            free(name);
        }

        json_object_set_value(root_object, "group", device_value);
        serialized_string = json_serialize_to_string_pretty(root_value);
        fprintf(fptr, serialized_string);
        fclose(fptr);
        free(file_contents);
        stat(filename, &filestatus);
    }
    else
    {
        hasFile = true;
    }
    file_size = filestatus.st_size;
    file_contents = (char *)malloc(filestatus.st_size);
    if (file_contents == NULL)
    {
        fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fptr = fopen(filename, "rt");
    if (fread(file_contents, file_size, 1, fptr) != 1)
    {
        fprintf(stderr, "Unable to read content of %s\n", filename);
        fclose(fptr);
        free(file_contents);
        return 1;
    }
    fclose(fptr);
    user_data = json_parse_string(file_contents);
    free(file_contents);
}

void registerGroups()
{
    printf("Adding groups and devices from config.json \n");
    char buffer[7];
    JSON_Object *deviceJson = json_object(user_data);
    JSON_Object *grpArr = json_object_get_object(deviceJson, "group");
    size_t i;
    for (i = 0; i < json_object_get_count(grpArr); i++)
    {
        char *name;
        asprintf(&name, "%i", i);
        JSON_Value *grpName = json_object_get_value(grpArr, name);
        JSON_Array *gArr = json_array(grpName);
        free(name);
        printf("Group %d id: ", i);
        for (size_t j = 0; j < json_array_get_count(gArr); j++)
        {
            JSON_Object *groupDev = json_array_get_object(gArr, j);
            u_int8_t ShortAddr = json_object_get_number(groupDev, "Device");
            if (ShortAddr <= 64)
            {
                print_nbits(ShortAddr, 6);
                char devID[7]; 
                strcpy(devID, bitToStringArr);
                print_nbits(i, 4);
                char GrpAddr[5];
                strcpy(GrpAddr, bitToStringArr);
                char *longAddr = NULL;
                asprintf(&longAddr, "0%c%c%c%c%c%c10110%c%c%c%c", devID[0],devID[1],devID[2],devID[3],devID[4],devID[5], GrpAddr[0],GrpAddr[1],GrpAddr[2],GrpAddr[3]);
                long hexval = strtol(longAddr, NULL, 2);
                printf("Bytestring: %s ", longAddr);
                printf("hex: %04x ", hexval);
                sprintf(buffer, "%04x", hexval);
                send_command(fd, &buffer[0]);
                usleep(90000);
                send_command(fd, &buffer[0]);
                usleep(90000);
            }
        }
        printf("\n");
    }
}

void removeFromGroup(int group, int shortAddr)
{
    char buffer[7];
    printf("Removing from group %d device %d\n", group, shortAddr);
    print_nbits(shortAddr, 6);
    char devID[7]; 
    strcpy(devID, bitToStringArr);
    //puts(devID);
    print_nbits(group, 4);
    char GrpAddr[5];
    strcpy(GrpAddr, bitToStringArr);
    //puts(GrpAddr);
    char *longAddr = NULL;
    asprintf(&longAddr, "0%c%c%c%c%c%c10111%c%c%c%c", devID[0],devID[1],devID[2],devID[3],devID[4],devID[5], GrpAddr[0],GrpAddr[1],GrpAddr[2],GrpAddr[3]);
    printf("Bytestring: %s\n", longAddr);
    long hexval = strtol(longAddr, NULL, 2);
    sprintf(buffer, "%04x\n", hexval);
    printf("hex: %04x\n", hexval);
    send_command(fd, &buffer[0]);
    usleep(90000);
    send_command(fd, &buffer[0]);
    usleep(90000);
}

void addToGroup(int group, int shortAddr)
{
    char buffer[7];
    printf("Adding to group %d device %d\n", group, shortAddr);
    print_nbits(shortAddr, 6);
    char devID[6]; 
    memcpy(devID, bitToStringArr,6);
    //puts(devID);
    print_nbits(group, 4);
    char GrpAddr[4];
    memcpy(GrpAddr, bitToStringArr,4);
    //strcpy(GrpAddr, bitToStringArr);
    //puts(GrpAddr);
    char *longAddr = NULL;
    asprintf(&longAddr, "0%c%c%c%c%c%c10110%c%c%c%c",devID[0],devID[1],devID[2],devID[3],devID[4],devID[5],GrpAddr[0],GrpAddr[1],GrpAddr[2],GrpAddr[3]); //%c%c%c%c%c%c  devID[0],devID[1],devID[2],devID[3],devID[4],devID[5],
    printf("Bytestring: %s\n", longAddr);
    long hexval = strtol(longAddr, NULL, 2);
    sprintf(buffer, "%04x\n", hexval);
    printf("Hex: %04x\n", hexval);
    send_command(fd, &buffer[0]);
    usleep(90000);
    send_command(fd, &buffer[0]);
    usleep(90000);
}

int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        printf("Usage: %s add | remove | all <group ID> <Device ID>\n", argv[0]);
        printf("Example: add 0 2\n");
        printf("add to 0 group device with id 2\n");
        printf("all 0 0 \n");
        printf("scan config.json and add all devices\n");
        exit(0);
    }
    printf("Group: %s ", argv[2]);
    printf("Device: %s\n", argv[3]);
    char *p;
    int shortAddr;
    int group;

    errno = 0;
    long parseshortAddr = strtol(argv[3], &p, 10);
    long parseGroup = strtol(argv[2], &p, 10);

    shortAddr = parseshortAddr;
    group = parseGroup;

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
    // sprintf(buffer, "0370");
    // send_command(fd, &buffer[0]);
    // usleep(90000);
    // send_command(fd, &buffer[0]);
    // usleep(90000);
    // send_command(fd, "A100");
    // registerGroups();
    // addToGroup(group, shortAddr);
    if (strcmp("add", argv[1]) == 0)
    {
        addToGroup(group, shortAddr);
    }
    if (strcmp("remove", argv[1]) == 0)
    {
        removeFromGroup(group, shortAddr);
    }
    if (strcmp("all", argv[1]) == 0)
    {
        registerGroups();
    }
    send_command(fd, "A100");
}