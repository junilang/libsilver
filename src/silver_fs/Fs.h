#ifndef Fs_SAFE
	#define Fs_SAFE LIBSILVER_SAFE
#endif

typedef enum : u8 {
	FsFileMode_Read
} FsFileMode;

typedef STRUCTDECL(FsFile);
FsRes FsFile_open(String path_utf8, FsFileMode mode, FsFile *out_file);
FsRes FsFile_close(FsFile this);
FsRes FsFile_size(FsFile this, usize *out_size);

typedef enum : u8 {
	FsMappingMode_Read,
} FsMappingMode;

typedef STRUCTDECL(FsMapping);
FsRes FsFile_map(FsFile this, FsMappingMode mode, FsMapping *out_mapping);
FsRes FsMapping_unmap(FsMapping *mapping);
usize FsMapping_size(const FsMapping *this);
const ubyte *FsMapping_data(const FsMapping *this);
