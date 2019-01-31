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
void cJsonNodeAddFrontAt(pJsonNode_T base, pJsonNode_T value)
{
    printf("add front\n");
    value->next = base;
    value->prev = base->prev;
    if (base->prev != NULL) {
        base->prev->next = value;
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
                cJsonNodeAddFrontAt(tmp, value);
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

    pJsonNode_T node = (pJsonNode_T)malloc(sizeof(JsonNode_T));
    if (node == NULL) {
        return;
    }
    node->key = (char *)malloc(strlen(key)+1);
    if (node->key == NULL) {
        S_FREE(node);
        return;
    }
    strcpy(node->key, key);

    node->type = TYPE_INT;
    node->value.lVal = value;
    node->next = NULL;
    node->prev = NULL;

    cJsonAdd(obj, node);
}

void cJsonPrint(pJsonObj_T obj)
{
    printf("size of obj : %ld\n", obj->size);
    for (pJsonNode_T tmp = obj->head; tmp != NULL; tmp = tmp->next) {
        if (tmp->type == TYPE_INT) {
            printf("key : %s, value : %ld\n", tmp->key, tmp->value.lVal);
        }
    }
}
