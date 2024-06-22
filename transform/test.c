#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transform_central.h"


int main (int argc, char* argv[]) {
    int numfiles = argc - 1;
    printf("Numfiles: %d\n", numfiles);
    for(int i = 0; i < numfiles; i++){
        printf("File %d: %s\n", i, argv[i+1]);
    }
    return 0;
}
