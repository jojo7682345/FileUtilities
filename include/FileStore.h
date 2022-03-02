#pragma once
#ifndef _FILE_UTILITIES_INCLUDED_
#include "FileUtilities.h"
#endif
#include <iostream>

typedef uint32_t FS_STORE_RESULT;
const FS_STORE_RESULT FS_STORE_SUCCESS = 0b0;
const FS_STORE_RESULT FS_STORE_NEW_INSTANCES_REQUIRED = 0b1;
const FS_STORE_RESULT FS_STORE_ERROR = 0b10;
const FS_STORE_RESULT FS_STORE_ERROR_OPENING_FILE = 0b110;
const FS_STORE_RESULT FS_STORE_ERROR_READING_FILE = 0b1010;
const FS_STORE_RESULT FS_STORE_ERROR_CREATING_FILE = 0b10010;
const FS_STORE_RESULT FS_STORE_ERROR_WRITING_FILE = 0b100010;
const FS_STORE_RESULT FS_STORE_BUFFER_OVERRUN = 0b1000000;

struct FsFileStore {
	FsFileHandle handle;
	void** data;
	size_t size;
	char* path;
	char* file;
};

template<typename T>
inline void fsFileStoreCreate(const char* filePath, const char* fileName, T** pData, FsFileStore* fileStore) {
	fsFileStoreCreate(filePath, fileName, (void**)pData, sizeof(T), fileStore);
}
void fsFileStoreCreate(const char* filePath, const char* fileName, void** pData, size_t size, FsFileStore* fileStore);

FS_STORE_RESULT fsFileStoreGetCount(size_t& count, FsFileStore* fileStore);

void fsFileStoreDestroy(FsFileStore* fileStore);

bool fsFileStoreExists(FsFileStore fileStore);

FS_STORE_RESULT fsFileStoreLoad(FsFileStore* fileStore);

FS_STORE_RESULT fsFileStoreSave(FsFileStore* fileStore, size_t count);

