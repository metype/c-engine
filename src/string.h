#ifndef CENGINE_STRING_H
#define CENGINE_STRING_H

#include "ref.h"

// s(str) Create a new string
#define s(str) S_create(str)

// so(str) Create a new temporary string (calls S_final on result)
#define so(str) S_final(S_create(str))

// sc(str) Create a string from an existing char*
#define sc(str) S_convert(str)

// sc(str) Create a temporary string from an existing char*
#define sco(str) S_final(S_convert(str))

// s_cat(str_1, str_2) Concatenates two strings, mutating the first.
#define s_cat(str_1, str_2) str_1 = S_append(S_final(str_1), str_2)

// s_rep(str_1, str_s, str_r) Searches for and replaces every occurrence of str_s with str_r in str_1, mutates str_1.
#define s_rep(str_1, str_s, str_r) str_1 = S_replace(S_final(str_1), str_s, str_r)

// s_rep_n(str_1, str_s, str_r, max) Searches for and replaces up to max occurrences of str_s with str_r in str_1, mutates str_1.
#define s_rep_n(str_1, str_s, str_r, max) str_1 = S_replace_n(S_final(str_1), str_s, str_r, max)

typedef struct string {
    char* c_str;
    long length;
    ref refcount;
} string;

static void string_free(const ref *ref) {
    ref_counted_free_begin(string, str);
    free(str->c_str);
    ref_counted_free_end(str);
}

// Take an existing CString and convert it to a string
string* S_convert(char* c_str);

// Creates a string from a CONST CString.
string* S_create(const char* c_str);

// Appends two strings and returns a new string, does not modify the input strings
string* S_append(const string* str_1, const string* str_2);

// Replaces all occurrences of str_search in str_1 with str_replace
string* S_replace(const string* str_1, const string* str_search, const string* str_replace);

// Replaces the first max_replacements occurrences of str_search in str_1 with str_replace
string* S_replace_n(const string* str_1, const string* str_search, const string* str_replace, int max_replacements);

// Copies a string
string* S_copy(const string* str_1);

// Returns the length of a string
unsigned int S_length(const string* str_1);

// Marks this string as final. The next string method that uses it will free the string.
string *S_final(string* str_1);

#endif //CENGINE_STRING_H
