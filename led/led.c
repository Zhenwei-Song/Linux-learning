
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LED_TRIGGER "/sys/class/leds/sys-led/trigger"
#define LED_BRIGHTNESS "/sys/class/leds/sys-led/brightness"

int main(int argc, char **argv)
{
    FILE *fd1, *fd2;
    if (argc < 2) {
        fprintf(stderr, "usage:\n%s <on|off>\n %s <trigger> <type>\n", argv[0], argv[0]);
        exit(-1);
    }

    fd1 = fopen(LED_TRIGGER, "r+");
    if (fd1 == NULL) {
        perror("open error\n");
        exit(-1);
    }

    fd2 = fopen(LED_BRIGHTNESS, "r+");
    if (fd2 == NULL) {
        perror("open error\n");
        exit(-1);
    }

    if (strcmp(argv[1], "on") == 0) {
        fwrite("none", 1, 4, fd1);
        fwrite("1", 1, 1, fd2);
    } else if (strcmp(argv[1], "off") == 0) {
        fwrite("none", 1, 4, fd1);
        fwrite("0", 1, 1, fd2);
    } else if (strcmp(argv[1], "trigger") == 0) {
        if (argc != 3) {
            fprintf(stderr, "usage:\n%s <on|off>\n %s <trigger> <type>\n", argv[0], argv[0]);
            exit(-1);
        }
        if (fwrite(argv[2], 1, strlen(argv[2]), fd1) != strlen(argv[2])) {
            perror("write error\n");
            fprintf(stderr, "usage:\n%s <on|off>\n %s <trigger> <type>\n", argv[0], argv[0]);
            exit(-1);
        }

    } else {
        fprintf(stderr, "usage:\n%s <on|off>\n %s <trigger> <type>\n", argv[0], argv[0]);
        exit(-1);
    }
    exit(0);
}