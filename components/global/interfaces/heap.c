#ifndef HEAP_INCLUDED
#define HEAP_INCLUDED


typedef struct {
	void*  (*allocate)         (Number block_size, void(*on_free)(void* data));
	void   (*free)             (void* block);
	Number (*get_free_size)    ();
}
Heap_Interface;


#endif//HEAP_INCLUDED