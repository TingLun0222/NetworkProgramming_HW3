#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char *int_to_str(int n)
{
    int i = 0, sign = n;
    char c;
    char *s = malloc(12); // 11 digits + sign + '\0'

    if (s == NULL)
    {
        return NULL; // Allocation failed
    }

    if (sign < 0)
    {
        n = -n;
        s[i++] = '-';
    }

    do
    {
        s[i++] = n % 10 + '0';
        n /= 10;
    } while (n > 0);

    s[i] = '\0';

    // Reverse the string
    int len = strlen(s);
    for (int j = 0; j < len / 2; j++)
    {
        c = s[j];
        s[j] = s[len - j - 1];
        s[len - j - 1] = c;
    }

    return s;
}

int str_to_int(char *s)
{
    int n = 0, sign = 1;

    if (*s == '-')
    {
        sign = -1;
        s++;
    }

    while (*s != '\0')
    {
        n = n * 10 + *s - '0';
        s++;
    }

    return n * sign;
}

void removeFirstCharacter(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        str[i] = str[i+1]; // 將後一個字元往前移動
        i++;
    }
}