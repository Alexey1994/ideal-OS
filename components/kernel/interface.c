Interface* interfaces_list = 0;


/*
void print_interfaces(Interface* interfaces_list)
{
	while(interfaces_list) {
		print("%s\n", interfaces_list->name);

		interfaces_list = interfaces_list->next;
	}
}*/


Interface* open_interface(Interface* parent_interface, Byte* name)
{
	Interface* interface;
	
	if(parent_interface) {
		interface = parent_interface->data;
	}
	else {
		interface = interfaces_list;
	}
	
	while(interface) {
		if(!compare_strings(interface->name, name)) {
			return interface;
		}
		
		interface = interface->next;
	}
	
	return 0;
}


void free_interface(Interface* interface)
{
	Interface** prev;

	if(interface->parent) {
		prev = &((Interface*)interface->parent)->data;
	}
	else {
		prev = &interfaces_list;
	}

	while((*prev)->next && (*prev)->next != interface) {
		prev = &(*prev)->next;
	}

	*prev = interface->next;
}


Interface* create_interface(Interface* parent_interface, Byte* name, void* data, Boolean is_folder)
{
	Interface** interface_pointer;
	
	if(parent_interface) {
		interface_pointer = &parent_interface->data;
	}
	else {
		interface_pointer = &interfaces_list;
	}
	
	while(*interface_pointer) {
		interface_pointer = &(*interface_pointer)->next;
	}
	
	*interface_pointer = allocate_memory(sizeof(Interface), &free_interface); //TODO: free_interface_folder
	(*interface_pointer)->parent = parent_interface;
	(*interface_pointer)->next = 0;
	copy_bytes((*interface_pointer)->name, name, 32);
	(*interface_pointer)->is_folder = is_folder;
	(*interface_pointer)->data = data;
	
	return *interface_pointer;
}


/*
void delete_interface(Interface* parent_interface, Byte* name)
{
	
}*/




Interface* get_interface(Byte* path)
{
	Interface* root;
	Byte       name[32];
	Number     i;
	
	root = 0;
	
	while(*path) {
		if(*path == '/') {
			++path;
		}

		for(i = 0; i < sizeof(name) - 1 && *path && *path != '/'; ++i) {
			name[i] = *path;
			++path;
		}
		name[i] = '\0';
		
		root = open_interface(root, name);
		
		if(!root) {
			return 0;
		}
	}
	
	return root;
}


Interface* create_interface_by_path(Byte* path, void* data)
{
	Interface* root;
	Byte       name[32];
	Number     i;
	
	root = 0;
	name[0] = '\0';
	
	while(*path) {
		if(*path == '/') {
			++path;
		}

		for(i = 0; i < sizeof(name) - 1 && *path && *path != '/'; ++i) {
			name[i] = *path;
			++path;
		}
		name[i] = '\0';
		
		if(*path == '/') {
			root = open_interface(root, name);
			
			if(!root) {
				root = create_interface(root, name, 0, 1);
			}
			//TODO: maybe error for incompatible types
			//else if(!root->is_folder) {
			//	
			//}
		}
		else {
			break;
		}
	}
	
	return create_interface(root, name, data, 0);
}


void* get_interface_data(Byte* path)
{
	Interface* interface;
	
	interface = get_interface(path);
	
	if(interface) {
		return interface->data;
	}
	
	return 0;
}