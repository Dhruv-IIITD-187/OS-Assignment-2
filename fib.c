#include <stdio.h>
#include <stdlib.h>

long long fib(int n) {
    if (n <= 1) return n;
    return fib(n-1) + fib(n-2);
}

int main(int argc, char *argv[]) {
    int n = atoi(argv[1]);
    printf("%lld\n", fib(n));
    return 0;
}
