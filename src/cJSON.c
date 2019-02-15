//
// Created by Zer0 on 2019/1/30.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "cJsonDefines.h"

//解析 json 函数指针
typedef int (*cJsonParserFunc) (pParserStruct_T st);

/*
    IDLE = 0,
    WAIT_FOR_KEY,
    PARSE_KEY_START,
    PARSE_KEY_END,
    WAIT_FOR_VAL,
    PARSE_VAL_NUM_START,
    PARSE_VAL_BOOL_START,
    PARSE_VAL_STRING_START,
    PARSE_VAL_OBJECT_START,
    PARSE_VAL_ARRAY_START,
    PARSE_VAL_END,
    ERROR,
    SUCCESS,
    DONE
 */
int cJsonParseObjStateIDLE(pParserStruct_T st);
int cJsonParseObjStateWFK(pParserStruct_T st);
int cJsonParseObjStatePKS(pParserStruct_T st);
int cJsonParseObjStatePKE(pParserStruct_T st);
int cJsonParseObjStateWFV(pParserStruct_T st);
int cJsonParseObjStatePVNS(pParserStruct_T st);
int cJsonParseObjStatePVBS(pParserStruct_T st);
int cJsonParseObjStatePVSS(pParserStruct_T st);
int cJsonParseObjStatePVOS(pParserStruct_T st);
int cJsonParseObjStatePVAS(pParserStruct_T st);
int cJsonParseObjStatePVE(pParserStruct_T st);
int cJsonParseObjStateERR(pParserStruct_T st);
int cJsonParseObjStateSuccess(pParserStruct_T st);

cJsonParserFunc gJsonObjParsers[] = {
    cJsonParseObjStateIDLE,
    cJsonParseObjStateWFK,
    cJsonParseObjStatePKS,
    cJsonParseObjStatePKE,
    cJsonParseObjStateWFV,
    cJsonParseObjStatePVNS,
    cJsonParseObjStatePVBS,
    cJsonParseObjStatePVSS,
    cJsonParseObjStatePVOS,
    cJsonParseObjStatePVAS,
    cJsonParseObjStatePVE,
    cJsonParseObjStateERR,
    cJsonParseObjStateSuccess
};


pJsonObj_T cJsonNew()
{
    pJsonObj_T obj = (pJsonObj_T)malloc(sizeof(JsonObj_T));
    if (obj == NULL) {
        perror("malloc obj failed\n");
        return NULL;
    }

    obj->head = NULL;
    obj->size = 0;
    obj->jsonStr = NULL;

    return obj;
}

void cJsonNodeFree(pJsonNode_T node)
{
    if (node == NULL) {
        return;
    }

    S_FREE(node->key);
    node->next = NULL;
    node->prev = NULL;
    if (node->type == TYPE_STRING) {
        S_FREE(node->value.stringVal);
    } else if (node->type == TYPE_OBJECT) {
        cJsonFree(&(node->value.objVal));
    }
    S_FREE(node);
}

pJsonNode_T cJsonNodeNew(unsigned long kSize, unsigned long vSize, JSONTYPE_E type)
{
    pJsonNode_T n = (pJsonNode_T)malloc(sizeof(JsonNode_T));
    if (n == NULL) {
        perror("malloc node failed\n");
        goto ERR;
    }

    n->key = (char *)malloc(kSize);
    if (n->key == NULL) {
        perror("malloc node->key failed\n");
        goto ERR;
    }

    if (type == TYPE_STRING) {
        n->value.stringVal = (char *)malloc(vSize);
        if (n->value.stringVal == NULL) {
            perror("malloc node->value.stringVal failed\n");
            goto ERR;
        }
    }

    n->next = NULL;
    n->prev = NULL;
    n->type = type;

    return n;

    ERR:
    cJsonNodeFree(n);
    return NULL;
}

void cJsonDel(pJsonObj_T obj, const char *key)
{
    if (obj == NULL) {
        return;
    }

    pJsonNode_T tmp = obj->head;
    while (tmp != NULL) {
        int cmp = strcmp(tmp->key, key);
        /*this is a sorted list, if the key of tmp is larger than key, then tmp->next->key must larger than key*/
        if (cmp > 0) {
            return;
        } else if (cmp == 0) {
            if (tmp->prev != NULL) {
                tmp->prev->next = tmp->next;
            }
            if (tmp->next != NULL) {
                tmp->next->prev = tmp->prev;
            }
            cJsonNodeFree(tmp);
        }
        tmp = tmp->next;
    }
}

void cJsonFree(pJsonObj_T *obj)
{
    pJsonNode_T tmp = (*obj)->head;
    while (tmp != NULL) {
        (*obj)->head = (*obj)->head->next;
        --(*obj)->size;

        if ((*obj)->head != NULL)
            (*obj)->head->prev = NULL;

        cJsonNodeFree(tmp);
        tmp = (*obj)->head;
    }
    S_FREE((*obj)->jsonStr);
    S_FREE(*obj);
}

