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
    TYPE_ARRAY,
    TYPE_MAX
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
    ARR_STATE_PARSE_BOOL_START,
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
    pJsonNode_T head;
    pJsonNode_T end;
    unsigned long size;
}JsonArray_T, *pJsonArray_T;

typedef struct JsonObj_S{
    unsigned long size;
    pJsonNode_T head;
}JsonObj_T, *pJsonObj_T;

typedef union {
    pJsonObj_T obj;
    pJsonArray_T arr;
}Json_U;

typedef union {
    OBJ_PARSE_STATE_E obj_state;
    ARR_PARSE_STATE_E arr_state;
}PARSE_STATE_U;

typedef struct {
//    OBJ_PARSE_STATE_E state;
    PARSE_STATE_U state;
    const char *text;
//    pJsonObj_T obj;
    Json_U  json;
    pJsonNode_T curNode;
    int ret;
    bool isSubParser;
}ParserStruct_T, *pParserStruct_T;


pJsonObj_T cJsonNew();
void cJsonAddInt(pJsonObj_T obj, const char *key, long value);
void cJsonAddString(pJsonObj_T obj, const char *key, const char *value);
void cJsonAddObj(pJsonObj_T obj, const char *key, pJsonObj_T value);
void cJsonAddBool(pJsonObj_T obj, const char *key, bool value);
void cJsonAddArray(pJsonObj_T obj, const char *key, pJsonArray_T val);
void cJsonDel(pJsonObj_T obj, const char *key);
void cJsonFree(pJsonObj_T *obj);
char *cJsonMashal(pJsonObj_T obj);
void cJsonPrint(pJsonObj_T obj);
pJsonObj_T cJsonParse(const char *text);

pJsonArray_T cJsonArrNew();
void cJsonArrFree(pJsonArray_T *arr);
void cJsonArrDel(pJsonArray_T arr, size_t idx);
void cJsonArrAppend(pJsonArray_T arr, pJsonNode_T val);
void cJsonArrAppendNum(pJsonArray_T arr, long val);
void cJsonArrAppendString(pJsonArray_T arr, const char *val);
void cJsonArrAppendBool(pJsonArray_T arr, bool val);
void cJsonArrAppendObj(pJsonArray_T arr, pJsonObj_T val);
void cJsonArrAppendArr(pJsonArray_T arr, pJsonArray_T val);
void cJsonArrInsertAt(pJsonArray_T arr, size_t idx, pJsonNode_T val);
void cJsonArrInsertNumAt(pJsonArray_T arr, size_t idx, long val);
void cJsonArrInsertStringAt(pJsonArray_T arr, size_t idx, const char *val);
void cJsonArrInsertBoolAt(pJsonArray_T arr, size_t idx, bool val);
void cJsonArrInsertObjAt(pJsonArray_T arr, size_t idx, pJsonObj_T val);
void cJsonArrInsertArrAt(pJsonArray_T arr, size_t idx, pJsonArray_T val);
void cJsonArrReplaceAt(pJsonArray_T arr, size_t idx, pJsonNode_T val);
void cJsonArrReplaceNumAt(pJsonArray_T arr, size_t idx, long val);
void cJsonArrReplaceStringAt(pJsonArray_T arr, size_t idx, const char *val);
void cJsonArrReplaceBoolAt(pJsonArray_T arr, size_t idx, bool val);
void cJsonArrReplaceObjAt(pJsonArray_T arr, size_t idx, pJsonObj_T val);
void cJsonArrReplaceArrAt(pJsonArray_T arr, size_t idx, pJsonArray_T val);
void cJsonArrPrint(pJsonArray_T arr);
char *cJsonArrMashal(pJsonArray_T arr);
pJsonNode_T cJsonArrAt(pJsonArray_T arr, size_t idx);
pJsonArray_T cJsonArrParse(const char *text);

pJsonNode_T cJsonVal(pJsonObj_T obj, const char *key);
JSONTYPE_E cJsonValType(pJsonNode_T n);
long cJsonValNum(pJsonNode_T node);
const char *cJsonValString(pJsonNode_T node);
bool cJsonValBool(pJsonNode_T node);
pJsonObj_T cJsonValObj(pJsonNode_T node);
pJsonArray_T cJsonValArr(pJsonNode_T node);

#endif //CJSON_CJSON_H
