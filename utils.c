#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void print(char *path)
{
    int f_size;

    FILE *fp = fopen(path, "r");

   while(!feof(fp))
   {
        char a = fgetc(fp);
        printf("%c", a);
   }
    
    fclose(fp);
}