//
// Created by Zer0 on 2019/1/31.
//

#include <stdio.h>

#include "../src/cJSON.h"

int main(int arc, char *argv[])
{
    pJsonObj_T obj = cJsonNew();
    if (obj == NULL) {
        printf("new json object failed\n");
        return -1;
    }

    cJsonAddInt(obj, "test1", 1);
    cJsonAddInt(obj, "test2", 2);
    cJsonAddString(obj, "test3", "test3");
    cJsonAddInt(obj, "test0", 0);

    cJsonPrint(obj);

    cJsonFree(obj);

    return 0;
}
