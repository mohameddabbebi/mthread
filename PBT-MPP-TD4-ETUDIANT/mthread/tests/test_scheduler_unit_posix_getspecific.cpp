#include <mthread.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>

void dummy_destructor(void *ptr) {
    free(ptr);
}

int main() {
    printf("==== TEST mthread_getspecific ====\n");

    mthread_key_t key{};
    int ret = mthread_key_create(&key, dummy_destructor);
    assert(ret == 0);

    int *val = (int*)malloc(sizeof(int));
    *val = 99;

    mthread_setspecific(&key, val);

    int *res = (int*)mthread_getspecific(&key);
    assert(res != nullptr);
    assert(*res == 99);

    printf("[OK] getspecific returned %d\n", *res);
    printf("==== TEST OK ====\n");

    return 0;
}