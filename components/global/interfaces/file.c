#ifndef FILE_INCLUDED
#define FILE_INCLUDED


typedef enum {
	IdealFS_FILE   = 1,
	IdealFS_FOLDER = 2,
	IdealFS_DATA   = 4,
}
IdealFS_Node_Type;


typedef struct {
	Number16 type;
	Number16 size;
	Number32 next;
	Number32 prev;
	Byte     data[500];
}
IdealFS_Node;


typedef struct {
	Number16 type;
	Number16 size;
	Number32 next;
	Number32 prev;
	Number32 inner;
	Byte     name[496];
}
IdealFS_File_Node;


typedef struct {
	Number32 last_node_index;
}
IdealFS;


typedef struct {
	Number32          file_node_index;
	Number32          data_node;
	IdealFS_File_Node file_node;
}
File;


typedef struct {
	Boolean       (*read)        (File* file, Number32 file_node_index);
	Boolean       (*find)        (File* file, Byte* path);
	Boolean       (*create)      (File* file, Byte* name);
	Signed_Number (*read_bytes)  (File* file, Byte* bytes, Number number_of_bytes);
	Signed_Number (*write_bytes) (File* file, Byte* bytes, Number number_of_bytes);
}
File_Interface;


#endif//FILE_INCLUDED