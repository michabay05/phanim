#ifndef __PHSTR_H__
#define __PHSTR_H__

#include <stddef.h>

typedef struct {
    char *text;
    size_t count, capacity;
} PhanimStr;

void PhanimStrInit(PhanimStr *str, const char *text);
void PhanimStrDeinit(PhanimStr *str);
void PhanimStrClear(PhanimStr *str);
void PhanimStrDestroy(void);
void PhanimStrAppend(PhanimStr *str, const char *text);
void PhanimStrConcat(PhanimStr *str, PhanimStr *other);
int PhanimStrIndexOf(PhanimStr *str, const char *pattern, size_t offset);
void PhanimStrPrint(PhanimStr *str);

// Potential addition to these functions
//    - bool PhanimStrEquals(PhanimStr a, PhanimStr b);
//    - char PhanimStrCharAt(PhanimStr a, size_t index);
//    - char PhanimStrSubstr(PhanimStr a, size_t start, size_t end);
//    * For more, look at the Java String implementation.

#endif // __PHSTR_H__

#ifdef PHANIM_STR_IMPLEMENTATION
#include <string.h>
#include "arena.h"

Arena temp = {0};

void PhanimStrInit(PhanimStr *str, const char *text)
{
    size_t n = strlen(text);
    str->count = n;
    str->capacity = 2 * n;
    // str->text = arena_memdup(&temp, (char*)text, n);
    str->text = arena_alloc(&temp, n * sizeof(char));
    for (size_t i = 0; i < n; i++) {
        str->text[i] = text[i];
    }
}

void PhanimStrAppend(PhanimStr *str, const char *text)
{
    size_t n = strlen(text);
    if (str->count + n >= str->capacity) {
        size_t new_cap = (str->count + n) * 2;
        str->text = arena_realloc(&temp, str->text, str->capacity * sizeof(*str->text), new_cap * sizeof(*str->text));
        str->capacity = new_cap;
    }

    for (size_t i = 0; i < n; i++) {
        str->text[str->count + i] = text[i];
    }
    str->count += n;
}

void PhanimStrConcat(PhanimStr *str, PhanimStr *other)
{
    size_t n = other->count;
    if (str->count + n >= str->capacity) {
        size_t new_cap = (str->count + n) * 2;
        str->text = arena_realloc(&temp, str->text, str->capacity * sizeof(*str->text), new_cap * sizeof(*str->text));
        str->capacity = new_cap;
    }

    for (size_t i = 0; i < n; i++) {
        str->text[str->count + i] = other->text[i];
    }
    str->count += n;
}

int PhanimStrIndexOf(PhanimStr *str, const char *pattern, size_t offset)
{
    char *pos = strstr(str->text, pattern);
    return pos == NULL ? -1 : pos - str->text;
}

void PhanimStrPrint(PhanimStr *str)
{
    // Not really an efficient method, but it doesn't really matter!
    for (size_t i = 0; i < str->count; i++) {
        printf("%c", str->text[i]);
    }
}

void PhanimStrClear(PhanimStr *str)
{
    str->count = 0;
}

void PhanimStrDeinit(PhanimStr *str)
{
    // Not sure, if I should free the memory here or do it in the destroy
}

void PhanimStrDestroy(void)
{
    arena_free(&temp);
}

#endif // PHSTR_IMPLEMENTATION
