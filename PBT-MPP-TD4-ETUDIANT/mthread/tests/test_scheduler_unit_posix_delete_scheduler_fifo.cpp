#include <mthread.h>
#include <cassert>
#include <cstdio>

void dummy_destructor(void*) {}

int main() {
    printf("==== TEST mthread_key_delete ====\n");

    mthread_key_t key{};
    int ret;

    ret = mthread_key_create(&key, dummy_destructor);
    assert(ret == 0);

    ret = mthread_key_delete(&key);
    assert(ret == 0);

    assert(key.is_initialized == false);

    printf("[OK] key deleted\n");
    printf("==== TEST OK ====\n");

    return 0;
}