//
// Created by Zer0 on 2019/1/31.
//

#include <stdio.h>

#include "../src/cJSON.h"
#include "../src/cJsonDefines.h"

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
    cJsonAddBool(obj, "bool0", true);
    cJsonAddBool(obj, "bool1", false);
    pJsonObj_T obj_2 = cJsonNew();
    cJsonAddString(obj_2, "hello", "hello");
    cJsonAddObj(obj, "obj2", obj_2);
    pJsonArray_T arr = cJsonArrNew();
    cJsonArrAppendNum(arr, 1);
    cJsonArrAppendString(arr, "123");
    cJsonArrAppendBool(arr, true);
    cJsonArrAppendBool(arr, false);
    pJsonObj_T obj_3 = cJsonNew();
    cJsonArrAppendObj(arr, obj_3);
    pJsonArray_T arr_2 = cJsonArrNew();
    cJsonArrAppendArr(arr, arr_2);
    cJsonAddArray(obj, "arr", arr);
    char *str = cJsonMashal(obj);
    printf("mashal : %s\n", str);
    S_FREE(str);

    cJsonDel(obj, "test2");
    str = cJsonMashal(obj);
    printf("mashal : %s\n", str);
    S_FREE(str);

    cJsonArrInsertNumAt(arr_2, 0, 2);
    str = cJsonMashal(obj);
    printf("mashal : %s\n", str);
    S_FREE(str);

    cJsonArrInsertStringAt(arr, 0, "123123");
    str = cJsonMashal(obj);
    printf("mashal : %s\n", str);
    S_FREE(str);

    cJsonArrDel(arr, 0);
    str = cJsonMashal(obj);
    printf("mashal : %s\n", str);
    S_FREE(str);

    cJsonFree(&obj);

    pJsonObj_T parse_obj = cJsonParse("{\"number\":12,  \"str\":\"111\", \"b\":true, \"f\":false, \"obj\":{\"str\":\"lll\"}}");
    str = cJsonMashal(parse_obj);
    printf("mashal : %s\n", str);
    S_FREE(str);

    return 0;
}