void cJsonNodeUpdate(pJsonNode_T oldV, pJsonNode_T newV)
{
    printf("update value\n");
    newV->prev = oldV->prev;
    if (oldV->prev != NULL)
        oldV->prev->next = newV;
    newV->next = oldV->next;
    cJsonNodeFree(oldV);
}

/*
 * @description : if base->key larger than value->key, then calling this fucntion.The value
 *                will be inserted before the base;
 */
void cJsonNodeAddFrontAt(pJsonObj_T obj, pJsonNode_T base, pJsonNode_T value)
{
    value->next = base;
    value->prev = base->prev;
    if (base->prev != NULL) {
        base->prev->next = value;
    } else {
        obj->head = value;
    }
    base->prev = value;
}

/*
 * @description : if base->key less than value->key, and (base->next->key larger than value->key or base->next is NULL)
 *                then calling this fucntion. the value will be inserted after the base;
 */
void cJsonNodeAddNextTo(pJsonNode_T base, pJsonNode_T value)
{
    value->next = base->next;
    value->prev = base;
    if (base->next != NULL) {
        base->next->prev = value;
    }
    base->next = value;
}

/*
 * @description : insertion sorting
 */
void cJsonAdd(pJsonObj_T obj, pJsonNode_T value)
{
    if (obj == NULL)
        return;

    if (obj->head == NULL) {
        obj->head = value;
        ++obj->size;
    } else {
        for (pJsonNode_T tmp = obj->head; tmp != NULL; tmp = tmp->next) {
            if (strcmp(tmp->key, value->key) == 0) {
                cJsonNodeUpdate(tmp, value);
                break;
            } else if (strcmp(tmp->key, value->key) < 0
                        && (tmp->next == NULL || strcmp(tmp->next->key, value->key) > 0)) {
                cJsonNodeAddNextTo(tmp, value);
                ++obj->size;
                break;
            } else if (strcmp(tmp->key, value->key) > 0) {
                cJsonNodeAddFrontAt(obj, tmp, value);
                ++obj->size;
                break;
            }
        }
    }
}

void cJsonAddInt(pJsonObj_T obj, const char *key, long value)
{
    if (obj == NULL) {
        return ;
    }

    pJsonNode_T node = cJsonNodeNew(strlen(key)+1, 0, TYPE_INT);
    if (node == NULL) {
        return;
    }

    strcpy(node->key, key);
    node->value.lVal = value;

    cJsonAdd(obj, node);
}

/*
 * @description : if the value is deep copy in this funcion,
 *                  so you should free value if the value is dynamic alloc out of the fucnion.
 */
void cJsonAddString(pJsonObj_T obj, const char *key, const char *value)
{
    if (obj == NULL) {
        return ;
    }

    pJsonNode_T node = cJsonNodeNew(strlen(key)+1, strlen(value)+1, TYPE_STRING);
    if (node == NULL) {
        return;
    }

    strcpy(node->key, key);
    strcpy(node->value.stringVal, value);

    cJsonAdd(obj, node);
}

void cJsonAddBool(pJsonObj_T obj, const char *key, bool value)
{
    if (obj == NULL) {
        return;
    }

    pJsonNode_T node = cJsonNodeNew(strlen(key)+1, 0, TYPE_BOOL);
    if (node == NULL) {
        return;
    }

    strcpy(node->key, key);
    node->value.boolVal = value;

    cJsonAdd(obj, node);
}

void cJsonAddObj(pJsonObj_T obj, const char *key, pJsonObj_T value)
{
    if (obj == NULL) {
        return;
    }
    pJsonNode_T node = cJsonNodeNew(strlen(key)+1, 0, TYPE_OBJECT);
    if (node == NULL) {
        return;
    }

    strcpy(node->key, key);
    node->value.objVal = value;

    cJsonAdd(obj, node);
}

void cJsonPrint(pJsonObj_T obj)
{
    for (pJsonNode_T tmp = obj->head; tmp != NULL; tmp = tmp->next) {
        if (tmp->type == TYPE_INT) {
            printf("key : %s, value : %ld\n", tmp->key, tmp->value.lVal);
        } else if (tmp->type == TYPE_STRING) {
            printf("key : %s, value : %s\n", tmp->key, tmp->value.stringVal);
        } else if (tmp->type == TYPE_BOOL) {
            printf("key : %s, value : %s\n", tmp->key, tmp->value.boolVal ? TRUE_STR : FALSE_STR);
        } else if (tmp->type == TYPE_OBJECT) {
            printf("key : %s, object start\n", tmp->key);
            cJsonPrint(tmp->value.objVal);
            printf("key : %s, object end\n", tmp->key);
        }
    }
}

