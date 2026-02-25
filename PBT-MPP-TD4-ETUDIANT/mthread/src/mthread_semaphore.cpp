#include <mthread.h>
#include <mthread_thread.h>
#include <mthread_common_helpers.h>
#include <mthread_vp_internal.h>
#include <stdexcept>


#include <mthread.h>
#include <atomic>
int mthread_sem_init(mthread_sem_t *sem, int value)
{
    if (!sem) {
      return MTHREAD_SEM_ERROR_NULL;
    }

    if (sem->is_initialized) {
        return MTHREAD_SEM_ERROR_INIT;
    }

    if (value < 0)  {
        return MTHREAD_SEM_ERROR_INIT;
    }
    atomic_init(&sem->value, value);
    atomic_flag_clear(&sem->thread_list_spinlock);
    sem->thread_list.head = nullptr;
    sem->thread_list.tail = nullptr;
    sem->is_initialized = true;
    return 0;
}


int mthread_sem_wait(mthread_sem_t *sem) {
    if (!sem)
        return MTHREAD_SEM_ERROR_NULL;

    if (!sem->is_initialized)
        return MTHREAD_SEM_ERROR_INIT;

    int old = atomic_fetch_sub(&sem->value, 1);

    if (old <= 0) {
        
        while (atomic_flag_test_and_set(&sem->thread_list_spinlock))
            ; 

        mthread_list_item_t *item = new mthread_list_item_t;
        item->thread = mthread_self();
        item->next = nullptr;

        if (!sem->thread_list.tail) {
            sem->thread_list.head = item;
            sem->thread_list.tail = item;
        } else {
            sem->thread_list.tail->next = item;
            sem->thread_list.tail = item;
        }

        atomic_flag_clear(&sem->thread_list_spinlock);

        
        mthread_yield(); 
    }

    return 0;
}

int mthread_sem_post(mthread_sem_t *sem) {
    if (!sem)
        return MTHREAD_SEM_ERROR_NULL;

    if (!sem->is_initialized)
        return MTHREAD_SEM_ERROR_INIT;

    int old = atomic_fetch_add(&sem->value, 1);

    if (old < 0) {
        
        while (atomic_flag_test_and_set(&sem->thread_list_spinlock))
            ;

        mthread_list_item_t *item = (mthread_list_item_t *)sem->thread_list.head;

        if (item) {
            sem->thread_list.head = item->next;
            if (!sem->thread_list.head)
                sem->thread_list.tail = nullptr;

            
            mthread_yield(); 
            delete item;
        }

        atomic_flag_clear(&sem->thread_list_spinlock);
    }

    return 0;
}


int mthread_sem_getvalue(mthread_sem_t *sem, int *val) {
    if (!sem || !val)
        return MTHREAD_SEM_ERROR_NULL;

    if (!sem->is_initialized)
        return MTHREAD_SEM_ERROR_INIT;

    *val = atomic_load(&sem->value);
    return 0;
}


int mthread_sem_trylock(mthread_sem_t *sem) {
    if (!sem)
        return MTHREAD_SEM_ERROR_NULL;

    if (!sem->is_initialized)
        return MTHREAD_SEM_ERROR_INIT;

    int val = atomic_load(&sem->value);

    while (val > 0) {
        if (atomic_compare_exchange_weak(&sem->value, &val, val - 1))
            return 0; 
    }

    return MTHREAD_SEM_ERROR_ALREADY_LOCKED;
}

int mthread_sem_destroy(mthread_sem_t *sem) {
    if (!sem)
        return MTHREAD_SEM_ERROR_NULL;

    if (!sem->is_initialized)
        return MTHREAD_SEM_ERROR_INIT;

    // Acquérir le spinlock pour protéger la liste
    while (atomic_flag_test_and_set(&sem->thread_list_spinlock))
        ;

    // Vérifier si la liste des threads bloqués est vide
    bool empty = (sem->thread_list.head == nullptr);

    atomic_flag_clear(&sem->thread_list_spinlock);

    if (!empty)
        return MTHREAD_SEM_ERROR_ALREADY_LOCKED;

    sem->is_initialized = false;
    sem->thread_list.head = nullptr;
    sem->thread_list.tail = nullptr;

    return 0;
}
