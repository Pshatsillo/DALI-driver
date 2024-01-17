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
char devices[10][64];

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
    if (!hasFile)
    {
        send_command(fd, "A700");
        usleep(90000);

        send_command(fd, "A700");
        usleep(90000);
    }
}

int SearchAndCompare(int fd, u_int32_t addr)
{
    u_int8_t h, m, l;
    char buffer[7];
    struct pollfd fds;
    int ret;
    int res = 0;

    h = addr >> 16;
    h = h & 0xff;

    m = addr >> 8;
    m = m & 0xff;

    l = addr & 0xff;

    sprintf(buffer, "B1%02x", h);
    send_command(fd, &buffer[0]);
    usleep(34194);

    sprintf(buffer, "B3%02x", m);
    send_command(fd, &buffer[0]);
    usleep(34194);

    sprintf(buffer, "B5%02x", l);
    send_command(fd, &buffer[0]);
    usleep(34194);

    send_command(fd, "A900");
    usleep(34194);

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
        read(fd, buffer, 7);
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

char *print_nbits(unsigned x, unsigned n)
{
    char *out = malloc(n);
    int i = int_bits();
    i = (n < i) ? n - 1 : i - 1;
    for (; i >= 0; i--)
    {
        if ((x >> i) & 1U)
        {
            out[(n - 1) - i] = '1';
        }
        else
        {
            out[(n - 1) - i] = '0';
        }
    }
    return out;
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

void addDeviceToJSON(int SearchAddr, int ShortAddr)
{
    JSON_Object *deviceJson = json_object(user_data);
    JSON_Array *devArr = json_object_get_array(deviceJson, "devices");
    char *object = NULL;
    asprintf(&object, "{\"Address\": \"%06x\", \"Short\": %d}", SearchAddr, ShortAddr);
    json_array_append_value(devArr, json_parse_string(object));
    char *serialized_string = NULL;
    serialized_string = json_serialize_to_string_pretty(user_data);
}

int findShortAddress(int Addr)
{
    JSON_Object *deviceJson = json_object(user_data);
    JSON_Array *devArr = json_object_get_array(deviceJson, "devices");
    for (int i = 0; i < json_array_get_count(devArr); i++)
    {
        JSON_Object *getDaliDevice = json_array_get_object(devArr, i);
        u_int8_t ShortAddr = json_object_get_number(getDaliDevice, "Short");
        if (ShortAddr == Addr)
        {
            return 0;
        }
    }
    return 1;
}

int findLongAddress(int Addr)
{
    JSON_Object *deviceJson = json_object(user_data);
    JSON_Array *devArr = json_object_get_array(deviceJson, "devices");
    for (int i = 0; i < json_array_get_count(devArr); i++)
    {
        JSON_Object *getDaliDevice = json_array_get_object(devArr, i);
        char *longAddr = NULL;
        asprintf(&longAddr, "%06x", Addr);
        int matchString = strcmp(json_object_get_string(getDaliDevice, "Address"), longAddr);
        if (matchString == 0)
        {
            return json_object_get_number(getDaliDevice, "Short");
        }
    }
    return -1;
}

void registerGroups()
{
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
                char *resp = print_nbits(ShortAddr, 6);
                char *devID = malloc(6);
                strcpy(devID, resp);
                free(resp);
                char *respG = print_nbits(i, 4);
                char *GrpAddr = malloc(4);
                strcpy(GrpAddr, respG);
                free(respG);
                char *longAddr = NULL;
                asprintf(&longAddr, "0%s10110%s", devID, GrpAddr);
                long hexval = strtol(longAddr, NULL, 2);
                sprintf(buffer, "%04x", hexval);
                send_command(fd, &buffer[0]);
                usleep(90000);
                send_command(fd, &buffer[0]);
                usleep(90000);
                printf("%d ", ShortAddr);
            }
        }
        printf("\n");
    }
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

                // sprintf(buffer, "0170");
                // send_command(fd, &buffer[0]);
                // usleep(90000);
                // send_command(fd, &buffer[0]);
                // usleep(90000);
                // send_command(fd, "A100");

    while (ShortAddr < 64)
    {
        if (findShortAddress(ShortAddr) != 0)
        {
            u_int32_t SearchAddr = 0xFFFFFF;
            u_int8_t Response = 0;
            u_int32_t LowLimit = 0;
            u_int32_t HighLimit = 0x1000000;
            Response = SearchAndCompare(fd, SearchAddr);
            if (Response)
            {
                printf("Device detected, address searching...\n");
                if (!SearchAndCompare(fd, SearchAddr - 1))
                {
                    SearchAndCompare(fd, SearchAddr);
                    int findShortAddr = findLongAddress(SearchAddr);
                    if (findShortAddr == -1)
                    {
                        sprintf(buffer, "B7%02x", ((ShortAddr << 1) | 1));
                        send_command(fd, &buffer[0]);
                        usleep(102582);
                        send_command(fd, "AB00");
                        usleep(34194);
                        printf("24-bit address found first %06x - assigning short address %d\n", SearchAddr, ShortAddr);
                        addDeviceToJSON(SearchAddr, ShortAddr);
                        break;
                    }
                    else
                    {
                        sprintf(buffer, "B7%02x", ((findShortAddr << 1) | 1));
                        send_command(fd, &buffer[0]);
                        usleep(102582);
                        send_command(fd, "AB00");
                        usleep(34194);
                        printf("Founded first in config address %06x - assigned short address %d\n", SearchAddr, findShortAddr);
                    }
                }
            }
            else
            {
                printf("No devices detected\n");
                break;
            }

            while (1)
            {
                SearchAddr = (long)((LowLimit + HighLimit) / 2);
                Response = SearchAndCompare(fd, SearchAddr);

                if (Response)
                {
                    if ((SearchAddr == 0) || (!SearchAndCompare(fd, SearchAddr - 1)))
                        break;
                    HighLimit = SearchAddr;
                }
                else
                    LowLimit = SearchAddr;
            }
            SearchAndCompare(fd, SearchAddr);
            int findShortAddr = findLongAddress(SearchAddr);
            if (findShortAddr == -1)
            {
                sprintf(buffer, "B7%02x", ((ShortAddr << 1) | 1));
                send_command(fd, &buffer[0]);
                usleep(102582);
                send_command(fd, "AB00");
                usleep(34194);
                printf("24-bit address found second %06x - assigning short address %d\n", SearchAddr, ShortAddr);
                addDeviceToJSON(SearchAddr, ShortAddr);
            }
            else
            {
                sprintf(buffer, "B7%02x", ((findShortAddr << 1) | 1));
                send_command(fd, &buffer[0]);
                usleep(102582);
                send_command(fd, "AB00");
                usleep(34194);
                printf("Founded second in config address %06x - assigned short address %d\n", SearchAddr, findShortAddr);
            }
        }
        ShortAddr++;
    }
    registerGroups();
    send_command(fd, "A100");
    printf("Init complete\n");

    if (0 != close(fd))
    {
        perror("Could not close device\n");
    }
    json_serialize_to_file_pretty(user_data, filename);
    json_value_free(user_data);
    return 0;
}
