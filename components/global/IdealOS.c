#ifndef API_INCLUDED
#define API_INCLUDED


#include "types.c"


typedef struct {
	struct Interface* next;
	struct Interface* parent;
	Byte              name[32];
	Boolean           is_folder;
	void*             data;
}
Interface;


#define MAX_NUMBER_OF_ARGUMENTS 256


#include "interfaces/heap.c"
#include "interfaces/file.c"
#include "interfaces/process.c"

#include "interfaces/graphics.c"
#include "interfaces/keyboard.c"


#define get_module_address_by_function(function) \
	Number module_address; \
	asm("call . + 5\n" "pop %0" : "=a"(module_address)); \
	module_address -= (Number)&(function) + 15;

#define global(name) *(void**)((Byte*)&(name) + module_address)
#define global_ptr(name) ((Byte*)&(name) + module_address)


#endif//API_INCLUDED