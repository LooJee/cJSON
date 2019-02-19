//
// Created by Zer0 on 2019/1/30.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
//    obj->jsonStr = NULL;

    return obj;
}

void cJsonNodeFree(pJsonNode_T *node)
{
    if (node == NULL || *node == NULL) {
        return;
    }

    S_FREE((*node)->key);
    (*node)->next = NULL;
    (*node)->prev = NULL;
    if ((*node)->type == TYPE_STRING) {
        S_FREE((*node)->value.stringVal);
    } else if ((*node)->type == TYPE_OBJECT) {
        cJsonFree(&((*node)->value.objVal));
    }
    S_FREE(*node);
}

/*
 * @description : 创建一个 json 的键值对
 * @in :
 *      kSize : 键的空间
 *      vSize : 值的空间
 *      type : json 数据的类型
 * @out :
 *      成功，则返回创建的对象；
 *      否则，返回 NULL。
 * */
pJsonNode_T cJsonNodeNew(unsigned long kSize, unsigned long vSize, JSONTYPE_E type)
{
    pJsonNode_T n = (pJsonNode_T)malloc(sizeof(JsonNode_T));
    if (n == NULL) {
        perror("malloc node failed\n");
        goto ERR;
    }

    if (kSize == 0) {
        n->key = NULL;
    } else {
        n->key = (char *)malloc(kSize);
        if (n->key == NULL) {
            perror("malloc node->key failed\n");
            goto ERR;
        }
        memset(n->key, 0, kSize);
    }

    if (type == TYPE_STRING) {
        n->value.stringVal = (char *)malloc(vSize);
        if (n->value.stringVal == NULL) {
            perror("malloc node->value.stringVal failed\n");
            goto ERR;
        }
        memset(n->value.stringVal, 0, vSize);
    }

    n->next = NULL;
    n->prev = NULL;
    n->type = type;

    return n;

    ERR:
    cJsonNodeFree(&n);
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
            pJsonNode_T freeNode = tmp;
            tmp = tmp->next;
            cJsonNodeFree(&freeNode);
        } else {
            tmp = tmp->next;
        }
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

        cJsonNodeFree(&tmp);
        tmp = (*obj)->head;
    }
//    S_FREE((*obj)->jsonStr);
    S_FREE(*obj);
}

