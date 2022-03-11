#include <stdbool.h>                       
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "list.h"

static int find(struct list_head *heads, int size, int key);
char *fractionToDecimal(int numerator, int denominator);
void free_list(struct list_head *heads, size_t size);

struct rem_node {
    int key;
    int index;
    struct list_head link;
};  
    
static int find(struct list_head *heads, int size, int key)
{
    struct rem_node *node;
    int hash = key % size;
    list_for_each_entry (node, &heads[hash], link) {
        if (key == node->key)
            return node->index;
    }
    return -1;
}

char *fractionToDecimal(int numerator, int denominator)
{
    int size = 1024;
    char *result = malloc(size);
    char *p = result;

    if (denominator == 0) {
        result[0] = '\0';
        return result;
    }

    if (numerator == 0) {
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    /* using long long type make sure there has no integer overflow */
    long long n = numerator;
    long long d = denominator;

    /* deal with negtive cases */
    n= ((n >> 63) ^ n) - (n >> 63);
    d = ((d >> 63) ^ d) - (d >> 63);

    bool sign = (float) numerator / denominator >= 0;
    if (!sign)
        *p++ = '-';

    long long remainder = n % d;
    long long division = n / d;

    sprintf(p, "%ld", division > 0 ? (long) division : (long) -division);
    if (remainder == 0)
        return result;

    p = result + strlen(result);
    *p++ = '.';

    /* Using a map to record all of reminders and their position.
     * if the reminder appeared before, which means the repeated loop begin,
     */
    char *decimal = malloc(size);
    memset(decimal, 0, size);
    char *q = decimal;
    char *decimal_origin = decimal;

    size = 1333;
    struct list_head *heads = malloc(size * sizeof(*heads));
    for (int i = 0; i < size; i++)
        INIT_LIST_HEAD(&heads[i]);

    for (int i = 0; remainder; i++) {
        int pos = find(heads, size, remainder);
        if (pos >= 0) {
            while (pos-- > 0)
                *p++ = *decimal++;
            *p++ = '(';
            while (*decimal != '\0')
                *p++ = *decimal++;
            *p++ = ')';
            *p = '\0';

            goto finish;
        }
        struct rem_node *node = malloc(sizeof(*node));
        node->key = remainder;
        node->index = i;

        list_add(&node->link, &heads[remainder % size]);

        *q++ = (remainder * 10) / d + '0';
        remainder = (remainder * 10) % d;
    }

    strcpy(p, decimal);
    
    finish:
        free_list(heads, size);
        free(decimal_origin);

    return result;
}

void free_list(struct list_head *heads, size_t size)
{
     for (int i = 0; i < size; i++)
    {
        struct rem_node *n, *s;
        list_for_each_entry_safe(n, s, &heads[i], link)
        {
            free(n);
        }
    }
    free(heads);
}

int main(){
    
    char* res1 = fractionToDecimal(-2, 3);
    char* res2 = fractionToDecimal(3, -2);
  
    printf("farctionToDecimal(-2, 3) = %s\n", res1);
    printf("farctionToDecimal(3, -2) = %s\n", res2);

    free(res1);
    free(res2);

    return 0;
}
