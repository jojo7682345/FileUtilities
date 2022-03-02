#pragma once
#ifndef _FILE_UTILITIES_INCLUDED_
#include <FileUtilities.h>
#endif
#include <iostream>

typedef uint32_t FS_STACK_RESULT;
const FS_STACK_RESULT FS_STACK_SUCCESS = 0b0;
const FS_STACK_RESULT FS_STACK_NEW_INSTANCES_REQUIRED = 0b1;
const FS_STACK_RESULT FS_STACK_ERROR = 0b10;
const FS_STACK_RESULT FS_STACK_ERROR_OPENING_FILE = 0b110;
const FS_STACK_RESULT FS_STACK_ERROR_READING_FILE = 0b1010;
const FS_STACK_RESULT FS_STACK_ERROR_CREATING_FILE = 0b10010;
const FS_STACK_RESULT FS_STACK_ERROR_WRITING_FILE = 0b100010;
const FS_STACK_RESULT FS_STACK_BUFFER_OVERRUN = 0b1000000;

struct FsFileStack {
	FsFileHandle handle;
	void** data;
	size_t size;
	char* path;
	char* file;
};

template<typename T>
inline void fsFileStackCreate(const char* filePath, const char* fileName, T** pData, FsFileStack* fileStack) {
	fsFileStackCreate(filePath, fileName, (void**)pData, sizeof(T), fileStack);
}
void fsFileStackCreate(const char* filePath, const char* fileName, void** pData, size_t size, FsFileStack* fileStack);

FS_STACK_RESULT fsFileStackGetCount(size_t& count, FsFileStack* fileStack);

void fsFileStackDestroy(FsFileStack* fileStack);

bool fsFileStackExists(FsFileStack fileStack);

FS_STACK_RESULT fsFileStackLoad(FsFileStack* fileStack, size_t count);

FS_STACK_RESULT fsFileStackSave(FsFileStack* fileStack, size_t count);