void cJsonNodeUpdate(pJsonNode_T oldV, pJsonNode_T newV)
{
    printf("update value\n");
    newV->prev = oldV->prev;
    if (oldV->prev != NULL)
        oldV->prev->next = newV;
    newV->next = oldV->next;
    cJsonNodeFree(&oldV);
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

void cJsonAddArray(pJsonObj_T obj, const char *key, pJsonArray_T val)
{
    if (obj == NULL) {
        return;
    }

    pJsonNode_T node = cJsonNodeNew(strlen(key)+1, 0, TYPE_ARRAY);
    if (node == NULL) {
        return;
    }

    strncpy(node->key, key, strlen(key));
    node->value.arrVal = val;

    cJsonAdd(obj, node);
}

void cJsonPrint(pJsonObj_T obj)
{
    if (obj == NULL) {
        printf("null\n");
        return;
    }

    for (pJsonNode_T tmp = obj->head; tmp != NULL; tmp = tmp->next) {
        switch (tmp->type) {
            case TYPE_INT:
                printf("key : %s, value : %ld\n", tmp->key, tmp->value.lVal);
                break;
            case TYPE_STRING:
                printf("key : %s, value : %s\n", tmp->key, tmp->value.stringVal);
                break;
            case TYPE_BOOL:
                printf("key : %s, value : %s\n", tmp->key, tmp->value.boolVal ? TRUE_STR : FALSE_STR);
                break;
            case TYPE_OBJECT:
                printf("--key : %s, object start {\n", tmp->key);
                cJsonPrint(tmp->value.objVal);
                printf("--key : %s, object end }\n", tmp->key);
                break;
            case TYPE_ARRAY:
                printf("array start [\n");
                cJsonArrPrint(tmp->value.arrVal);
                printf("array end ]\n");
                break;
            default:
                break;
        }
    }
}

char *cJsonNodeFormatNum(pJsonNode_T node)
{
    if (node->key == NULL) {
        /*json array, without key*/
        size_t len = MAX_NUM_STR_LEN + 1;
        char *str = (char *)malloc(len);
        if (str) {
            snprintf(str, len, "%ld", node->value.lVal);
        }

        return str;
    } else {
        /*json object, with key*/
        size_t len = strlen(KEY_VAL_INT)+strlen(node->key)+MAX_NUM_STR_LEN+1;
        char *str = (char *)malloc(len);
        if (str) {
            snprintf(str, len, KEY_VAL_INT, node->key, node->value.lVal);
        }

        return str;
    }
}

char *cJsonNodeFormatBool(pJsonNode_T node)
{
    if (node->key == NULL) {
        return (node->value.boolVal) ? TRUE_STR : FALSE_STR;
    } else {
        unsigned long len = strlen(KEY_VAL_BOOL) + strlen(node->key) + MAX_BOOL_STR_LEN + 1;
        char *str = (char *)malloc(len);
        if (str) {
            snprintf(str, len, KEY_VAL_BOOL, node->key, node->value.boolVal ? TRUE_STR : FALSE_STR);
        }

        return str;
    }
}

char *cJsonNodeFormatString(pJsonNode_T node)
{
    if (node->key == NULL) {
        size_t len = strlen(ARR_VAL_STR) + strlen(node->value.stringVal) + 1;
        char *str = (char *)malloc(len);
        if (str) {
            snprintf(str, len, ARR_VAL_STR, node->value.stringVal);
        }
        return str;
    } else {
        size_t len = strlen(KEY_VAL_STR) + strlen(node->key) + strlen(node->value.stringVal)+1;
        char *str = (char *)malloc(len);
        if (str) {
            snprintf(str, len, KEY_VAL_STR, node->key, node->value.stringVal);
        }

        return str;
    }
}

char *cJsonNodeFormatObj(pJsonNode_T node)
{
    if (node->key == NULL) {
        return cJsonMashal(node->value.objVal);
    } else {
        char *val = cJsonMashal(node->value.objVal);
        char *str = NULL;
        if (val) {
            size_t len = strlen(KEY_VAL_BOOL) + strlen(node->key) + strlen(val)+1;
            str = (char *)malloc(len);
            if (str) {
                snprintf(str, len, KEY_VAL_BOOL, node->key, val);
                S_FREE(val);
            }
        }

        return str;
    }
}

char *cJsonNodeFormatArray(pJsonNode_T node)
{
    if (node->key == NULL) {
        return cJsonArrMashal(node->value.arrVal);
    } else {
        char *val = cJsonArrMashal(node->value.arrVal);
        char *str = NULL;
        if (val) {
            size_t len = strlen(KEY_VAL_BOOL) + strlen(node->key) + strlen(val) +1;
            str = (char *)malloc(len);
            if (str) {
                snprintf(str, len, KEY_VAL_BOOL, node->key, val);
                S_FREE(val);
            }
        }

        return str;
    }
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
            break;
        case TYPE_ARRAY:
            s = cJsonNodeFormatArray(node);
            break;
        default:
            break;
    }
    return s;
}

char *cJsonArrMashal(pJsonArray_T arr)
{
    char *str = (char *)malloc(BASE_STR_SIZE);
    if (str == NULL) {
        return str;
    }

    char *tmpstr = str;
    size_t strSize = BASE_STR_SIZE;
    memset(str, 0, BASE_STR_SIZE);
    tmpstr[0] = ARR_PRE_FIX;
    ++tmpstr;

    for (pJsonNode_T tmp = arr->head; tmp != NULL; tmp = tmp->next) {
        char *val = cJsonNodeFormat(tmp);
        if (val == NULL) {
            S_FREE(str);
            return NULL;
        }

        size_t len = tmpstr - str;
        size_t valLen = strlen(val);
        if (len + valLen + SHOULD_REALLOC > strSize) {
            strSize += (valLen/SHOULD_REALLOC+1)*SHOULD_REALLOC;
            char *rstr = (char *)realloc(str, strSize);
            if (rstr == NULL) {
                S_FREE(str);
                S_FREE(val);
                return NULL;
            } else {
                str = rstr;
                tmpstr = str+len;
            }
        }
        sprintf(tmpstr, "%s", val);

        if (tmp->type != TYPE_BOOL) {
            S_FREE(val);
        }
        tmpstr += valLen;

        if (tmp->next != NULL) {
            *tmpstr = ',';
            ++tmpstr;
        }
    }

    *tmpstr = ARR_SUF_FIX;
    *(tmpstr+1) = '\0';

    return str;
}


