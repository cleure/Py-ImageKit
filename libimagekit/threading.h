#pragma once

#define MUTEX_LOCK(m)               pthread_mutex_lock(&(m))
#define MUTEX_UNLOCK(m)             pthread_mutex_unlock(&(m));
#define THREAD_ID()                 (uintptr_t)pthread_self()

#define MUTEX_T                     pthread_mutex_t
#define MUTEX_T_INITIALIZER         PTHREAD_MUTEX_INITIALIZER

#define BEGIN_SYNCHRONIZED(m)       MUTEX_LOCK(m);
#define END_SYNCHRONIZED(m)         MUTEX_UNLOCK(m)
