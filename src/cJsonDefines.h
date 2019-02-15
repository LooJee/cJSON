//
// Created by Zer0 on 2019/2/14.
//

#ifndef CJSON_CJSONDEFINES_H
#define CJSON_CJSONDEFINES_H

#define S_FREE(p) do {  \
    if (p) {            \
        free(p);        \
        p = NULL;       \
    }                   \
} while(0);

#define SKIP_GAP(p) do {    \
    if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') {  \
        ++p;                \
    } else {                \
        break;              \
    }                       \
}while(true);

#define KEY_VAL_STR             "\"%s\":\"%s\"" //"key":prefix val suffix;
#define KEY_VAL_INT             "\"%s\":%ld"
#define KEY_VAL_BOOL            "\"%s\":%s"     // for bool, json object
#define STR_PRE_SUF_FIX         '\"'
#define OBJ_PRE_FIX             '{'
#define OBJ_SUF_FIX             '}'
#define ARR_PRE_FIX             '['
#define ARR_SUF_FIX             ']'
#define BOOL_TRUE_PRE_FIX       't'
#define BOOL_FALSE_PRE_FIX      'f'
#define COLON_SYMBOL            ':'
#define COMMA_SYMBOL            ','
#define BASE_STR_SIZE           128
#define SHOULD_REALLOC          64
#define TRUE_STR                "true"
#define FALSE_STR               "false"
#define MAX_NUM_STR_LEN         32
#define MAX_BOOL_STR_LEN        6


#endif //CJSON_CJSONDEFINES_H
