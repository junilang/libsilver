struct FsFile {
	linux_fd fd;
};

constexpr usize Fs_maxpath = PATH_MAX;

FsRes FsFile_open(String path, FsFileMode mode, FsFile *out_file) {
	#if Fs_SAFE
		if (String_isnull(path))
			return FsRes_ErrInvalidPath;
	#endif

	if (path.size >= Fs_maxpath)
		return FsRes_ErrPathOverflow;

	// copy path string and null terminate for syscall compat
	ubyte path_str[Fs_maxpath];
	memcpy(path_str, path.data, path.size);
	path_str[path.size] = 0;

	struct open_how how = {};

	switch (mode) {
		case FsFileMode_Read:
			how.flags = O_RDONLY;
			break;
		default:
			return FsRes_ErrInvalidFileMode;
	}

	auto res = linux_openat2(AT_FDCWD, (Str)path_str, &how, sizeof(how));
	if (res < 0) switch (-res) {
		case ENOENT:
			return FsRes_ErrNotFound;
		default:
			return FsRes_ErrUnknown;
	}

	out_file->fd = (linux_fd)res;
	return FsRes_Ok;
}

FsRes FsFile_close(FsFile this) {
	auto res = linux_close(this.fd);
	if (res < 0) switch (-res) {
		default:
			return FsRes_ErrUnknown;
	}

	return FsRes_Ok;
}

FsRes FsFile_size(FsFile this, usize *out_size) {
	struct statx info;
	auto res = linux_statx(this.fd, nullptr, AT_EMPTY_PATH, STATX_SIZE, &info);
	if (res < 0) switch (-res) {
		default:
			return FsRes_ErrUnknown;
	}

	*out_size = info.stx_size;
	return FsRes_Ok;
}

struct FsMapping {
	usize size;
	Ptr data;
};

FsRes FsFile_map(FsFile this, FsMappingMode mode, FsMapping *out_mapping) {
	switch (mode) {
		default:
			return FsRes_ErrInvalidMappingMode;
		case FsMappingMode_Read:;
	}

	usize size; {
		auto res = FsFile_size(this, &size);
		if (res) return res;
	}

	auto res = linux_mmap(nullptr, size, PROT_READ, MAP_PRIVATE, this.fd, 0);
	if (linux_mmap_iserror(res)) switch (-res) {
		default:;
			return FsRes_ErrUnknown;
	}

	out_mapping->size = size;
	out_mapping->data = (Ptr)res;

	return FsRes_Ok;
}

FsRes FsMapping_unmap(FsMapping *this) {
	auto res = linux_munmap(this->data, this->size);
	if (res < 0) switch (res) {
		default:
			return FsRes_ErrUnknown;
	}

	return FsRes_Ok;
}

usize FsMapping_size(const FsMapping *this) {
	return this->size;
}

const ubyte *FsMapping_data(const FsMapping *this) {
	return this->data;
}
