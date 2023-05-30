#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#define SIZE 256

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "Vhf:")) != -1) {
        switch (opt) {
            case 'V':
                printf("my-look from CS537 Summer 2022\n");
                exit(0);

            case 'h':
                printf("In order to search type: ./my-look -f file/name prefix\n");
                exit(0);
            
            case 'f': ;

                FILE* fp;
                fp = fopen(argv[2], "r");
                if (fp == NULL) {
                    printf("my-look: cannot open file\n");
                    exit(1);
                }

                char src[SIZE], dst[SIZE];

                while (1) {
                    fgets(src, SIZE, fp);
                    fgets(dst, SIZE, fp);
                    // 1. Get a dst buffer and strip of all punct: 
                    // 2. Compare all chars in prefix to dst:
                    // 3. If match, then print src buuffer:

                    for (int i = 0; i < strlen(dst); i++) {
                        if (ispunct(dst[i])) {
                            memmove(&dst[i], &dst[i + 1], strlen(dst) - i);
                            //counter++;
                        }
                    }

                    if (strncasecmp(argv[3], dst, strlen(argv[3])) == 0) {
                        printf("%s", src);
                    }


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

    // Start of stdin, to use command prompt to search for a prefix on a string.
    char str[SIZE];
    char prefix[SIZE];

    printf("Stdin: Please type a string to search.\n");
    scanf("%s", str);
    if (str == NULL) {
        //throw a fit!
    }
    printf("Stdin: Please type a prefix to search on the string.\n");
    scanf("%s", prefix);

    if (strncasecmp(prefix, str, strlen(prefix)) == 0) {
        printf("%s", str);
    }

    return 0;
}
