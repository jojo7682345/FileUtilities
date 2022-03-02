#include <FileStack.h>

void fsFileStackCreate(const char* filePath, const char* fileName, void** pData, size_t size, FsFileStack* fileStack) {
	fileStack->size = size;
	fileStack->data = pData;
	fileStack->file = (char*)fsAllocate(strlen(fileName) + 1);
	fileStack->path = (char*)fsAllocate(strlen(filePath) + 1);
	memcpy(fileStack->file, fileName, strlen(fileName));
	memcpy(fileStack->path, filePath, strlen(filePath));

	fsFileHandleCreate(fileStack->path, fileStack->file, &fileStack->handle);
}

FS_STACK_RESULT _FileStackGetCount(size_t& count, FsFileStack* fileStack) {
	FS_RESULT result;
	fsFileMoveReadPosition(false, 0, FS_FILE_LOCATION_END, &fileStack->handle);
	result = fsFileReadBack(&count, sizeof(size_t), &fileStack->handle);
	if (result != FS_SUCCESS) {
		count = 0;
		return FS_STACK_NEW_INSTANCES_REQUIRED | FS_STACK_ERROR_READING_FILE;
	}
	return FS_STACK_SUCCESS;
}

FS_STACK_RESULT fsFileStackGetCount(size_t& count, FsFileStack* fileStack) {
	bool exists;
	fsFileExists(exists, fileStack->handle);
	if (!exists) {
		count = 0;
		return FS_STACK_ERROR;
	}
	FsFileReadInfo readInfo{};
	readInfo.openMode = FS_FILE_OPEN_MODE_AT_END;
	FS_RESULT result;
	result = fsFileOpenRead(readInfo, &fileStack->handle);
	if (result != FS_SUCCESS) {
		count = 0;
		return FS_STACK_ERROR_OPENING_FILE;
	}
	_FileStackGetCount(count, fileStack);
	fsFileClose(&fileStack->handle);
	return FS_STACK_SUCCESS;
}

void fsFileStackDestroy(FsFileStack* fileStack) {
	free(fileStack->file);
	free(fileStack->path);
	fsFileHandleDestroy(fileStack->handle);
}

bool fsFileStackExists(FsFileStack fileStack) {
	bool exists;
	fsFileExists(exists, fileStack.handle);
	return exists;
}

FS_STACK_RESULT fsFileStackLoad(FsFileStack* fileStack, size_t readCount) {
	bool exists;
	fsFileExists(exists, fileStack->handle);
	if (!exists) {
		return FS_STACK_NEW_INSTANCES_REQUIRED;
	}
	FsFileReadInfo readInfo{};
	readInfo.openMode = FS_FILE_OPEN_MODE_AT_BEGIN;
	FS_RESULT result;
	result = fsFileOpenRead(readInfo, &fileStack->handle);
	if (result != FS_SUCCESS) {

		return FS_STACK_NEW_INSTANCES_REQUIRED | FS_STACK_ERROR_OPENING_FILE;
	}
	size_t count;
	_FileStackGetCount(count, fileStack);
	if (readCount > count) {
		return FS_STACK_BUFFER_OVERRUN | FS_STACK_ERROR;
	}
	fsFileMoveReadPosition(true, sizeof(size_t), FS_FILE_LOCATION_END, &fileStack->handle);
	for (size_t i = 0; i < readCount; i++) {
		void* writePos = (((char*)(*fileStack->data)) + (readCount - i - 1) * fileStack->size);
		result = fsFileReadBack(writePos, fileStack->size, &fileStack->handle);
		fsFileMoveReadPosition(true, fileStack->size, FS_FILE_LOCATION_CURRENT, &fileStack->handle);
		if (result != FS_SUCCESS) {
			return FS_STACK_NEW_INSTANCES_REQUIRED | FS_STACK_ERROR_READING_FILE;
		}
	}
	fsFileClose(&fileStack->handle);
	return FS_STACK_SUCCESS;
}

FS_STACK_RESULT fsFileStackSave(FsFileStack* fileStack, size_t count) {
	size_t currentCount;
	fsFileStackGetCount(currentCount, fileStack);

	bool exists;
	fsFileExists(exists, fileStack->handle);
	FS_RESULT result;
	if (!exists) {
		FsFileCreateInfo createInfo{};
		createInfo.directoryMode = FS_DIRECTORY_MODE_CREATE_NEW_DIRECTORY;
		createInfo.fileCreationMode = FS_FILE_CREATION_MODE_NO_ERROR_ON_EXISTING_FILE;
		result = fsFileCreate(createInfo, &fileStack->handle);
		if (result != FS_SUCCESS) {
			return FS_STACK_ERROR_CREATING_FILE;
		}
	}
	FsFileWriteInfo writeInfo{};
	writeInfo.openMode = FS_FILE_OPEN_MODE_AT_BEGIN;
	writeInfo.writeMode = FS_FILE_WRITE_MODE_MODIFY;
	result = fsFileOpenWrite(writeInfo, &fileStack->handle);
	if (result != FS_SUCCESS) {
		return FS_STACK_ERROR_OPENING_FILE;
	}

	//move position back in order to overwrite previous size at the end of the file
	fsFileMoveWritePosition(true, sizeof(size_t), FS_FILE_LOCATION_END, &fileStack->handle);

	//write data
	for (size_t i = 0; i < count; i++) {
		//reverse order so the most recent entries end up at the end of the file
		//void* _dataPtr = (((char*)(*fileStack->data)) + (count - i - 1)*fileStack->size);
		void* _dataPtr = ((char*)*fileStack->data) + (i * fileStack->size);
		//write
		result = fsFileWrite(_dataPtr, fileStack->size, &fileStack->handle);
		if (result != FS_SUCCESS) {
			return FS_STACK_ERROR_WRITING_FILE;
		}
	}

	//temp
	//fsFileMoveWritePosition(false, 0, FS_FILE_LOCATION_END, &fileStack->handle);
	count += currentCount;
	result = fsFileWrite(&count, sizeof(size_t), &fileStack->handle);
	if (result != FS_SUCCESS) {
		return FS_STACK_ERROR_WRITING_FILE;
	}

	return FS_STACK_SUCCESS;
}
