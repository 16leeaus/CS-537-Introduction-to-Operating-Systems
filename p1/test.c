#include <stdio.h>
#include <ctype.h>

#define SIZE 256

int insenistive_strcmp(char const* s1, char const* s2)
{
    unsigned char const* p1 = (unsigned char const*)s1;
    unsigned char const* p2 = (unsigned char const*)s2;

    for (;; ++p1, ++p2)
    {
        while (ispunct(*p1))
            ++p1;

        while (ispunct(*p2))
            ++p2;
        
        int ch1 = toupper(*p1);
        int ch2 = toupper(*p2);

        if (!ch1 || !ch2 || ch1 != ch2)
            return ch1 - ch2;
    }
}

void filter_alpha(char *s) {
  for (char *p = s; *p; ++p)
    if (isalpha(*p))
      *s++ = *p;
  *s = '\0';
}

int main(int argc, char *argv[])
{
    //printf("%d\n", insenistive_strcmp(argv[1], argv[2]));

    char s[] = argv[1];
    filter_alpha(s);
    printf("'%s'\n", s);
}