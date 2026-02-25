#include <mthread.h>

#include <atomic>
#include <unordered_map>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

// ==========================
// Gestion globale des clés
// ==========================

static std::atomic_int g_next_key_id{0};

// TLS : une map par thread
static thread_local std::unordered_map<int, void*> tls_values;

// ==========================
// Q7 : mthread_key_create
// ==========================

int mthread_key_create(mthread_key_t *key, void (*destructor)(void*)) {
    if (key == nullptr) {
        return -1;
    }

    if (key->is_initialized) {
        return -1;
    }

    key->id = g_next_key_id.fetch_add(1);
    key->destructor = destructor;
    key->is_initialized = true;

    return 0;
}

// ==========================
// Q8 : mthread_key_delete
// ==========================

int mthread_key_delete(mthread_key_t *key) {
    if (key == nullptr || !key->is_initialized) {
        return -1;
    }

    // Si une valeur TLS existe pour ce thread, appeler le destructeur
    auto it = tls_values.find(key->id);
    if (it != tls_values.end()) {
        if (key->destructor != nullptr) {
            key->destructor(it->second);
        }
        tls_values.erase(it);
    }

    key->is_initialized = false;
    key->destructor = nullptr;
    key->id = -1;

    return 0;
}

// ==========================
// Q9 : mthread_setspecific
// ==========================

int mthread_setspecific(mthread_key_t *key, void *value) {
    if (key == nullptr || !key->is_initialized) {
        return -1;
    }

    tls_values[key->id] = value;
    return 0;
}

// ==========================
// Q10 : mthread_getspecific
// ==========================

void* mthread_getspecific(mthread_key_t *key) {
    if (key == nullptr || !key->is_initialized) {
        return nullptr;
    }

    auto it = tls_values.find(key->id);
    if (it == tls_values.end()) {
        return nullptr;
    }

    return it->second;
}

#ifdef __cplusplus
}
#endif