#include <stdio.h>
#include <stdlib.h>


int main(){

    char num[12] = "111111111111";
    long unsigned int addr = strtol(num, NULL, 10);
    printf("%lu", addr);
    
}