char *cJsonMashal(pJsonObj_T obj)
{
    char *str = (char *)malloc(BASE_STR_SIZE);
    if (str == NULL) {
        return NULL;
    }
    size_t strSize = BASE_STR_SIZE;
    memset(str, 0, BASE_STR_SIZE);
    str[0] = OBJ_PRE_FIX;
    char *tmp = str+1;

    for (pJsonNode_T node = obj->head; node != NULL; node = node->next) {
        char *val = cJsonNodeFormat(node);
        if (val == NULL) {
            S_FREE(str);
            return NULL;
        }
        size_t len = tmp-str;
        size_t valLen = strlen(val);
        if (len + valLen + SHOULD_REALLOC > strSize) {
            strSize += (valLen/SHOULD_REALLOC+1)*SHOULD_REALLOC;
            char *rstr = (char *)realloc(str, strSize);
            if (rstr == NULL) {
                S_FREE(str);
                S_FREE(val);
                return NULL;
            } else {
                str = rstr;
                tmp = str+len;
            }
        }
        sprintf(tmp, "%s", val);
        S_FREE(val);
        tmp += valLen;

        if (node->next != NULL) {
            *tmp = ',';
            ++tmp;
        }
    }

    *tmp = OBJ_SUF_FIX;
    *(tmp+1) = '\0';

//    S_FREE(obj->jsonStr);
//    obj->jsonStr = str;

    return str;
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

/*
 * @description: 解析json对象字符串的初始状态
 *                  1、当遇到 { 时，状态变为 OBJ_STATE_WAIT_FOR_KEY;
 *                  2、当遇到间隔符时，状态不变化;
 *                  3、其它字符时，状态变为 OBJ_STATE_ERROR;
 * */
int cJsonParseObjStateIDLE(pParserStruct_T st)
{
    if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else if (*st->text == OBJ_PRE_FIX) {
        st->obj = cJsonNew();
        st->state = OBJ_STATE_WAIT_FOR_KEY;
        ++st->text;
    } else {
        st->state = OBJ_STATE_ERROR;
        ++st->text;
    }

    return 0;
}

/*
 * @description : 等待 json key 状态
 *                  1、当遇到字符 “ 时，进入 OBJ_STATE_PARSE_KEY_START
 *                  2、当遇到字符 } 时，进入 OBJ_STATE_SUCCESS（空的 json 对象）
 *                  3、当遇到间隔符时，状态不变
 *                  4、其它字符时，状态变为 OBJ_STATE_ERROR
 * */
int cJsonParseObjStateWFK(pParserStruct_T st)
{
    if (*st->text == STR_PRE_SUF_FIX) {
        st->curNode = (pJsonNode_T)malloc(sizeof(JsonNode_T));
        if (st->curNode == NULL) {
            st->state = OBJ_STATE_ERROR;
        } else {
            memset(st->curNode, 0, sizeof(JsonNode_T));
            st->state = OBJ_STATE_PARSE_KEY_START;
            ++st->text;
        }
    } else if (*st->text == OBJ_SUF_FIX) {
        st->state = OBJ_STATE_SUCCESS;
        ++st->text;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state = OBJ_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析 json key 状态
 *                  1、当遇到字符 " 时，进入 OBJ_STATE_PARSE_KEY_END
 *                  2、当遇到字符 '\0' 时，进入 OBJ_STATE_ERROR
 *                  3、其他字符时，状态不改变
 * */
int cJsonParseObjStatePKS(pParserStruct_T st)
{
    char *strSuffix = strchr(st->text, STR_PRE_SUF_FIX);
    if (strSuffix == NULL) {
        st->state = OBJ_STATE_ERROR;
    } else {
        unsigned long keyLen = strSuffix - st->text;
        st->curNode->key = (char *)malloc(keyLen);
        if (st->curNode->key == NULL) {
            st->state = OBJ_STATE_ERROR;
        } else {
            strncpy(st->curNode->key, st->text, keyLen);
            st->curNode->key[keyLen] = STR_EOF;
            st->text = strSuffix + 1;
            st->state = OBJ_STATE_PARSE_KEY_END;
        }
    }

    return 0;
}

/*
 * @description : 解析 json object key 结束，等待 : 字符
 *                  1、当遇到 : 字符时，状态变为 OBJ_STATE_WAIT_FOR_VAL
 *                  2、当字符为间隔符时，状态不变
 *                  3、当为其它字符时，状态变为 OBJ_STATE_ERROR
 * */
int cJsonParseObjStatePKE(pParserStruct_T st)
{
    char *colon = strchr(st->text, COLON_SYMBOL);
    if (colon == NULL) {
        st->state = OBJ_STATE_ERROR;
    } else {
        st->state = OBJ_STATE_WAIT_FOR_VAL;
        st->text = colon + 1;
    }

    return 0;
}

/*
 * @description : 等待 json object 的值
 *                  1、当字符为 " 时，值可能为 string 类型，状态变为 OBJ_STATE_PARSE_VAL_STR_START
 *                  2、当字符为 数字 时，值可能为数字，状态变为 OBJ_STATE_PARSE_VAL_NUM_START
 *                  3、当字符为 t 或 f 时，值可能为 bool 类型，状态变为 OBJ_STATE_PARSE_VAL_BOOL_START
 *                  4、当字符为 [ 时，值可能为数组类型，状态变为 OBJ_STATE_PARSE_VAL_ARR_START
 *                  5、当字符为 { 时，值可能为 json 对象，状态变为 OBJ_STATE_PARSE_VAL_OBJ_START
 *                  6、当为间隔符时，状态不变
 *                  7、当为其它字符时，状态变为 OBJ_STATE_ERROR
 * */
int cJsonParseObjStateWFV(pParserStruct_T st)
{
    if (*st->text == STR_PRE_SUF_FIX) {
        st->state = OBJ_STATE_PARSE_VAL_STR_START;
        ++st->text;
    } else if (isdigit(*st->text)) {
        st->state = OBJ_STATE_PARSE_VAL_NUM_START;
    } else if (IS_BOOL_PREFIX(*st->text)) {
        st->state = OBJ_STATE_PARSE_VAL_BOOL_START;
    } else if (*st->text == ARR_PRE_FIX) {
        st->state = OBJ_STATE_PARSE_VAL_ARR_START;
    } else if (*st->text == OBJ_PRE_FIX) {
        st->state = OBJ_STATE_PARSE_VAL_OBJ_START;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state = OBJ_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析 number value 状态
 *                  1、当字符为间隔符，数字值可能结束，状态变为 OBJ_STATE_PARSE_VAL_END
 *                  2、当字符为数字，状态不变
 *                  3、当字符为 , 时，状态变为 OBJ_STATE_WAIT_FOR_KEY
 *                  4、当为其它字符时，状态变为 OBJ_STATE_ERROR
 * */
int cJsonParseObjStatePVNS(pParserStruct_T st)
{
    if (IS_GAP_SYMBOL(*st->text)) {
        st->state = OBJ_STATE_PARSE_VAL_END;
        cJsonAdd(st->obj, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else if (isdigit(*st->text)) {
        //字符串 to 数字
        st->curNode->value.lVal = st->curNode->value.lVal * 10 + (*st->text - '0');
        ++st->text;
    } else if (*st->text == COMMA_SYMBOL) {
        st->state = OBJ_STATE_WAIT_FOR_KEY;
        cJsonAdd(st->obj, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else if (*st->text == OBJ_SUF_FIX) {
        st->state = OBJ_STATE_SUCCESS;
        cJsonAdd(st->obj, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else {
        st->state = OBJ_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析 bool value 状态
 *                  1、当字符串和 true 或 false 相等时，值确认为 bool类型，状态变为 OBJ_STATE_PARSE_VAL_END
 *                  2、否则，状态变为 OBJ_STATE_ERROR
 * */
int cJsonParseObjStatePVBS(pParserStruct_T st)
{
    if (strncmp(st->text, TRUE_STR, TRUE_STR_LEN) == 0) {
        st->state = OBJ_STATE_PARSE_VAL_END;
        st->curNode->value.boolVal = true;
        st->curNode->type = TYPE_BOOL;
        cJsonAdd(st->obj, st->curNode);
        st->curNode = NULL;
        st->text += TRUE_STR_LEN;
    } else if (strncmp(st->text, FALSE_STR, FALSE_STR_LEN) == 0) {
        st->state = OBJ_STATE_PARSE_VAL_END;
        st->curNode->type = TYPE_BOOL;
        st->curNode->value.boolVal = false;
        cJsonAdd(st->obj, st->curNode);
        st->curNode = NULL;
        st->text += FALSE_STR_LEN;
    } else {
        st->state = OBJ_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析 string value 状态
 *                  1、当字符为 " 时，解析字符串结束，状态变为 OBJ_STATE_PARSE_VAL_END
 *                  2、当字符为 '\0' 时， 状态变为 OBJ_STATE_ERROR
 *                  3、当为其它字符时，状态不变。
 * */
int cJsonParseObjStatePVSS(pParserStruct_T st)
{
    char *strSuffix = strchr(st->text, STR_PRE_SUF_FIX);
    if (strSuffix == NULL) {
        st->state = OBJ_STATE_ERROR;
    } else {
        size_t valLen = strSuffix - st->text;
        st->curNode->value.stringVal = (char *)malloc(valLen);
        if (st->curNode->value.stringVal == NULL) {
            st->state = OBJ_STATE_ERROR;
        } else {
            strncpy(st->curNode->value.stringVal, st->text, valLen);
            st->curNode->value.stringVal[valLen] = STR_EOF;
            st->curNode->type = TYPE_STRING;
            cJsonAdd(st->obj, st->curNode);
            st->curNode = NULL;
            st->text = strSuffix + 1;
            st->state = OBJ_STATE_PARSE_VAL_END;
        }
    }

    return 0;
}

/*
 * @description : 解析 object value 状态
 *                  1、创建新的 json object，通过状态机来解析，然后根据解析结果来判断解析是否成功；
 *                  解析成功，则状态转为 OBJ_STATE_PARSE_VAL_END;否则，状态转为
 * */
int cJsonParseObjStatePVOS(pParserStruct_T st)
{
    ParserStruct_T subSt;
    subSt.text = st->text;
    subSt.state = OBJ_STATE_IDLE;
    subSt.obj = NULL;
    subSt.curNode = (pJsonNode_T)malloc(sizeof(JsonNode_T));

    if (subSt.curNode == NULL) {
        st->state = OBJ_STATE_ERROR;
    } else {
        while (subSt.state != OBJ_STATE_DONE) {
            gJsonObjParsers[subSt.state](&subSt);
        }

        if (subSt.ret == 0) {
            st->text = subSt.text;
            st->curNode->value.objVal = subSt.obj;
            st->curNode->type = TYPE_OBJECT;
            cJsonAdd(st->obj, st->curNode);
            st->curNode = NULL;
            st->state = OBJ_STATE_PARSE_VAL_END;
        } else {
            st->state = OBJ_STATE_ERROR;
        }
    }


    return 0;
}

int cJsonParseObjStatePVAS(pParserStruct_T st)
{

    return 0;
}

/*
 * @description : 解析 value 成功状态
 *                  1、当遇到 } 时，已经提取处完整的 json 对象，状态设置为 OBJ_STATE_SUCCESS
 *                  2、当遇到 , 时，说明可能有其它的 json 键值对存在，状态设置为 OBJ_STATE_WAIT_FOR_KEY
 *                  3、当为间隔符时，状态不变化
 *                  4、当为其它字符时，状态设置为 OBJ_STATE_ERROR
 * */
int cJsonParseObjStatePVE(pParserStruct_T st)
{
    if (*st->text == '}') {
        st->state = OBJ_STATE_SUCCESS;
        ++st->text;
    } else if (*st->text == ',') {
        st->state = OBJ_STATE_WAIT_FOR_KEY;
        ++st->text;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state = OBJ_STATE_ERROR;
    }

    return 0;
}

int cJsonParseObjStateERR(pParserStruct_T st)
{
    cJsonFree(&(st->obj));
    cJsonNodeFree(&(st->curNode));
    st->ret = -1;
    st->state = OBJ_STATE_DONE;

    return 0;
}

int cJsonParseObjStateSuccess(pParserStruct_T st)
{
    st->ret = 0;
    st->state = OBJ_STATE_DONE;

    return 0;
}

pJsonArray_T cJsonArrNew()
{
    pJsonArray_T arr = (pJsonArray_T)malloc(sizeof(JsonArray_T));
    if (arr == NULL) {
        perror("malloc array failed\n");
        return NULL;
    }

    arr->size = 0;
    arr->head = NULL;
    arr->end = NULL;

    return arr;
}

void cJsonArrAppend(pJsonArray_T arr, pJsonNode_T val)
{
    if (arr == NULL || val == NULL) {
        return;
    }

    if (arr->head == NULL && arr->end == NULL) {
        arr->head = val;
        arr->end = val;
    } else if (arr->head == arr->end) {
        arr->head->next = val;
        val->prev = arr->head;
        arr->end = val;
    } else {
        arr->end->next = val;
        val->prev = arr->end;
        arr->end = val;
    }

    ++arr->size;
}

void cJsonArrAppendNum(pJsonArray_T arr, long val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_INT);
    item->value.lVal = val;
    cJsonArrAppend(arr, item);
}

void cJsonArrAppendString(pJsonArray_T arr, const char *val)
{
    pJsonNode_T item = cJsonNodeNew(0, strlen(val)+1, TYPE_STRING);
    strncpy(item->value.stringVal, val, strlen(val));
    cJsonArrAppend(arr, item);
}

void cJsonArrAppendBool(pJsonArray_T arr, bool val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_BOOL);
    item->value.boolVal = val;
    cJsonArrAppend(arr, item);
}

void cJsonArrAppendObj(pJsonArray_T arr, pJsonObj_T val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_OBJECT);
    item->value.objVal = val;
    cJsonArrAppend(arr, item);
}

void cJsonArrAppendArr(pJsonArray_T arr, pJsonArray_T val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_ARRAY);
    item->value.arrVal = val;
    cJsonArrAppend(arr, item);
}

void cJsonArrInsertAt(pJsonArray_T arr, size_t idx, pJsonNode_T val)
{
    if (arr == NULL || val == NULL) {
        return;
    }

    pJsonNode_T tmp = NULL;

    if (idx >= arr->size) {
        cJsonArrAppend(arr, val);
        return;
    } else if (idx <= arr->size/2) {
        tmp = arr->head;
        for (size_t i = 0; i <= idx; ++i, tmp = tmp->next){
            if (i == idx)
                break;
        }
    } else if (idx > arr->size/2) {
        tmp = arr->end;
        for (size_t i = arr->size; i > idx; --i, tmp = tmp->prev) {
            if (i == idx)
                break;
        }
    } else {
        return;
    }

    val->next = tmp->next;
    if (tmp->prev) {
        tmp->prev->next = val;
    } else {
        arr->head = val;
    }
    val->prev = tmp->prev;

    cJsonNodeFree(&tmp);
}
void cJsonArrInsertNumAt(pJsonArray_T arr, size_t idx, long val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_INT);
    item->value.lVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertStringAt(pJsonArray_T arr, size_t idx, const char *val)
{
    pJsonNode_T item = cJsonNodeNew(0, strlen(val)+1, TYPE_STRING);
    strncpy(item->value.stringVal, val, strlen(val));
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertBoolAt(pJsonArray_T arr, size_t idx, bool val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_BOOL);
    item->value.boolVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertObjAt(pJsonArray_T arr, size_t idx, pJsonObj_T val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_OBJECT);
    item->value.objVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertArrAt(pJsonArray_T arr, size_t idx, pJsonArray_T val)
{
    pJsonNode_T item= cJsonNodeNew(0, 0, TYPE_ARRAY);
    item->value.arrVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrPrint(pJsonArray_T arr)
{
    if (arr->size == 0) {
        printf("[]");
        return;
    }

    for (pJsonNode_T tmp = arr->head; tmp != NULL; tmp = tmp->next) {
        switch (tmp->type) {
            case TYPE_INT:
                printf("key : %s, value : %ld\n", tmp->key, tmp->value.lVal);
                break;
            case TYPE_STRING:
                printf("key : %s, value : %s\n", tmp->key, tmp->value.stringVal);
                break;
            case TYPE_BOOL:
                printf("key : %s, value : %s\n", tmp->key, tmp->value.boolVal ? TRUE_STR : FALSE_STR);
                break;
            case TYPE_OBJECT:
                printf("--key : %s, object start {\n", tmp->key);
                cJsonPrint(tmp->value.objVal);
                printf("--key : %s, object end }\n", tmp->key);
                break;
            case TYPE_ARRAY:
                printf("--key : %s, array start [\n", tmp->key);
                cJsonArrPrint(tmp->value.arrVal);
                printf("--key : %s, array end ]\n", tmp->key);
                break;
            default:
                break;
        }
    }
}
