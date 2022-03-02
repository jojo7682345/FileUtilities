#pragma once
#include <cinttypes>

#define _FILE_UTILITIES_INCLUDED_

typedef uint32_t FS_RESULT;
const FS_RESULT FS_SUCCESS = 0b0;
const FS_RESULT FS_ERROR = 0b1;
const FS_RESULT FS_ERROR_NO_DIRECTORY = 0b11;
const FS_RESULT FS_ERROR_EXISTING_FILE = 0b101;
const FS_RESULT FS_ERROR_FILE_NOT_FOUND = 0b1001;
const FS_RESULT FS_ERROR_CREATING_DIRECTORY = 0b10001;
const FS_RESULT FS_ERROR_CREATING_FILE = 0b100001;
const FS_RESULT FS_ERROR_FILE_ALREADY_OPEN = 0b1000001;
const FS_RESULT FS_ERROR_FILE_NOT_OPEN = 0b10000001;
const FS_RESULT FS_FILE_EMPTY = 0b100000000;
const FS_RESULT FS_ERROR_READING_OUT_OF_BOUNDS = 0b1000000001;

typedef uint32_t FS_DIRECTORY_MODE;
const FS_DIRECTORY_MODE FS_DIRECTORY_MODE_ERROR_ON_NO_DIRECTORY = 0;
const FS_DIRECTORY_MODE FS_DIRECTORY_MODE_CREATE_NEW_DIRECTORY = 1;

typedef uint32_t FS_FILE_CREATION_MODE;
const FS_FILE_CREATION_MODE FS_FILE_CREATION_MODE_ERROR_ON_EXISTING_FILE = 0;
const FS_FILE_CREATION_MODE FS_FILE_CREATION_MODE_NO_ERROR_ON_EXISTING_FILE = 1;

struct FsFileCreateInfo {
	FS_DIRECTORY_MODE directoryMode;
	FS_FILE_CREATION_MODE fileCreationMode;
};

typedef uint32_t FS_FILE_OPEN_MODE;
const FS_FILE_OPEN_MODE FS_FILE_OPEN_MODE_AT_BEGIN = 0;
const FS_FILE_OPEN_MODE FS_FILE_OPEN_MODE_AT_END = 1;

typedef uint32_t FS_FILE_WRITE_MODE;
const FS_FILE_WRITE_MODE FS_FILE_WRITE_MODE_REPLACE = 0;
const FS_FILE_WRITE_MODE FS_FILE_WRITE_MODE_MODIFY = 1;


typedef uint32_t FS_FILE_LOCATION;
const FS_FILE_LOCATION FS_FILE_LOCATION_CURRENT = 0;
const FS_FILE_LOCATION FS_FILE_LOCATION_END = 1;
const FS_FILE_LOCATION FS_FILE_LOCATION_BEGIN = 2;

struct FsFileWriteInfo {
	FS_FILE_OPEN_MODE openMode;
	FS_FILE_WRITE_MODE writeMode;
};

struct FsFileReadInfo {
	FS_FILE_OPEN_MODE openMode;
};

typedef uint32_t FS_STREAM_STATE;
const FS_STREAM_STATE FS_STREAM_STATE_CLOSED = 0;
const FS_STREAM_STATE FS_STREAM_STATE_OPEN_WRITE = 1;
const FS_STREAM_STATE FS_STREAM_STATE_OPEN_READ = 2;

typedef void* FsFileHandle;

FS_RESULT fsFileHandleCreate(const char* path, const char* fileName, FsFileHandle* fileHandle);
const char* fsFileHandleGetFileName(FsFileHandle handle);
const char* fsFileHandleGetFilePath(FsFileHandle handle);
const char* fsFileHandleGetFullPath(FsFileHandle handle);
FS_STREAM_STATE fsFileHandleGetState(FsFileHandle handle);
void fsFileHandleDestroy(FsFileHandle handle);

FS_RESULT fsFileExists(bool& exists, FsFileHandle handle);
FS_RESULT fsFileGetSize(size_t& size, FsFileHandle* handle);
FS_RESULT fsFileCreate(FsFileCreateInfo info, FsFileHandle* fileHandle);

FS_RESULT fsFileOpenWrite(FsFileWriteInfo info, FsFileHandle* fileHandle);
FS_RESULT fsFileOpenRead(FsFileReadInfo info, FsFileHandle* fileHandle);

FS_RESULT fsFileRead(void* data, size_t size, FsFileHandle* fileHandle);
FS_RESULT fsFileReadBack(void* data, size_t size, FsFileHandle* fileHandle);

FS_RESULT fsFileMoveWritePosition(bool sign, size_t amount, FS_FILE_LOCATION location, FsFileHandle* fileHandle);

FS_RESULT fsFileMoveReadPosition(bool sign, size_t amount, FS_FILE_LOCATION location, FsFileHandle* fileHandle);

FS_RESULT fsFileWrite(void* data, size_t size, FsFileHandle* fileHandle);
FS_RESULT fsFileWriteBack(void* data, size_t size, FsFileHandle* fileHandle);

FS_RESULT fsFileClose(FsFileHandle* fileHandle);

void* allocateHeap(size_t size, int line, const char* file);
#define fsAllocate(size) allocateHeap(size,__LINE__,__FILE__)