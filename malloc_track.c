#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>

static void* (*real_malloc)(size_t)=NULL;
static void (*real_free)(void*)=NULL;

static void mtrace_init(void)
{
	real_malloc = dlsym(RTLD_NEXT, "malloc");
	real_free = dlsym(RTLD_NEXT, "free");
	assert(real_free && real_malloc);
}

void *malloc(size_t size)
{
	if(!real_malloc)
	{
		mtrace_init();
	}

	void *p = NULL;
	p = real_malloc(size);
	return p;
}

void free(void* p)
{
	if (!real_free)
	{
		mtrace_init();
	}

	real_free(p);
}
