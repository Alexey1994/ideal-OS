IdealFS fs = {1};

/*
void print_IdealFS()
{
	IdealFS_Node* node;
	Number32    node_index;

	node = 0x500;
	node_index = 1;

	while(node_index) {
		loader_api->read_sector(node_index);

		if(node->type != IdealFS_FILE && node->type != IdealFS_FOLDER) { //read error
			return;
		}

		print("%s\n", node->data + 4);

		node_index = node->next;		
	}
}*/


Boolean read_file(File* file, Number32 file_node_index)
{
	IdealFS_Node* node;

	node = 0x500;
	loader_api->read_sector(file_node_index);

	if(node->type != IdealFS_FILE && node->type != IdealFS_FOLDER) { //read error
		return 0;
	}

	copy_bytes(&file->file_node, node, sizeof(file->file_node));
	file->data_node = file->file_node.inner;
	file->file_node_index = file_node_index;

	return 1;
}


Boolean find_file(File* file, Byte* path)
{
	IdealFS_Node* node;

	node = 0x500;
	file->file_node_index = 1;

	while(file->file_node_index) {
		loader_api->read_sector(file->file_node_index);

		if(node->type != IdealFS_FILE && node->type != IdealFS_FOLDER) { //read error
			return 0;
		}

		if(!compare_strings(path, node->data + 4)) {
			copy_bytes(&file->file_node, node, sizeof(file->file_node));
			file->data_node = file->file_node.inner;
			return 1;
		}

		file->file_node_index = node->next;
	}

	return 0;
}


Signed_Number read_bytes_from_file(File* file, Byte* bytes, Number number_of_bytes)
{
	Signed_Number   bytes_readed;
	IdealFS_Node*   node;

	bytes_readed = 0;
	node = 0x500;

	while(number_of_bytes >= 500 && file->data_node) {
		loader_api->read_sector(file->data_node);

		if(node->type != IdealFS_DATA) { //read error
			goto error;
		}

		copy_bytes(bytes, node->data, 500);

		number_of_bytes -= 500;
		bytes += 500;
		bytes_readed += 500;

		file->data_node = node->next;
	}

	if(file->data_node && number_of_bytes) {
		loader_api->read_sector(file->data_node);

		if(node->type != IdealFS_DATA) { //read error
			goto error;
		}

		copy_bytes(bytes, node->data, 500);

		bytes_readed += number_of_bytes;

		file->data_node = node->next;
	}

	return bytes_readed;

	error: {
		return -1;
	}
}


Number32 rewind_IdealFS_to_end()
{
	IdealFS_Node* node;
	Number32      node_index;

	node = 0x500;
	node_index = 1;

	for(;;) {
		loader_api->read_sector(node_index);

		if(!node->type) {
			goto error;
		}
		else {
			if(!node->next) {
				return node_index;
			}

			node_index = node->next;
		}
	}

	error: {
		return 0;
	}
}


Number32 find_free_IdealFS_node()
{
	IdealFS_Node* node;

	node = 0x500;

	for(;;) {
		loader_api->read_sector(fs.last_node_index);

		if(!node->type) {
			break;
		}

		++fs.last_node_index;
	}

	return fs.last_node_index;

	error: {
		return 0;
	}
}


Number32 allocate_IdealFS_node(Number32 prev_node_index, Number32 next_node_index, IdealFS_Node* new_node)
{
	Number32      new_node_index;
	IdealFS_Node* prev_node;
	IdealFS_Node* next_node;

	new_node_index = find_free_IdealFS_node();

	if(!new_node_index) {
		goto error;
	}

	new_node->prev = prev_node_index;
	new_node->next = next_node_index;

	copy_bytes(0x500, new_node, 512);
	loader_api->write_sector(new_node_index);

	if(prev_node_index) {
		prev_node = 0x500;
		loader_api->read_sector(prev_node_index);
		prev_node->next = new_node_index;
		loader_api->write_sector(prev_node_index);
	}

	if(next_node_index) {
		next_node = 0x500;
		loader_api->read_sector(next_node_index);
		next_node->prev = new_node_index;
		loader_api->write_sector(next_node_index);
	}

	return new_node_index;

	error: {
		return 0;
	}
}


Number32 create_IdealFS_file(IdealFS_File_Node* file_node, Byte* name)
{
	Number32 file_node_index;
	Number32 prev_node_index;
	Number   i;


	clean_bytes(file_node, sizeof(IdealFS_File_Node));
	file_node->type = IdealFS_FILE;
	file_node->inner = 0;

	for(i = 0; name[i] && i < sizeof(file_node->name) - 1; ++i) {
		file_node->name[i] = name[i];
	}
	file_node->name[i] = '\0';


	prev_node_index = rewind_IdealFS_to_end();
	file_node_index = allocate_IdealFS_node(prev_node_index, 0, file_node);


	return file_node_index;

	error: {
		return 0;
	}
}


Boolean create_file(File* file, Byte* name)
{
	file->file_node_index = create_IdealFS_file(&file->file_node, name);

	if(!file->file_node_index) {
		goto error;
	}

	file->data_node = 0;

	return 1;

	error: {
		return 0;
	}
}


Signed_Number write_bytes_in_file(File* file, Byte* bytes, Number number_of_bytes)
{
	IdealFS_Node new_node;
	Boolean      is_empty;

	is_empty = !file->data_node;

	clean_bytes(&new_node, sizeof(new_node));
	new_node.type = IdealFS_DATA;
	copy_bytes(new_node.data, bytes, number_of_bytes);
	file->data_node = allocate_IdealFS_node(file->data_node, 0, &new_node);

	if(!file->data_node) {
		goto error;
	}

	if(is_empty) {
		file->file_node.inner = file->data_node;

		copy_bytes(0x500, &file->file_node, 512);
		loader_api->write_sector(file->file_node_index);
	}

	return number_of_bytes;

	error: {
		return -1;
	}
}


File_Interface file_interface = {
	.read        = &read_file,
	.find        = &find_file,
	.create      = &create_file,
	.read_bytes  = &read_bytes_from_file,
	.write_bytes = &write_bytes_in_file
};