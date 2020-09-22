#ifndef _RT_POSIX_MUTEX_H_
#define _RT_POSIX_MUTEX_H_
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <embdCOMMON.h>
#include <rt_posix_task.h>
/*****************************************************************************/
typedef struct {
	pthread_mutex_t lock;
	char* name;
}PT_MUTEX;
/*****************************************************************************/
int pt_mutex_acquire(PT_MUTEX *mutex);
int pt_mutex_create(PT_MUTEX *mutex, char* name);
int pt_mutex_delete(PT_MUTEX *mutex);
int pt_mutex_release(PT_MUTEX *mutex);

#ifdef __cplusplus
}
#endif 
#endif // _RT_POSIX_MUTEX_H_