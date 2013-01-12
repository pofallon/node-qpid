// Adapted almost entirely from node_sqlite3
// Copyright (c) 2011, Konstantin Käfer <kkaefer@gmail.com>
// All rights reserved.

// https://github.com/developmentseed/node-sqlite3/blob/master/LICENSE

#ifndef NODE_CPROTON_SRC_THREADING_H
#define NODE_CPROTON_SRC_THREADING_H


#ifdef _WIN32

#include <windows.h>

    #define NODE_CPROTON_MUTEX_t HANDLE mutex;

    #define NODE_CPROTON_MUTEX_INIT CreateMutex(NULL, FALSE, NULL);

    #define NODE_CPROTON_MUTEX_LOCK(m) WaitForSingleObject(m, INFINITE);

    #define NODE_CPROTON_MUTEX_UNLOCK(m) ReleaseMutex(m);

    #define NODE_CPROTON_MUTEX_DESTROY CloseHandle(mutex);

#else

    #define NODE_CPROTON_MUTEX_t pthread_mutex_t mutex;

    #define NODE_CPROTON_MUTEX_INIT pthread_mutex_init(&mutex,NULL);

    #define NODE_CPROTON_MUTEX_LOCK(m) pthread_mutex_lock(m);

    #define NODE_CPROTON_MUTEX_UNLOCK(m) pthread_mutex_unlock(m);

    #define NODE_CPROTON_MUTEX_DESTROY pthread_mutex_destroy(&mutex);

#endif


#endif // NODE_CPROTON_SRC_THREADING_H
