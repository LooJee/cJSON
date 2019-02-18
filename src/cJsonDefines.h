//
// Created by Zer0 on 2019/2/14.
//

#ifndef CJSON_CJSONDEFINES_H
#define CJSON_CJSONDEFINES_H

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
#define STR_EOF                 '\0'
#define BASE_STR_SIZE           128
#define SHOULD_REALLOC          64
#define TRUE_STR                "true"
#define TRUE_STR_LEN            4
#define FALSE_STR               "false"
#define FALSE_STR_LEN           5
#define MAX_NUM_STR_LEN         32
#define MAX_BOOL_STR_LEN        6

#define S_FREE(p) do {  \
    if (p) {            \
        free(p);        \
        p = NULL;       \
    }                   \
} while(0);

#define IS_GAP_SYMBOL(c)  ((c == ' ' || c == '\t' || c == '\r' || c == '\n') ? true : false)
#define IS_BOOL_PREFIX(c) ((c == BOOL_TRUE_PRE_FIX || c == BOOL_FALSE_PRE_FIX) ? true : false)

#define SKIP_GAP(p) do {    \
    if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') {  \
        ++p;                \
    } else {                \
        break;              \
    }                       \
}while(true);



#endif //CJSON_CJSONDEFINES_H
