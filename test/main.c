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
    cJsonAddInt(obj, "test3", 3);

    cJsonPrint(obj);

    cJsonFree(obj);

    return 0;
}
