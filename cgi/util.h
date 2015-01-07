#ifndef __UTIL_H__
#define __UTIL_H__
#define INS_MODE_BEF 0
#define INS_MODE_AFT 1
#define INS_MODE_BOTH 2

void str_split(char* str, const char* tok, char*** res, int* count);
int is_match(const char* str, char* regex);
void str_replace_one_world(char* str, char match, char to_replace);
int count_char_num(char* str, char c);
void replace_to_html(char* str);
int str_ends_with(char* str, char c);
int str_starts_with(char* str, char c);
#endif
