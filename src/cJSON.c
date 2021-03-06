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

/*
 *
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
 * */
int cJsonParseArrStateIDLE(pParserStruct_T st);
int cJsonParseArrStateWFV(pParserStruct_T st);
int cJsonParseArrStatePNS(pParserStruct_T st);
int cJsonParseArrStatePBS(pParserStruct_T st);
int cJsonParseArrStatePSS(pParserStruct_T st);
int cJsonParseArrStatePOS(pParserStruct_T st);
int cJsonParseArrStatePAS(pParserStruct_T st);
int cJsonParseArrStatePVE(pParserStruct_T st);
int cJsonParseArrStateErr(pParserStruct_T st);
int cJsonParseArrStateSuc(pParserStruct_T st);

cJsonParserFunc gJsonArrParsers[] = {
        cJsonParseArrStateIDLE,
        cJsonParseArrStateWFV,
        cJsonParseArrStatePNS,
        cJsonParseArrStatePBS,
        cJsonParseArrStatePSS,
        cJsonParseArrStatePOS,
        cJsonParseArrStatePAS,
        cJsonParseArrStatePVE,
        cJsonParseArrStateErr,
        cJsonParseArrStateSuc
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
    } else if ((*node)->type == TYPE_ARRAY) {
        cJsonArrFree(&(*node)->value.arrVal);
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
 * *//**/
pJsonNode_T cJsonNodeNew(unsigned long kSize, unsigned long vSize, JSONTYPE_E type)
{
    pJsonNode_T n = (pJsonNode_T)malloc(sizeof(JsonNode_T));
    if (n == NULL) {
        perror("malloc node failed\n");
        goto ERR;
    }

    memset(n, 0, sizeof(JsonNode_T));

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
        if (vSize == 0) {
            n->value.stringVal = NULL;
        } else {
            n->value.stringVal = (char *)malloc(vSize);
            if (n->value.stringVal == NULL) {
                perror("malloc node->value.stringVal failed\n");
                goto ERR;
            }
            memset(n->value.stringVal, 0, vSize);
        }
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
        /*json array, without key*/
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
        /*json array, without key*/
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
        /*json array, without key*/
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
        /*json array, without key*/
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
    if (arr == NULL) {
        return NULL;
    }

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

/*
 * @description : 将json对象转为字符串
 * */
char *cJsonMashal(pJsonObj_T obj)
{
    if (obj == NULL) {
        return NULL;
    }

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

    return str;
}

pJsonObj_T cJsonParse(const char *text)
{
    if (text == NULL) {
        return NULL;
    }

    ParserStruct_T obj;
    obj.curNode = NULL;
    obj.json.obj = NULL;
    obj.text = text;
    obj.state.obj_state = OBJ_STATE_IDLE;
    obj.isSubParser = false;

    while (obj.state.obj_state != OBJ_STATE_DONE) {
        gJsonObjParsers[obj.state.obj_state](&obj);
    }

    return obj.json.obj;
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
        st->json.obj = cJsonNew();
        st->state.obj_state = OBJ_STATE_WAIT_FOR_KEY;
        ++st->text;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
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
            st->state.obj_state = OBJ_STATE_ERROR;
        } else {
            memset(st->curNode, 0, sizeof(JsonNode_T));
            st->state.obj_state = OBJ_STATE_PARSE_KEY_START;
            ++st->text;
        }
    } else if (*st->text == OBJ_SUF_FIX) {
        st->state.obj_state = OBJ_STATE_SUCCESS;
        ++st->text;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
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
        st->state.obj_state = OBJ_STATE_ERROR;
    } else {
        unsigned long keyLen = strSuffix - st->text;
        st->curNode->key = (char *)malloc(keyLen);
        if (st->curNode->key == NULL) {
            st->state.obj_state = OBJ_STATE_ERROR;
        } else {
            strncpy(st->curNode->key, st->text, keyLen);
            st->curNode->key[keyLen] = STR_EOF;
            st->text = strSuffix + 1;
            st->state.obj_state = OBJ_STATE_PARSE_KEY_END;
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
        st->state.obj_state = OBJ_STATE_ERROR;
    } else {
        st->state.obj_state = OBJ_STATE_WAIT_FOR_VAL;
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
        st->state.obj_state = OBJ_STATE_PARSE_VAL_STR_START;
        ++st->text;
        st->curNode->type = TYPE_STRING;
    } else if (isdigit(*st->text)) {
        st->state.obj_state = OBJ_STATE_PARSE_VAL_NUM_START;
        st->curNode->type = TYPE_INT;
    } else if (IS_BOOL_PREFIX(*st->text)) {
        st->state.obj_state = OBJ_STATE_PARSE_VAL_BOOL_START;
        st->curNode->type = TYPE_BOOL;
    } else if (*st->text == ARR_PRE_FIX) {
        st->state.obj_state = OBJ_STATE_PARSE_VAL_ARR_START;
        st->curNode->type = TYPE_ARRAY;
    } else if (*st->text == OBJ_PRE_FIX) {
        st->state.obj_state = OBJ_STATE_PARSE_VAL_OBJ_START;
        st->curNode->type = TYPE_OBJECT;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
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
        st->state.obj_state = OBJ_STATE_PARSE_VAL_END;
//        st->curNode->type = TYPE_INT;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else if (isdigit(*st->text)) {
        //字符串 to 数字
        st->curNode->value.lVal = st->curNode->value.lVal * 10 + (*st->text - '0');
        ++st->text;
    } else if (*st->text == COMMA_SYMBOL) {
        st->state.obj_state = OBJ_STATE_WAIT_FOR_KEY;
//        st->curNode->type = TYPE_INT;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else if (*st->text == OBJ_SUF_FIX) {
        st->state.obj_state = OBJ_STATE_SUCCESS;
//        st->curNode->type = TYPE_INT;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
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
        st->state.obj_state = OBJ_STATE_PARSE_VAL_END;
        st->curNode->value.boolVal = true;
//        st->curNode->type = TYPE_BOOL;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        st->text += TRUE_STR_LEN;
    } else if (strncmp(st->text, FALSE_STR, FALSE_STR_LEN) == 0) {
        st->state.obj_state = OBJ_STATE_PARSE_VAL_END;
//        st->curNode->type = TYPE_BOOL;
        st->curNode->value.boolVal = false;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        st->text += FALSE_STR_LEN;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
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
        st->state.obj_state = OBJ_STATE_ERROR;
    } else {
        size_t valLen = strSuffix - st->text;
        st->curNode->value.stringVal = (char *)malloc(valLen);
        if (st->curNode->value.stringVal == NULL) {
            st->state.obj_state = OBJ_STATE_ERROR;
        } else {
            strncpy(st->curNode->value.stringVal, st->text, valLen);
            st->curNode->value.stringVal[valLen] = STR_EOF;
//            st->curNode->type = TYPE_STRING;
            cJsonAdd(st->json.obj, st->curNode);
            st->curNode = NULL;
            st->text = strSuffix + 1;
            st->state.obj_state = OBJ_STATE_PARSE_VAL_END;
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
    subSt.state.obj_state = OBJ_STATE_IDLE;
    subSt.json.obj = NULL;
    subSt.curNode = NULL;//(pJsonNode_T)malloc(sizeof(JsonNode_T));
    subSt.isSubParser = true;

    while (subSt.state.obj_state != OBJ_STATE_DONE) {
        gJsonObjParsers[subSt.state.obj_state](&subSt);
    }

    if (subSt.ret == 0) {
        st->text = subSt.text;
        st->curNode->value.objVal = subSt.json.obj;
//        st->curNode->type = TYPE_OBJECT;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        st->state.obj_state = OBJ_STATE_PARSE_VAL_END;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
    }


    return 0;
}

int cJsonParseObjStatePVAS(pParserStruct_T st)
{
    ParserStruct_T subSt;
    subSt.text = st->text;
    subSt.state.arr_state = ARR_STATE_IDLE;
    subSt.ret = 0;
    subSt.json.arr = NULL;
    subSt.curNode = NULL;
    subSt.isSubParser = true;

    while (subSt.state.arr_state != ARR_STATE_DONE) {
        gJsonArrParsers[subSt.state.arr_state](&subSt);
    }

    if (subSt.ret == 0) {
        st->curNode->value.arrVal = subSt.json.arr;
//        st->curNode->type = TYPE_ARRAY;
        st->text = subSt.text;
        cJsonAdd(st->json.obj, st->curNode);
        st->curNode = NULL;
        st->state.obj_state = OBJ_STATE_PARSE_VAL_END;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
    }

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
        st->state.obj_state = OBJ_STATE_SUCCESS;
        ++st->text;
    } else if (*st->text == ',') {
        st->state.obj_state = OBJ_STATE_WAIT_FOR_KEY;
        ++st->text;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state.obj_state = OBJ_STATE_ERROR;
    }

    return 0;
}

int cJsonParseObjStateERR(pParserStruct_T st)
{
    cJsonFree(&(st->json.obj));
    cJsonNodeFree(&(st->curNode));
    st->ret = -1;
    st->state.obj_state = OBJ_STATE_DONE;

    return 0;
}

/*
 * @description : 解析出完整的 json 对象后
 *                  1、如果解析的
 * */
int cJsonParseObjStateSuccess(pParserStruct_T st)
{
    if(!st->isSubParser) {
        while (*st->text != STR_EOF) {
            if (!IS_GAP_SYMBOL(*st->text)) {
                st->state.obj_state = OBJ_STATE_ERROR;
                return 0;
            }
            ++st->text;
        }
    }

    st->ret = 0;
    st->state.obj_state = OBJ_STATE_DONE;
    cJsonNodeFree(&(st->curNode));

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

/*
 * @description : get node from array at idx
 * */
pJsonNode_T cJsonArrAt(pJsonArray_T arr, size_t idx)
{
    pJsonNode_T node = NULL;

    if (arr != NULL && idx < arr->size) {
        size_t mid = arr->size/2;
        if (idx <= mid) {
            node = arr->head;
            for (size_t i = 0; i <= mid; ++i) {
                if (i == idx) {
                    break;
                }
                node = node->next;
            }
        } else {
            node = arr->end;
            for (size_t i = arr->size-1; i > mid; --i) {
                if (i == idx) {
                    break;
                }
                node = node->prev;
            }
        }
    }

    return node;
}

void cJsonArrDel(pJsonArray_T arr, size_t idx)
{
    pJsonNode_T node = cJsonArrAt(arr, idx);
    if (node == NULL) {
        return;
    }

    if (node->prev == NULL) {
        arr->head = node->next;
        node->next->prev = NULL;
    } else if (node->next == NULL) {
        arr->end = node->prev;
        node->prev->next = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    --arr->size;

    cJsonNodeFree(&node);
}

void cJsonArrFree(pJsonArray_T *arr)
{
    if (arr == NULL || (*arr) == NULL) {
        return;
    }

    pJsonNode_T node = (*arr)->head;

    while (node != NULL) {
        pJsonNode_T tmp = node->next;
        cJsonNodeFree(&node);
        node = tmp;
        --(*arr)->size;
    }
    (*arr)->head = NULL;
    (*arr)->end = NULL;

    S_FREE(*arr);
//    *arr = NULL;
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

    pJsonNode_T node = cJsonArrAt(arr, idx);
    if (node == NULL) {
        cJsonArrAppend(arr, val);
    } else {
        if (node->prev == NULL) {
            arr->head = val;
        } else {
            node->prev->next = val;
        }
        val->next = node->next;
        node->prev = val;
    }
}

void cJsonArrInsertNumAt(pJsonArray_T arr, size_t idx, long val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_INT);
    if (item == NULL)
        return;
    item->value.lVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertStringAt(pJsonArray_T arr, size_t idx, const char *val)
{
    pJsonNode_T item = cJsonNodeNew(0, strlen(val)+1, TYPE_STRING);
    if (item == NULL)
        return;

    strncpy(item->value.stringVal, val, strlen(val));
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertBoolAt(pJsonArray_T arr, size_t idx, bool val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_BOOL);
    if (item == NULL)
        return;

    item->value.boolVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertObjAt(pJsonArray_T arr, size_t idx, pJsonObj_T val)
{
    pJsonNode_T item = cJsonNodeNew(0, 0, TYPE_OBJECT);
    if (item == NULL)
        return;

    item->value.objVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrInsertArrAt(pJsonArray_T arr, size_t idx, pJsonArray_T val)
{
    pJsonNode_T item= cJsonNodeNew(0, 0, TYPE_ARRAY);
    if (item == NULL)
        return;

    item->value.arrVal = val;
    cJsonArrInsertAt(arr, idx, item);
}

void cJsonArrReplaceAt(pJsonArray_T arr, size_t idx, pJsonNode_T val)
{
    if (arr == NULL || val == NULL) {
        return;
    }

    pJsonNode_T tmp = cJsonArrAt(arr, idx);
    if (tmp == NULL) {
        return;
    } else {
        if (tmp->prev == NULL) {
            arr->head = val;
        } else {
            tmp->prev->next = val;
        }

        if (tmp->next == NULL) {
            arr->end = val;
        } else {
            tmp->next->prev = val;
        }

        val->prev = tmp->prev;
        val->next = tmp->next;
        cJsonNodeFree(&tmp);
    }
}

void cJsonArrReplaceNumAt(pJsonArray_T arr, size_t idx, long val)
{
    pJsonNode_T n = cJsonNodeNew(0, 0, TYPE_INT);
    if (n == NULL)
        return;

    n->value.lVal = val;

    cJsonArrReplaceAt(arr, idx, n);
}

void cJsonArrReplaceStringAt(pJsonArray_T arr, size_t idx, const char *val)
{
    pJsonNode_T n = cJsonNodeNew(0, strlen(val)+1, TYPE_STRING);
    if (n == NULL)
        return;

    strncpy(n->value.stringVal, val, strlen(val));

    cJsonArrReplaceAt(arr, idx, n);
}

void cJsonArrReplaceBoolAt(pJsonArray_T arr, size_t idx, bool val)
{
    pJsonNode_T n = cJsonNodeNew(0, 0, TYPE_BOOL);
    if (n == NULL)
        return;

    n->value.boolVal = val;

    cJsonArrReplaceAt(arr, idx, n);
}

void cJsonArrReplaceObjAt(pJsonArray_T arr, size_t idx, pJsonObj_T val)
{
    pJsonNode_T n = cJsonNodeNew(0, 0, TYPE_OBJECT);
    if (n == NULL)
        return;

    n->value.objVal = val;

    cJsonArrReplaceAt(arr, idx, n);
}

void cJsonArrReplaceArrAt(pJsonArray_T arr, size_t idx, pJsonArray_T val)
{
    pJsonNode_T n = cJsonNodeNew(0, 0, TYPE_ARRAY);
    if (n == NULL)
        return;

    n->value.arrVal = val;

    cJsonArrReplaceAt(arr, idx, n);
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


pJsonArray_T cJsonArrParse(const char *text)
{
    ParserStruct_T st;
    st.json.arr = NULL;
    st.curNode = NULL;
    st.text = text;
    st.ret = 0;
    st.state.arr_state = ARR_STATE_IDLE;
    st.isSubParser = false;

    while (st.state.arr_state != ARR_STATE_DONE) {
        gJsonArrParsers[st.state.arr_state](&st);
    }

    return st.json.arr;
}

/*
 * @description : 开始解析 json array
 *                  1、当遇到间隔符时，不改变状态
 *                  2、当遇到 [ 时，状态变为 ARR_STATE_WAIT_FOR_VAL
 *                  3、当为其它字符时，状态变为 ARR_STATE_ERROR
 * */
int cJsonParseArrStateIDLE(pParserStruct_T st)
{
    if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else if (*st->text == ARR_PRE_FIX) {
        ++st->text;
        st->json.arr = cJsonArrNew();
        if (st->json.arr == NULL) {
            st->state.arr_state = ARR_STATE_ERROR;
        } else {
            st->state.arr_state = ARR_STATE_WAIT_FOR_VAL;
        }
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 等待 json array 的值
 *                  1、当遇到 " 时，值可能为字符串，状态变为 ARR_STATE_PARSE_STR_START
 *                  2、当遇到 { 时，值可能为 json 对象，状态变为 ARR_STATE_PARSE_OBJ_START
 *                  3、当遇到 [ 时，值可能为 json array，状态变为 ARR_STATE_PARSE_ARR_START
 *                  4、当遇到 t 或 f 时，值可能为 bool，状态变为 ARR_STATE_PARSE_BOOL_START
 *                  5、当遇到数字时，值可能为数字，状态变为 ARR_STATE_PARSE_NUM_START
 *                  6、当遇到间隔符时，状态不变
 *                  7、其它字符，状态变为 ARR_STATE_ERROR
 * */
int cJsonParseArrStateWFV(pParserStruct_T st)
{
    JSONTYPE_E valtype = TYPE_MAX;
    if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else if (*st->text == STR_PRE_SUF_FIX) {
        ++st->text;
        valtype = TYPE_STRING;
        st->state.arr_state = ARR_STATE_PARSE_STR_START;
    } else if (*st->text == OBJ_PRE_FIX) {
        valtype = TYPE_OBJECT;
        st->state.arr_state = ARR_STATE_PARSE_OBJ_START;
    } else if (isdigit(*st->text)) {
        valtype = TYPE_INT;
        st->state.arr_state = ARR_STATE_PARSE_NUM_START;
    } else if (*st->text == ARR_PRE_FIX) {
        valtype = TYPE_ARRAY;
        st->state.arr_state = ARR_STATE_PARSE_ARR_START;
    } else if (IS_BOOL_PREFIX(*st->text)) {
        valtype = TYPE_BOOL;
        st->state.arr_state = ARR_STATE_PARSE_BOOL_START;
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }

    if (valtype != TYPE_MAX) {
        st->curNode = cJsonNodeNew(0, 0, valtype);
        if (st->curNode == NULL) {
            st->state.arr_state = ARR_STATE_ERROR;
        }
    }

    return 0;
}

/*
 * @description : 解析可能为数字的值
 *              1、当字符为数字时，状态不变
 *              2、当字符为 , 时，状态变为 ARR_STATE_WAIT_FOR_VAL
 *              3、当字符为 ] 时，状态变为 ARR_STATE_SUCCESS
 *              4、当为间隔符时，状态变为 ARR_STATE_PARSE_VAL_END
 *              5、当为其它字符时，状态变为 ARR_STATE_ERROR
 * */
int cJsonParseArrStatePNS(pParserStruct_T st)
{
    if (isdigit(*st->text)) {
        st->curNode->value.lVal = st->curNode->value.lVal * 10 + *st->text - '0';
        ++st->text;
    } else if (*st->text == COMMA_SYMBOL) {
        st->state.arr_state = ARR_STATE_WAIT_FOR_VAL;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else if (*st->text == ARR_SUF_FIX) {
        st->state.arr_state = ARR_STATE_SUCCESS;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        st->state.arr_state = ARR_STATE_PARSE_VAL_END;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
        ++st->text;
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析可能为 bool 类型的值
 *                  1、如果值和 true 相等或值和 false 相等，则值为 bool 类型，状态变为 ARR_STATE_PARSE_VAL_END
 *                  2、否则，状态变为 ARR_STATE_ERROR
 * */
int cJsonParseArrStatePBS(pParserStruct_T st)
{
    if (strncmp(st->text, TRUE_STR, TRUE_STR_LEN) == 0) {
        st->curNode->value.boolVal = true;
        st->state.arr_state = ARR_STATE_PARSE_VAL_END;
        st->text += TRUE_STR_LEN;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
        st->curNode = NULL;
    } else if (strncmp(st->text, FALSE_STR, FALSE_STR_LEN) == 0) {
        st->curNode->value.boolVal = false;
        st->state.arr_state = ARR_STATE_PARSE_VAL_END;
        st->text += FALSE_STR_LEN;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }
    return 0;
}

/*
 * @description : 解析可能为字符串的值
 *                  1、当字符为 “ 时，值为字符串，状态转为 ARR_STATE_PARSE_VAL_END
 *                  2、当字符为 '\0'时，状态状态 ARR_STATE_ERROR
 *                  3、当为其它字符时，不改变状态
 * */
int cJsonParseArrStatePSS(pParserStruct_T st)
{
    char *suffix = strchr(st->text, STR_PRE_SUF_FIX);
    if (suffix == NULL) {
        st->state.arr_state = ARR_STATE_ERROR;
    } else {
        size_t len = suffix - st->text;
        st->curNode->value.stringVal = (char *)malloc(len+1);
        if (st->curNode->value.stringVal == NULL) {
            st->state.arr_state = ARR_STATE_ERROR;
        } else {
            strncpy(st->curNode->value.stringVal, st->text, len);
            st->curNode->value.stringVal[len] = STR_EOF;
            st->text += len+1;
            st->state.arr_state = ARR_STATE_PARSE_VAL_END;
            cJsonArrAppend(st->json.arr, st->curNode);
            st->curNode = NULL;
        }
    }

    return 0;
}

/*
 * @description : 解析可能为 json 对象的值
 *                  1、当调用 json parse 成功时，状态转为 ARR_STATE_PARSE_VAL_END
 *                  2、否则，状态转为 ARR_STATE_ERROR
 * */
int cJsonParseArrStatePOS(pParserStruct_T st)
{
    ParserStruct_T subSt;
    subSt.isSubParser = true;
    subSt.text = st->text;
    subSt.state.obj_state = OBJ_STATE_IDLE;
    subSt.ret = 0;
    subSt.json.obj = NULL;
    subSt.curNode = NULL;

    while (subSt.state.obj_state != OBJ_STATE_DONE) {
        gJsonObjParsers[subSt.state.obj_state](&subSt);
    }

    if (subSt.ret == 0) {
        st->curNode->value.objVal = subSt.json.obj;
        st->state.arr_state = ARR_STATE_PARSE_VAL_END;
        st->text = subSt.text;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析可能为 json array 的值
 *                  调用 json array 解析状态机成功时，状态变为 ARR_STATE_PARSE_VAL_END
 *                  否则，状态变为 ARR_STATE_ERROR
 * */
int cJsonParseArrStatePAS(pParserStruct_T st)
{

    ParserStruct_T subSt;
    subSt.text = st->text;
    subSt.state.arr_state = ARR_STATE_IDLE;
    subSt.ret = 0;
    subSt.json.arr = NULL;
    subSt.curNode = NULL;
    subSt.isSubParser = true;

    while (subSt.state.arr_state != ARR_STATE_DONE) {
        gJsonArrParsers[subSt.state.arr_state](&subSt);
    }

    if (subSt.ret == 0) {
        st->curNode->value.arrVal = subSt.json.arr;
        st->curNode->type = TYPE_ARRAY;
        st->state.arr_state = ARR_STATE_PARSE_VAL_END;
        st->text = subSt.text;
        cJsonArrAppend(st->json.arr, st->curNode);
        st->curNode = NULL;
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析 json array value 成功
 *                  1、当字符为 , 时，状态变为 ARR_STATE_WAIT_FOR_VAL
 *                  2、当字符为 ] 时，状态变为 ARR_STATE_SUCCESS
 *                  3、当为间隔符时，状态不变
 *                  4、当为其它字符时，状态变为 ARR_STATE_ERROR
 * */
int cJsonParseArrStatePVE(pParserStruct_T st)
{
    if (*st->text == COMMA_SYMBOL) {
        st->state.arr_state = ARR_STATE_WAIT_FOR_VAL;
        ++st->text;
    } else if (*st->text == ARR_SUF_FIX) {
        st->state.arr_state = ARR_STATE_SUCCESS;
        ++st->text;
    } else if (IS_GAP_SYMBOL(*st->text)) {
        ++st->text;
    } else {
        st->state.arr_state = ARR_STATE_ERROR;
    }

    return 0;
}

/*
 * @description : 解析 json array 失败
 *                  释放资源
 * */
int cJsonParseArrStateErr(pParserStruct_T st)
{
    cJsonArrFree(&st->json.arr);
    cJsonNodeFree(&st->curNode);
    st->ret = -1;
    st->state.arr_state = ARR_STATE_DONE;

    return 0;
}

/*
 * @description : 解析 json array 成功
 * */
int cJsonParseArrStateSuc(pParserStruct_T st)
{
    if (!st->isSubParser) {
        while (*st->text != STR_EOF) {
            if (!IS_GAP_SYMBOL(*st->text)) {
                st->state.arr_state = ARR_STATE_ERROR;
                return 0;
            }
            ++st->text;
        }
    }

    cJsonNodeFree(&st->curNode);
    st->ret = 0;
    st->state.arr_state = ARR_STATE_DONE;

    return 0;
}

long cJsonValNum(pJsonNode_T node)
{
    if (node == NULL) {
        return 0;
    }
    return node->value.lVal;
}

const char *cJsonValString(pJsonNode_T node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->value.stringVal;
}

bool cJsonValBool(pJsonNode_T node)
{
    if (node == NULL) {
        return false;
    }

    return node->value.boolVal;
}

pJsonObj_T cJsonValObj(pJsonNode_T node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->value.objVal;
}

pJsonArray_T cJsonValArr(pJsonNode_T node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->value.arrVal;
}

pJsonNode_T cJsonVal(pJsonObj_T obj, const char *key)
{
    if (obj == NULL || key == NULL) {
        return NULL;
    }

    for (pJsonNode_T tmp = obj->head; tmp != NULL; tmp = tmp->next) {
        if (strcmp(tmp->key, key) == 0) {
            return tmp;
        }
    }

    return NULL;
}

JSONTYPE_E cJsonValType(pJsonNode_T n)
{
    if (n == NULL) {
        return TYPE_MAX;
    }

    return n->type;
}

