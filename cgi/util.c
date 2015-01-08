#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

void str_split(char* str, const char* tok, char*** res, int* count) {

    // To avoid Segmentation fault we need to convert to char array....
    // Maybe I need to write a new strtok function?
    char buff[1024];
    strncpy(buff, str, 1024);

    // some var for split
    int _count = 0;
    char* _res[5000];
    char* tmp_str;

    tmp_str = strtok(buff, tok);

    while(tmp_str != NULL) {
        int _len = strlen(tmp_str) + 1;
        _res[_count] = malloc(sizeof(char)*_len);
        strcpy(_res[_count], tmp_str);
        _count++;
        tmp_str = strtok(NULL, tok);
    }

    *count = _count;
    *res = _res;

}

int is_match(const char* str, char* regex_str) {
    regex_t regex;
    int res;
    const int nmatch = 1;
    regmatch_t pmatch[nmatch];

    // compile regex
    if(regcomp(&regex, regex_str, REG_EXTENDED) != 0) {
        // if there contains an error
        regfree(&regex);
        return -1;
    }

    // check match
    res = regexec(&regex, str, nmatch, pmatch, 0);

    // free regex data
    regfree(&regex);

    // return result
    if(res == REG_NOMATCH){
        return 0;// no match
    } else {
        return 1;// match
    }

}

void str_replace_one_world(char* str, char match, char to_replace) {
    int c=0;
    while(str[c] != '\0') {
        if(str[c] == match) {
            str[c] = to_replace;
        }
        c++;
    }
}

int count_char_num(char* str, char c) {
    int count = 0;
    while(*str != '\0') {
        if(*str == c) {
            count++;
        }
        str++;
    }
    return count;
}

int str_ends_with(char* str, char c) {
    int len = strlen(str);
    return str[len-1] == c;
}

int str_starts_with(char* str, char c) {
    return str[0] == c;
}
void replace_to_html(char* str) {

    int c, d;
    c = 0;
    d = 0;
    char tmp_str[10001];
    bzero(tmp_str, 10001);
    while(str[c] != '\0') {
        if(str[c] == '\n') {
            tmp_str[d++] = '<';
            tmp_str[d++] = 'b';
            tmp_str[d++] = 'r';
            tmp_str[d++] = '>';
            c++;
        } else if(str[c] == '\r') {
            c++;
        } else if(str[c] == '<') {
            tmp_str[d++] = '&';
            tmp_str[d++] = 'l';
            tmp_str[d++] = 't';
            tmp_str[d++] = ';';
            c++;
        } else if(str[c] == '>') {
            tmp_str[d++] = '&';
            tmp_str[d++] = 'g';
            tmp_str[d++] = 't';
            tmp_str[d++] = ';';
            c++;
        } else {
            tmp_str[d++] = str[c++];
        }
    }
    strcpy(str, tmp_str);

}


int get_ip_num(char* ip, int pos) {

    int c;
    int num = 0;
    int len = strlen(ip);
    int pos_t = 0;
    for(c=0; c<len; c++) {
        if(pos_t == pos && ip[c] == '.') {
            break;
        }
        if(pos_t == pos) {
            num += ip[c] - '0';
            num *= 10;
        }
        if(pos_t != pos && ip[c] == '.') {
            pos_t++;
        }
    }
    return num/10;
}
