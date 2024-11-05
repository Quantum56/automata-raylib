#include <stdio.h>

#include "set.h"


const char* test[2] = {"Hi", "hello"};

int main(void) {
    SimpleSet s;
    set_init(&s);
    printf("Hello, world!\n");
    

    set_add(&s, "Hello");
    for(int i = 0; i < 2; i++) {
        set_add(&s, test[i]);
    }

    if(set_add(&s, "Hello") == SET_ALREADY_PRESENT) {
        printf("Hello was already there\n");
    }
    set_destroy(&s);
}