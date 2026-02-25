#include <mthread.h>
#include <mthread_common_helpers.h>
#include <mthread_thread.h>
#include <mthread_vp_internal.h>
#include <stdexcept>

int mthread_cond_init(mthread_cond_t *cond) {
  if (cond == nullptr)
    return MTHREAD_COND_ERROR_NULL;
  if (cond->is_initialized)
    return MTHREAD_COND_ERROR_INIT;
  cond->thread_list.head = nullptr;
  cond->thread_list.tail = nullptr;

  atomic_flag_clear(&cond->thread_list_spinlock);

  cond->is_initialized = true;

  return 0;
}

int mthread_cond_signal(mthread_cond_t *cond) {
  if (cond == nullptr)
    return MTHREAD_COND_ERROR_NULL;

  if (!cond->is_initialized)
    return MTHREAD_COND_ERROR_INIT;

  mthread_internal_spin_lock(&cond->thread_list_spinlock);

  mthread_list_item_t *item =
      remove_head_in_list(&cond->thread_list);

  mthread_internal_spin_unlock(&cond->thread_list_spinlock);

  if (item != nullptr) {
    mthread_wake_thread(item->thread);
    free(item);
  }

  return 0;
}

int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mut) {
  if (cond == nullptr || mut == nullptr)
    return MTHREAD_COND_ERROR_NULL;

  if (!cond->is_initialized)
    return MTHREAD_COND_ERROR_INIT;

  if (!mut->is_locked || mut->owner != mthread_self())
    return MTHREAD_COND_ERROR_NOT_LOCKED;

  mthread_t self = mthread_self();

  mthread_list_item_t *item =
      (mthread_list_item_t *)malloc(sizeof(mthread_list_item_t));

  item->thread = self;
  item->next = nullptr;

  mthread_internal_spin_lock(&cond->thread_list_spinlock);
  insert_tail_in_list(item, &cond->thread_list);
  mthread_internal_spin_unlock(&cond->thread_list_spinlock);

  mthread_mutex_unlock(mut);

 mthread_block_self(&cond->thread_list_spinlock);

  mthread_mutex_lock(mut);

  return 0;
}

int mthread_cond_broadcast(mthread_cond_t *cond) {
  if (cond == nullptr)
    return MTHREAD_COND_ERROR_NULL;

  if (!cond->is_initialized)
    return MTHREAD_COND_ERROR_INIT;

  mthread_internal_spin_lock(&cond->thread_list_spinlock);

  mthread_list_item_t *item;
  while ((item = remove_head_in_list(&cond->thread_list)) != nullptr) {

    mthread_internal_spin_unlock(&cond->thread_list_spinlock);

    mthread_wake_thread(item->thread);
    free(item);

    mthread_internal_spin_lock(&cond->thread_list_spinlock);
  }

  mthread_internal_spin_unlock(&cond->thread_list_spinlock);

  return 0;
}

int mthread_cond_destroy(mthread_cond_t *cond) {
  if (cond == nullptr)
    return MTHREAD_COND_ERROR_NULL;

  if (!cond->is_initialized)
    return MTHREAD_COND_ERROR_INIT;

  if (cond->thread_list.head != nullptr)
    return MTHREAD_COND_ERROR_IN_USE;

  cond->is_initialized = false;

  return 0;
}
