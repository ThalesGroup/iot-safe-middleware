/*
 *    Copyright (c) 2019 - 2020, Thales DIS Singapore, Inc
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/rot_tests_helper.h"
static void sprintByteArray(char * pdst, const unsigned char* pbytes, size_t length)
{
    int i;
    char* pbuf = pdst;
    char* pendofbuf = pdst + length*2+1;
    for (i = 0; i < length; i++)
    {
        if (pbuf + 1 < pendofbuf)
        {
            pbuf += snprintf(pbuf, 3, "%02X", pbytes[i]);
        }
    }
    *pbuf = '\0';
}

void printByteArray(const char* pid, const unsigned char* pbytes, size_t length)
{
    char * pdst =  (char*) malloc((length * 2 + 1) * sizeof(char));
    if (pdst != NULL)
    {
        sprintByteArray( pdst, pbytes, length);
        printf("%s: [%zu] \n", pid, length);
        printf("%s\n", pdst);
        free(pdst);
    }
}
