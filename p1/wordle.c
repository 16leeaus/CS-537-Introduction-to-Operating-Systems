#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

#define SIZE 256

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "Vhf:")) != -1) {
        switch (opt) {
            case 'V':
                printf("my-look from CS537 Summer 2022\n");
                exit(0);

            case 'h':
                printf("Display Help!\n");
                exit(0);
            
            case 'f': ;

                FILE* fp;
                fp = fopen(argv[2], "r");
                if (fp == NULL) {
                    printf("my-look: cannot open file\n");
                    exit(1);
                }

                char str[SIZE];
                char* r;

                while (1) {
                    fgets(str, SIZE, fp);

                    // Check strlen() is 5 plus null terminator:

                    // *** Replace this Block ***
                    if (strlen(str) == 6) {

                        // NEED TO FILTER OUT WHITESPACE!
                        r = strpbrk(str, argv[3]);
                        if (r != 0) {
                            continue;
                        }
                        else {
                            printf("%s", str);
                            //printf("%ld", strlen(src));
                        }
                    }

                    // *** Replace this Block ***

                    if (feof(fp))
                        break;
                }

                fclose(fp);

                exit(0);

            default:
                printf("my-look: invalid command line\n");
                exit(1);
        }
    }
    return 0;
}

// Move up before compile time:


/* if (strlen(str) == 6) {
    if (!hasspace(str)) {
    r = strpbrk(src, argv[3]);
        if (r != 0) {
            continue;
        }
        else {
            printf("%s", src);
            //printf("%ld", strlen(src));
        }
    }
} */
