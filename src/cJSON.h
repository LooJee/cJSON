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

pJsonObj_T cJsonNew();
void cJsonAddInt(pJsonObj_T obj, const char *key, long value);
void cJsonAddString(pJsonObj_T obj, const char *key, const char *value);
void cJsonAddObj(pJsonObj_T obj, const char *key, pJsonObj_T value);
void cJsonAddBool(pJsonObj_T obj, const char *key, bool value);
void cJsonAddArray(pJsonObj_T obj, const char *key, pJsonNode_T value[]);
void *cJsonValue(pJsonObj_T obj, const char *key);
JSONTYPE_E cJsonValueType(pJsonObj_T obj, const char *key);
void cJsonDel(pJsonObj_T obj, const char *key);
void cJsonFree(pJsonObj_T obj);
void cJsonMashal(pJsonObj_T obj);
void cJsonParse(const char *text);
void cJsonPrint(pJsonObj_T obj);

#endif //CJSON_CJSON_H
