typedef struct {
	struct Memory_Chunk* previous_chunk;
	Boolean              is_busy;
	void                (*on_free)(void* data);
	Process*             owner;
}
Memory_Chunk;


extern Process* current_process;


Byte*          heap_top;
Byte*          max_heap_top;
Memory_Chunk*  top_chunk  = 0;


Boolean initialize_heap()
{
	Number i;
	Memory_Region* region;
	Memory_Region* largest_region;

	largest_region = 0;

	for(i = 0; i < loader_api->size_of_memory_regions; i += sizeof(Memory_Region)) {
		region = (Byte*)loader_api->memory_regions + i;

		if(region->type != 1 || region->address_high) {
			continue;
		}

		if(!largest_region || region->size_low > largest_region->size_low) {
			largest_region = region;
		}
	}

	if(!largest_region) {
		return 0;
	}

	heap_top = largest_region->address_low;
	max_heap_top = heap_top + largest_region->size_low;

	return 1;
}


Byte* allocate_memory(Number size, void(*on_free)(void* data))
{
	Memory_Chunk* new_chunk;
	Byte*         allocated_memory;

	if(size > max_heap_top - heap_top || size + sizeof(Memory_Chunk) > max_heap_top - heap_top) {
		return 0;
	}

	new_chunk = heap_top;
	new_chunk->previous_chunk = top_chunk;
	new_chunk->is_busy = 1;
	new_chunk->on_free = on_free;
	new_chunk->owner = current_process;

	heap_top += sizeof(Memory_Chunk);
	allocated_memory = heap_top;
	heap_top += size;

	top_chunk = new_chunk;

	return allocated_memory;
}


void free_memory(Byte* allocated_memory)
{
	Memory_Chunk* chunk;

	chunk = allocated_memory - sizeof(Memory_Chunk);

	if(chunk->owner != current_process) {
		return;
	}
	
	if(chunk->is_busy && chunk->on_free) {
		chunk->on_free(allocated_memory);
	}

	chunk->is_busy = 0;

	//defragmentation
	while(!top_chunk->is_busy) {
		heap_top = top_chunk;
		top_chunk = top_chunk->previous_chunk;
	}
}


Number get_free_memory_size()
{
	return max_heap_top - heap_top;
}


Heap_Interface heap_interface = {
	.allocate = &allocate_memory,
	.free = &free_memory,
	.get_free_size = &get_free_memory_size
};