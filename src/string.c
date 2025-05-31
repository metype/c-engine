#include <malloc.h>
#include <string.h>
#include <limits.h>
#include "string.h"

string *S_char_to_str(char character) {
    char* new_str = malloc(sizeof(char) * 2);
    new_str[0] = character;
    new_str[1] = 0;
    return S_convert(new_str);
}

string* S_convert(char* c_str) {
    unsigned total_len = strlen(c_str) + 1;
    string* str = malloc(sizeof(string));
    str->c_str = c_str;
    str->length = total_len - 1;
    str->refcount = (struct ref){string_free, 1};
    return str;
}

string* S_create(const char* c_str) {
    char* new_str = strdup(c_str);
    return S_convert(new_str);
}

string* S_append(const string* str_1, const string* str_2) {
    ref_inc(&str_1->refcount);
    ref_inc(&str_2->refcount);

    unsigned total_len = S_length(str_1) + S_length(str_2) + 1;
    char* new_str = malloc(sizeof(char) * total_len);
    snprintf(new_str, total_len, "%s%s", str_1->c_str, str_2->c_str);

    ref_dec(&str_1->refcount);
    ref_dec(&str_2->refcount);
    return S_convert(new_str);
}

string* S_replace(const string* str_1, const string* str_search, const string* str_replace) {
    return S_replace_n(str_1, str_search, str_replace, INT_MAX);
}

string* S_replace_n(const string* str_1, const string* str_search, const string* str_replace, int max_replacements) {
    ref_inc(&str_1->refcount);
    ref_inc(&str_search->refcount);
    ref_inc(&str_replace->refcount);
    // What this does is takes the length of str_1, and multiplies it by str_replace.
    // The max length of a new string is this value:
    // S_replace(s("aaaaa"), s("a"), s("ba")) -> bababababa
    // This is our worst case, every character of str_1 becoming str_replace;

    unsigned total_len = (S_length(str_1) * (S_length(str_replace) + 1)) + 1;
    char *new_str = malloc(sizeof(char) * total_len);

    unsigned replace_len = S_length(str_replace);
    unsigned search_len = S_length(str_search);
    unsigned input_len = S_length(str_1);

    unsigned replacement_count = 0;

    char *where = str_1->c_str;

    if (max_replacements < 0) {
        while ((where = strstr(where, str_search->c_str))) {
            where += search_len;
            replacement_count++;
        }
        max_replacements = (int)replacement_count + max_replacements;
        replacement_count = 0;
    }

    unsigned idx = 0;
    unsigned str_idx = 0;

    for(unsigned i = 0; i <= input_len - search_len; i++) {
        bool match = true;
        for(unsigned j = 0; j < search_len; j++) {
            if(str_1->c_str[i + j] != str_search->c_str[j]) {
                match = false;
                break;
            }
        }
        if(match && replacement_count < max_replacements) {
            for(unsigned j = 0; j < replace_len; j++) {
                new_str[idx++] = str_replace->c_str[j];
            }
            str_idx += search_len;
            replacement_count++;
            continue;
        }
        new_str[idx++] = str_1->c_str[str_idx++];
    }

    // Cap it off
    new_str[idx] = 0;

    ref_dec(&str_1->refcount);
    ref_dec(&str_search->refcount);
    ref_dec(&str_replace->refcount);
    return S_convert(new_str);
}

string* S_copy(const string* str_1) {
    return S_create(str_1->c_str);
}

unsigned int S_length(const string* str_1) {
    ref_inc(&str_1->refcount);

    if(str_1->length > 0) {
        unsigned len = str_1->length;
        ref_dec(&str_1->refcount);
        return len;
    }
    unsigned len = strlen(str_1->c_str);

    ref_dec(&str_1->refcount);
    return len;
}

string *S_final(string* str_1) {
    if(!str_1) {
        string* str = s("");
        str->refcount.count = 0;
        return str;
    }
    // this is all "marking as final" is lol
    str_1->refcount.count = 0;
    return str_1;
}