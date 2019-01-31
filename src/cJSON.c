//
// Created by Zer0 on 2019/1/30.
//

#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define S_FREE(p) do {  \
    if (p) {            \
        free(p);        \
        p = NULL;       \
    }                   \
} while(0);

pJsonObj_T cJsonNew()
{
    pJsonObj_T obj = (pJsonObj_T)malloc(sizeof(JsonObj_T));
    if (obj == NULL) {
        perror("malloc obj failed\n");
        return NULL;
    }

    obj->head = NULL;
    obj->size = 0;

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
        S_FREE(node->value.sVal);
    } else if (node->type == TYPE_OBJECT) {
        cJsonFree(node->value.objVal);
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
        n->value.sVal = (char *)malloc(vSize);
        if (n->value.sVal == NULL) {
            perror("malloc node->value.sVal failed\n");
            goto ERR;
        }
    } else if (type == TYPE_OBJECT) {
        n->value.objVal = (pJsonObj_T)malloc(vSize);
        if (n->value.objVal == NULL) {
            perror("malloc node->value.objVal failed\n");
            goto ERR;
        }
    } else if (type == TYPE_ARRAY) {
        n->value.arrVal = (pJsonArray_T)malloc(vSize);
        if (n->value.arrVal == NULL) {
            perror("malloc node->value.arrVal failed\n");
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

void cJsonFree(pJsonObj_T obj)
{
    pJsonNode_T tmp = obj->head;
    while (tmp != NULL) {
        obj->head = obj->head->next;

        if (obj->head != NULL)
            obj->head->prev = NULL;

        cJsonNodeFree(tmp);
        tmp = obj->head;
    }
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
    printf("add front\n");
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
    printf("add next\n");
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
    strcpy(node->value.sVal, value);

    cJsonAdd(obj, node);
}

void cJsonPrint(pJsonObj_T obj)
{
    printf("size of obj : %ld\n", obj->size);
    for (pJsonNode_T tmp = obj->head; tmp != NULL; tmp = tmp->next) {
        if (tmp->type == TYPE_INT) {
            printf("key : %s, value : %ld\n", tmp->key, tmp->value.lVal);
        } else if (tmp->type == TYPE_STRING) {
            printf("key : %s, value : %s\n", tmp->key, tmp->value.sVal);
        }
    }
}
