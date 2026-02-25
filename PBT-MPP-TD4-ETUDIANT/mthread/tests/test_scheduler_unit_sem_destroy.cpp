#include <cassert>
#include <cstdio>
#include <mthread.h>

int NB_VP = 1;
int NB_THS = 1;

#ifndef mthread_init_scheduler_test
#error "macro mthread_init_scheduler_test have to be defined for ce test"
#endif

extern "C" void mthread_abort();
static volatile int has_abort = 0;
void mthread_abort() { has_abort = 1; }

#define Check_val(val) \
    { \
        if (res != (val)) { \
            fprintf(stderr, "Expected %d, got %d\n", (val), res); \
            return 1; \
        } \
    }

void *thread_test(void *arg) {
    mthread_sem_t *sem = (mthread_sem_t *)arg;
    mthread_sem_wait(sem);
    return nullptr;
}

int main() {
    mthread_init_scheduler_test(NB_VP);

    // Test destroy sur sémaphore vide
    {
        mthread_sem_t sem;
        int res = mthread_sem_init(&sem, 1);
        Check_val(0);

        res = mthread_sem_destroy(&sem);
        Check_val(0);
    }

    // Test destroy sur sémaphore avec thread bloqué
    {
        mthread_sem_t sem;
        int res = mthread_sem_init(&sem, 0); // init à 0 pour bloquer
        Check_val(0);

        mthread_t t = mthread_create_thread(nullptr, thread_test, &sem);

        // Attendre que le thread soit bloqué dans la liste
        while (sem.thread_list.head == nullptr)
            mthread_yield();

        // Maintenant, destroy doit échouer
        res = mthread_sem_destroy(&sem);
        Check_val(MTHREAD_SEM_ERROR_ALREADY_LOCKED);
    }

    return 0;
}
