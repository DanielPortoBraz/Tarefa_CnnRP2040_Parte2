#include <stdlib.h>
#include <stdio.h>
#include "mnist_sample.h"

#define SIZE 20

int main() {

    for (int i = 0; i < 784; i++)
        printf("%d ", mnist_sample_28x28[i]);
 
    return 0;
}