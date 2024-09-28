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


#endif//API_INCLUDED