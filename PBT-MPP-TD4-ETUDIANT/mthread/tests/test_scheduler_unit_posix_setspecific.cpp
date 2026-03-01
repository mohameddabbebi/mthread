#include <mthread.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>

void dummy_destructor(void*) {}

int main() {
    printf("==== TEST mthread_setspecific ====\n");

    mthread_key_t key{};
    int ret = mthread_key_create(&key, dummy_destructor);
    assert(ret == 0);

    int *val = (int*)malloc(sizeof(int));
    *val = 55;

    ret = mthread_setspecific(&key, val);
    assert(ret == 0);

    printf("[OK] setspecific worked\n");
    printf("==== TEST OK ====\n");

    return 0;
}