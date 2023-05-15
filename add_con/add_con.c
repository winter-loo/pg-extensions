#include "postgres.h"
#include "fmgr.h"
#include "storage/lwlock.h"
#include "storage/shmem.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(shared_add);
PG_FUNCTION_INFO_V1(shared_reset);
PG_FUNCTION_INFO_V1(shared_get);
PG_FUNCTION_INFO_V1(priv_add);
void _PG_init(void);

typedef struct
{
    LWLock lwlock;
    int sharedData;
} MySharedData;

// sharedData shared by shared memory and persisted onto disk
static MySharedData *sharedData;
// privData is only for each database connnection
static int privData = 0;

// called immediatly when loads the dynamic library
void _PG_init(void)
{
    bool found;
    MySharedData *shmem;

    if (!sharedData) {
        LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);

        RequestAddinShmemSpace(sizeof(MySharedData));
        shmem = (MySharedData *)ShmemInitStruct("MyExtension", sizeof(MySharedData), &found);

        if (!found)
        {
            LWLockInitialize(&shmem->lwlock, LWLockNewTrancheId());
            shmem->sharedData = 0;
        }

        sharedData = shmem;

        LWLockRelease(AddinShmemInitLock);
    }
}

Datum shared_add(PG_FUNCTION_ARGS)
{
    int rv; 
    int count = PG_GETARG_INT32(0);
    LWLockAcquire(&(sharedData->lwlock), LW_EXCLUSIVE);
    // Access and modify sharedData->sharedData safely
    sharedData->sharedData += count;
    rv = sharedData->sharedData;
    LWLockRelease(&(sharedData->lwlock));

    PG_RETURN_INT32(rv);
}

Datum shared_reset(PG_FUNCTION_ARGS) {
    LWLockAcquire(&(sharedData->lwlock), LW_EXCLUSIVE);
    // Access and modify sharedData->sharedData safely
    sharedData->sharedData = 0;
    LWLockRelease(&(sharedData->lwlock));

    PG_RETURN_NULL();
}

Datum shared_get(PG_FUNCTION_ARGS) {
    int ans;
    LWLockAcquire(&(sharedData->lwlock), LW_EXCLUSIVE);
    // Access and modify sharedData->sharedData safely
    ans = sharedData->sharedData;
    LWLockRelease(&(sharedData->lwlock));

    PG_RETURN_INT32(ans);
}

Datum priv_add(PG_FUNCTION_ARGS) {
    int a = PG_GETARG_INT32(0);
    privData += a;
    PG_RETURN_INT32(privData);
}