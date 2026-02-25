#include <mthread.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>

// ==========================
// Destructeur de test
// ==========================

static int destructor_called = 0;

void tls_destructor(void *ptr) {
    printf("[destructor] called with ptr=%p\n", ptr);
    destructor_called++;
    free(ptr);
}

// ==========================
// Fonction exécutée par un thread
// ==========================

void* thread_func(void *arg) {
    mthread_key_t *key = (mthread_key_t*)arg;

    int *val = (int*)malloc(sizeof(int));
    *val = 42;

    printf("[thread %p] setspecific %d\n", mthread_self(), *val);
    mthread_setspecific(key, val);

    int *res = (int*)mthread_getspecific(key);
    assert(res != nullptr);
    assert(*res == 42);

    printf("[thread %p] getspecific %d\n", mthread_self(), *res);

    return nullptr;
}

// ==========================
// main
// ==========================

int main() {
    printf("==== TEST TLS POSIX ====\n");

    mthread_key_t key{};
    int ret;

    // --------------------------
    // Q7 : key_create
    // --------------------------
    ret = mthread_key_create(&key, tls_destructor);
    assert(ret == 0);
    printf("[main] key created (id=%d)\n", key.id);

    // --------------------------
    // Q9 / Q10 : main thread
    // --------------------------
    int *main_val = (int*)malloc(sizeof(int));
    *main_val = 100;

    mthread_setspecific(&key, main_val);

    int *main_res = (int*)mthread_getspecific(&key);
    assert(main_res != nullptr);
    assert(*main_res == 100);

    printf("[main] getspecific %d\n", *main_res);

    // --------------------------
    // Création de threads
    // --------------------------
    mthread_t t1 = mthread_create_thread(nullptr, thread_func, &key);
    mthread_t t2 = mthread_create_thread(nullptr, thread_func, &key);

    mthread_join(t1, nullptr);
    mthread_join(t2, nullptr);

    printf("[main] threads finished\n");

    // --------------------------
    // Q8 : key_delete
    // --------------------------
    ret = mthread_key_delete(&key);
    assert(ret == 0);

    printf("[main] key deleted\n");
    printf("[main] destructor called %d time(s)\n", destructor_called);

    // Ici : destructeur appelé AU MOINS pour le thread main
    assert(destructor_called >= 1);

    printf("==== TEST OK ====\n");
    return 0;
}