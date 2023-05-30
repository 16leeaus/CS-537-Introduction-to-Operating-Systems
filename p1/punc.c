#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define SIZE 256

void strip_punc(char *s) {
    strcpy(dst, src);
   
    for (int i = 0; i < strlen(dst); i++) {
        if (ispunct(dst[i])) {
            memmove(&dst[i], &dst[i + 1], strlen(dst) - i);
            counter++;
        }
    }

    printf("Source String: ");
    printf("%s\n", src);
    printf("Destination String: ");
    printf("%s\n", dst);
}

// Use this to strip strings of puncuation:
int main() {
    char src[SIZE], dst[SIZE];
    int counter = 0;
   
    printf("Enter a string: ");
    fgets(src, sizeof(src), stdin); // take input
    strip_punc(src);
    
    return 0;
}


/* strcpy(dst, src);
   
    for (int i = 0; i < strlen(dst); i++) {
        if (ispunct(dst[i])) {
            memmove(&dst[i], &dst[i + 1], strlen(dst) - i);
            counter++;
        }
    }

    printf("Source String: ");
    printf("%s\n", src);
    printf("Destination String: ");
    printf("%s\n", dst); */