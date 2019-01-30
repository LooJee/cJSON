//
// Created by Zer0 on 2019/1/30.
//

#ifndef CJSON_CJSON_H
#define CJSON_CJSON_H

typedef enum {
    TYPE_NULL,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_OBJECT,
    TYPE_ARRAY
}JSONTYPE_E;

typedef struct JsonNode_S {
    char                *key;
    void                *value;
    JSONTYPE_E          type;
    struct JsonNode_S   *next;
}JsonNode_T, *pJsonNode_T;

typedef struct {
    unsigned long size;
    pJsonNode_T head;
}JsonObj_T, *pJsonObj_T;

pJsonObj_T cJsonNew();
void cJsonAdd(pJsonObj_T obj, const char *key, const void *value, JSONTYPE_E type);
void *cJsonValue(pJsonObj_T obj, const char *key);
JSONTYPE_E cJsonValueType(pJsonObj_T obj, const char *key);
void cJsonDel(pJsonObj_T obj, const char *key);
void cJsonFree(pJsonObj_T obj);
void cJsonMashal(pJsonObj_T obj);
void cJsonParse(const char *text);

#endif //CJSON_CJSON_H
