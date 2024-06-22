#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
void removeSubstr (char *string, char *sub) {
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match+len);
    }
}
 
int main(int argc, const char *argv[]) {
    char test[] = "../auxM/Us1-Mail1.txt";
    removeSubstr(test, "../auxM/");
    puts(test);
    return 0;
}