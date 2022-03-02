#include <FileStore.h>
#include <iostream>
#include <FileStore.h>

void fsFileStoreCreate(const char* filePath, const char* fileName, void** pData, size_t size, FsFileStore* fileStore) {
	fileStore->size = size;
	fileStore->data = pData;
	fileStore->file = (char*)fsAllocate(strlen(fileName) + 1);
	fileStore->path = (char*)fsAllocate(strlen(filePath) + 1);
	memcpy(fileStore->file, fileName, strlen(fileName));
	memcpy(fileStore->path, filePath, strlen(filePath));

	fsFileHandleCreate(fileStore->path, fileStore->file, &fileStore->handle);
}

FS_STORE_RESULT fsFileStoreGetCount(size_t& count, FsFileStore* fileStore) {
	bool exists;
	fsFileExists(exists, fileStore->handle);
	if (!exists) {
		count = 0;
		return FS_STORE_ERROR;
	}
	FsFileReadInfo readInfo{};
	FS_RESULT result;
	result = fsFileOpenRead(readInfo, &fileStore->handle);
	if (result != FS_SUCCESS) {
		count = 0;
		return FS_STORE_ERROR_OPENING_FILE;
	}
	result = fsFileRead(&count, sizeof(size_t), &fileStore->handle);
	if (result != FS_SUCCESS) {
		count = 0;
		return FS_STORE_ERROR_READING_FILE;
	}
	fsFileClose(&fileStore->handle);
	return FS_STORE_SUCCESS;
}

void fsFileStoreDestroy(FsFileStore* fileStore) {
	free(fileStore->file);
	free(fileStore->path);
	fsFileHandleDestroy(fileStore->handle);
}

bool fsFileStoreExists(FsFileStore fileStore) {
	bool exists;
	fsFileExists(exists, fileStore.handle);
	return exists;
}

FS_STORE_RESULT fsFileStoreLoad(FsFileStore* fileStore) {
	bool exists;
	fsFileExists(exists, fileStore->handle);
	if (!exists) {
		return FS_STORE_NEW_INSTANCES_REQUIRED;
	}
	FsFileReadInfo readInfo{};
	FS_RESULT result;
	result = fsFileOpenRead(readInfo, &fileStore->handle);
	if (result != FS_SUCCESS) {
		return FS_STORE_NEW_INSTANCES_REQUIRED | FS_STORE_ERROR_OPENING_FILE;
	}
	size_t count;
	result = fsFileRead(&count, sizeof(size_t), &fileStore->handle);
	if (result != FS_SUCCESS) {
		return FS_STORE_NEW_INSTANCES_REQUIRED | FS_STORE_ERROR_READING_FILE;
	}
	result = fsFileRead(*fileStore->data, fileStore->size * count, &fileStore->handle);
	if (result != FS_SUCCESS) {
		return FS_STORE_NEW_INSTANCES_REQUIRED | FS_STORE_ERROR_READING_FILE;
	}
	fsFileClose(&fileStore->handle);
	return FS_STORE_SUCCESS;
}

FS_STORE_RESULT fsFileStoreSave(FsFileStore* fileStore, size_t count) {
	bool exists;
	fsFileExists(exists, fileStore->handle);
	FS_RESULT result;
	if (!exists) {
		FsFileCreateInfo createInfo{};
		createInfo.directoryMode = FS_DIRECTORY_MODE_CREATE_NEW_DIRECTORY;
		createInfo.fileCreationMode = FS_FILE_CREATION_MODE_NO_ERROR_ON_EXISTING_FILE;
		result = fsFileCreate(createInfo, &fileStore->handle);
		if (result != FS_SUCCESS) {
			return FS_STORE_ERROR_CREATING_FILE;
		}
	}
	FsFileWriteInfo writeInfo{};
	writeInfo.openMode = FS_FILE_OPEN_MODE_AT_BEGIN;
	result = fsFileOpenWrite(writeInfo, &fileStore->handle);
	if (result != FS_SUCCESS) {
		return FS_STORE_ERROR_OPENING_FILE;
	}

	result = fsFileWrite(&count, sizeof(size_t), &fileStore->handle);
	if (result != FS_SUCCESS) {
		return FS_STORE_ERROR_WRITING_FILE;
	}

	result = fsFileWrite(*fileStore->data, fileStore->size * count, &fileStore->handle);
	if (result != FS_SUCCESS) {
		return FS_STORE_ERROR_WRITING_FILE;
	}

	return FS_STORE_SUCCESS;
}