char *cJsonNodeFormatNum(pJsonNode_T node)
{
    unsigned long len = strlen(KEY_VAL_INT)+strlen(node->key)+MAX_NUM_STR_LEN+1;
    char *str = (char *)malloc(len);
    if (str) {
        snprintf(str, len, KEY_VAL_INT, node->key, node->value.lVal);
    }

    return str;
}

char *cJsonNodeFormatBool(pJsonNode_T node)
{
    unsigned long len = strlen(KEY_VAL_BOOL) + strlen(node->key) + MAX_BOOL_STR_LEN + 1;
    char *str = (char *)malloc(len);
    if (str) {
        snprintf(str, len, KEY_VAL_BOOL, node->key, node->value.boolVal ? TRUE_STR : FALSE_STR);
    }

    return str;
}

char *cJsonNodeFormatString(pJsonNode_T node)
{
    unsigned long len = strlen(KEY_VAL_STR) + strlen(node->key) + strlen(node->value.stringVal)+1;
    char *str = (char *)malloc(len);
    if (str) {
        snprintf(str, len, KEY_VAL_STR, node->key, node->value.stringVal);
    }

    return str;
}

char *cJsonNodeFormatObj(pJsonNode_T node)
{
    char *val = cJsonMashal(node->value.objVal);
    char *str = NULL;
    if (val) {
        unsigned long len = strlen(KEY_VAL_BOOL) + strlen(node->key) + strlen(val)+1;
        str = (char *)malloc(len);
        if (str) {
            snprintf(str, len, KEY_VAL_BOOL, node->key, val);
            S_FREE(val);
        }
    }

    return str;
}

char *cJsonNodeFormat(pJsonNode_T node)
{
    char *s = NULL;
    switch (node->type) {
        case TYPE_INT:
            s = cJsonNodeFormatNum(node);
            break;
        case TYPE_BOOL:
            s = cJsonNodeFormatBool(node);
            break;
        case TYPE_STRING:
            s = cJsonNodeFormatString(node);
            break;
        case TYPE_OBJECT:
            s = cJsonNodeFormatObj(node);
        default:
            break;
    }
    return s;
}

char *cJsonMashal(pJsonObj_T obj)
{
    char *str = (char *)malloc(BASE_STR_SIZE);
    if (str == NULL) {
        return NULL;
    }
    unsigned long strSize = BASE_STR_SIZE;
    memset(str, 0, BASE_STR_SIZE);
    str[0] = OBJ_PRE_FIX;
    char *tmp = str+1;

    for (pJsonNode_T node = obj->head; node != NULL; node = node->next) {
        char *val = cJsonNodeFormat(node);
        if (val == NULL) {
            S_FREE(str);
            return NULL;
        }
        unsigned long len = tmp-str;
        unsigned long valLen = strlen(val);
        if (len + valLen > SHOULD_REALLOC) {
            strSize += (valLen/SHOULD_REALLOC+1)*SHOULD_REALLOC;
            char *rstr = (char *)realloc(str, strSize);
            if (rstr == NULL) {
                S_FREE(str);
                S_FREE(val);
                break;
            } else {
                str = rstr;
                tmp = str+len;
            }
        }
        sprintf(tmp, "%s", val);
        S_FREE(val);
        tmp += valLen;

        if (node->next == NULL) {
            *tmp = OBJ_SUF_FIX;
            *(tmp+1) = '\0';
        } else {
            *tmp = ',';
            ++tmp;
        }
    }

    S_FREE(obj->jsonStr);
    obj->jsonStr = str;

    return obj->jsonStr;
}

pJsonObj_T cJsonParse(const char *text)
{
    if (text == NULL) {
        return NULL;
    }

    ParserStruct_T obj;
    obj.curNode = NULL;
    obj.obj = NULL;
    obj.text = text;
    obj.state = OBJ_STATE_IDLE;

    while (obj.state != OBJ_STATE_DONE) {
        gJsonObjParsers[obj.state](&obj);
    }

    return obj.obj;
}

int cJsonParseObjStateIDLE(pParserStruct_T st)
{
    return 0;
}

int cJsonParseObjStateWFK(pParserStruct_T st)
{

}

int cJsonParseObjStatePKS(pParserStruct_T st)
{

}

int cJsonParseObjStatePKE(pParserStruct_T st)
{

}

int cJsonParseObjStateWFV(pParserStruct_T st)
{

}

int cJsonParseObjStatePVNS(pParserStruct_T st)
{

}

int cJsonParseObjStatePVBS(pParserStruct_T st)
{

}

int cJsonParseObjStatePVSS(pParserStruct_T st)
{

}

int cJsonParseObjStatePVOS(pParserStruct_T st)
{

}

int cJsonParseObjStatePVAS(pParserStruct_T st)
{

}

int cJsonParseObjStatePVE(pParserStruct_T st)
{

}

int cJsonParseObjStateERR(pParserStruct_T st)
{

}

int cJsonParseObjStateSuccess(pParserStruct_T st)
{

}


