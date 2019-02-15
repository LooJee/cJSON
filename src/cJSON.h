//
// Created by Zer0 on 2019/1/30.
//

#ifndef CJSON_CJSON_H
#define CJSON_CJSON_H

#include <stdbool.h>

typedef enum {
    TYPE_NULL,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_OBJECT,
    TYPE_ARRAY
}JSONTYPE_E;

// json object 字符串解析状态机
typedef enum {
    OBJ_STATE_IDLE = 0,
    OBJ_STATE_WAIT_FOR_KEY,
    OBJ_STATE_PARSE_KEY_START,
    OBJ_STATE_PARSE_KEY_END,
    OBJ_STATE_WAIT_FOR_VAL,
    OBJ_STATE_PARSE_VAL_NUM_START,
    OBJ_STATE_PARSE_VAL_BOOL_START,
    OBJ_STATE_PARSE_VAL_STR_START,
    OBJ_STATE_PARSE_VAL_OBJ_START,
    OBJ_STATE_PARSE_VAL_ARR_START,
    OBJ_STATE_PARSE_VAL_END,
    OBJ_STATE_ERROR,
    OBJ_STATE_SUCCESS,
    OBJ_STATE_DONE,
}OBJ_PARSE_STATE_E;

//json array 字符串解析状态机
typedef enum {
    ARR_STATE_IDLE = 0,
    ARR_STATE_WAIT_FOR_VAL,
    ARR_STATE_PARSE_NUM_START,
    ARR_STATE_PARSE_BNOOL_START,
    ARR_STATE_PARSE_STR_START,
    ARR_STATE_PARSE_OBJ_START,
    ARR_STATE_PARSE_ARR_START,
    ARR_STATE_PARSE_VAL_END,
    ARR_STATE_ERROR,
    ARR_STATE_SUCCESS,
    ARR_STATE_DONE,
}ARR_PARSE_STATE_E;

typedef union {
    long lVal;
    char *stringVal;
    bool boolVal;
    struct JsonObj_S *objVal;
    struct JsonArray_S *arrVal;
}JsonValue_U;

typedef struct JsonNode_S {
    char                *key;
    struct JsonNode_S   *next;
    struct JsonNode_S   *prev;
    JsonValue_U         value;
    JSONTYPE_E          type;
}JsonNode_T, *pJsonNode_T;

typedef struct JsonArray_S{
    pJsonNode_T node;
    unsigned long size;
}JsonArray_T, *pJsonArray_T;

typedef struct JsonObj_S{
    unsigned long size;
    pJsonNode_T head;
    char *jsonStr;
}JsonObj_T, *pJsonObj_T;

typedef struct {
    OBJ_PARSE_STATE_E state;
    const char *text;
    pJsonObj_T obj;
    pJsonNode_T curNode;
}ParserStruct_T, *pParserStruct_T;


pJsonObj_T cJsonNew();
void cJsonAddInt(pJsonObj_T obj, const char *key, long value);
void cJsonAddString(pJsonObj_T obj, const char *key, const char *value);
void cJsonAddObj(pJsonObj_T obj, const char *key, pJsonObj_T value);
void cJsonAddBool(pJsonObj_T obj, const char *key, bool value);
void cJsonAddArray(pJsonObj_T obj, const char *key, pJsonNode_T value[]);
void *cJsonValue(pJsonObj_T obj, const char *key);
JSONTYPE_E cJsonValueType(pJsonObj_T obj, const char *key);
void cJsonDel(pJsonObj_T obj, const char *key);
void cJsonFree(pJsonObj_T *obj);
char *cJsonMashal(pJsonObj_T obj);
void cJsonPrint(pJsonObj_T obj);
pJsonObj_T cJsonParse(const char *text);

#endif //CJSON_CJSON_H
