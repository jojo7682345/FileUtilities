#include <FileUtilities.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>

using namespace std;
using namespace filesystem;

struct FileHandle {
	char* fileName;
	char* path;
	char* fullPath;
	fstream* stream;
	FS_STREAM_STATE state;
};

void freeFileHandle(FileHandle fileHandle) {
	free(fileHandle.fileName);
	free(fileHandle.path);
	free(fileHandle.fullPath);
	delete fileHandle.stream;
}

FS_RESULT fsFileHandleCreate(const char* path, const char* fileName, FsFileHandle* handle) {
	//allocate fullPath on the heap.
	size_t pathSize = strlen(path);
	size_t nameSize = strlen(fileName);


	char* fullPathPtr = (char*)fsAllocate(pathSize + nameSize + 1);

	//populate fullpathPtr;
	memcpy(fullPathPtr, path, pathSize);
	memcpy(fullPathPtr + pathSize, fileName, nameSize);

	//allocate fileName an path on the heap.
	char* pathPtr = (char*)fsAllocate(pathSize + 1);
	char* filePtr = (char*)fsAllocate(nameSize + 1);
	memcpy(pathPtr, path, pathSize - 1 * (path[pathSize - 1] == '/'));
	memcpy(filePtr, fileName, nameSize);

	//allocate handle
	FileHandle* fHandle = (FileHandle*)fsAllocate(sizeof(FileHandle));

	//populate the fileHandle;
	fHandle->path = pathPtr;
	fHandle->fileName = filePtr;
	fHandle->fullPath = fullPathPtr;
	fHandle->state = FS_STREAM_STATE_CLOSED;

	*handle = fHandle;

	return FS_SUCCESS;
}

const char* fsFileHandleGetFileName(FsFileHandle handle) {
	return ((FileHandle*)handle)->fileName;
}

const char* fsFileHandleGetFilePath(FsFileHandle handle) {
	return ((FileHandle*)handle)->path;
}

const char* fsFileHandleGetFullPath(FsFileHandle handle) {
	return ((FileHandle*)handle)->fullPath;
}

FS_STREAM_STATE fsFileHandleGetState(FsFileHandle handle) {
	return ((FileHandle*)handle)->state;
}

FS_RESULT fsFileExists(bool& rExists, FsFileHandle handle) {
	FileHandle* fileHandle = ((FileHandle*)handle);
	rExists = exists(std::filesystem::path{ fileHandle->fullPath });
	return FS_SUCCESS;
}

FS_RESULT fsFileCreate(FsFileCreateInfo info, FsFileHandle* handle) {

	//retrieve fileHandle + data;
	FileHandle* fileHandle = ((FileHandle*)*handle);
	char* path = fileHandle->path;
	char* filename = fileHandle->fileName;
	char* fullPathPtr = fileHandle->fullPath;

	//retrieve directory and creation mode
	FS_DIRECTORY_MODE directoryMode = info.directoryMode;
	FS_FILE_CREATION_MODE creationMode = info.fileCreationMode;

	//create new directory if required.
	if (directoryMode == FS_DIRECTORY_MODE_CREATE_NEW_DIRECTORY) {
		if (!exists(std::filesystem::path{ path })) {
			try {
				std::error_code ec;
				if (!create_directories(path, ec)) {
					std::cout << ec.category().name() << std::endl;
					std::cout << ec.message() << std::endl;
					*handle = nullptr;
					return  FS_ERROR | FS_ERROR_CREATING_DIRECTORY;
				}
			} catch (const std::exception& e) {
				std::cerr << e.what() << '\n';
				abort();
			}
		}
	}

	//check for already existing file.
	if (exists({ fullPathPtr })) {
		if (info.fileCreationMode == FS_FILE_CREATION_MODE_ERROR_ON_EXISTING_FILE) {
			free(fullPathPtr);
			return FS_ERROR_EXISTING_FILE;
		}
	}

	//creating new file
	ofstream file = ofstream(fullPathPtr, ios::app);
	if (!file.good()) {
		free(fullPathPtr);
		file.close();
		return FS_ERROR | FS_ERROR_CREATING_FILE;
	}
	file.close();

	return FS_SUCCESS;
}

FS_RESULT fsFileOpenWrite(FsFileWriteInfo info, FsFileHandle* fileHandle) {
	//retrieve handle and data
	FileHandle* file = ((FileHandle*)*fileHandle);
	char* fullPathPtr = file->fullPath;

	//check if already open
	if (file->state || file->stream) {
		return FS_ERROR | FS_ERROR_FILE_ALREADY_OPEN;
	}

	//check if file exists.
	if (!exists({ fullPathPtr })) {
		return FS_ERROR_FILE_NOT_FOUND;
	}

	//open mode
	ios_base::openmode fileOpenMode =
		ios_base::binary | ios_base::out |
		((info.writeMode == FS_FILE_WRITE_MODE_MODIFY) ? ios::in : 0) |
		((info.openMode == FS_FILE_OPEN_MODE_AT_END) ? ios::ate : ios::beg);

	//open file
	file->stream = new fstream(fullPathPtr, fileOpenMode);
	if (!file->stream->is_open()) {
		return FS_ERROR;
	}
	if (info.openMode == FS_FILE_OPEN_MODE_AT_END) {
		file->stream->seekp(ios_base::end);
	}

	file->state = FS_STREAM_STATE_OPEN_WRITE;

	return FS_SUCCESS;
}
FS_RESULT fsFileWrite(void* data, size_t size, FsFileHandle* fileHandle) {
	//retrieve handle;
	FileHandle* file = ((FileHandle*)*fileHandle);

	//check if file is open;
	if (!file->state) {
		return FS_ERROR | FS_ERROR_FILE_NOT_OPEN;
	}

	//write data;
	(file->stream)->write((char*)data, size);

	return FS_SUCCESS;
}

FS_RESULT fsFileWriteBack(void* data, size_t size, FsFileHandle* fileHandle) {
	fsFileMoveWritePosition(true, size, FS_FILE_LOCATION_CURRENT, fileHandle);
	return fsFileWrite(data, size, fileHandle);
}

FS_RESULT fsFileClose(FsFileHandle* fileHandle) {
	//retrieve handle
	FileHandle* file = (FileHandle*)*fileHandle;

	//close read file
	if (file->stream && file->stream->is_open()) {
		file->stream->close();
		delete file->stream;
		file->stream = nullptr;
	}

	//close handle;
	file->state = FS_STREAM_STATE_CLOSED;

	return FS_SUCCESS;
}

void fsFileHandleDestroy(FsFileHandle handle) {
	freeFileHandle(*(FileHandle*)handle);
	free((FileHandle*)handle);
}

FS_RESULT fsFileOpenRead(FsFileReadInfo info, FsFileHandle* fileHandle) {
	//retrieve handle and data
	FileHandle* file = ((FileHandle*)*fileHandle);
	char* fullPathPtr = file->fullPath;

	//check if already open
	if (file->state || file->stream) {
		return FS_ERROR | FS_ERROR_FILE_ALREADY_OPEN;
	}

	//check if file exists.
	if (!exists({ fullPathPtr })) {
		return FS_ERROR_FILE_NOT_FOUND;
	}

	//open mode
	ios_base::openmode fileOpenMode = ios_base::binary | ios_base::in;

	if (info.openMode == FS_FILE_OPEN_MODE_AT_END) {
		fileOpenMode = fileOpenMode | ios_base::ate;
	}

	//open file
	file->stream = new fstream(fullPathPtr, fileOpenMode);
	if (!file->stream->is_open()) {
		return FS_ERROR;
	}
	if (info.openMode == FS_FILE_OPEN_MODE_AT_END) {
		file->stream->seekg(ios_base::end);
	}

	file->state = FS_STREAM_STATE_OPEN_READ;

	return FS_SUCCESS;
}

FS_RESULT fsFileReadBack(void* data, size_t size, FsFileHandle* fileHandle) {
	return
		fsFileMoveReadPosition(true, size, FS_FILE_LOCATION_CURRENT, fileHandle) |
		fsFileRead(data, size, fileHandle);
}

std::streampos tellPos(fstream* fstream, bool read) {
	if (read) {
		return fstream->tellg();
	}
	return fstream->tellp();
}

void seekPos(fstream* fstream, bool read, std::streamoff offset, ios_base::seekdir position) {
	if (read) {
		fstream->seekg(offset, position);
		return;
	}
	fstream->seekp(offset, position);
}

FS_RESULT fsFileRead(void* data, size_t size, FsFileHandle* fileHandle) {
	//retrieve handle;
	FileHandle* file = ((FileHandle*)*fileHandle);

	//check if file is open;
	if (!file->state) {
		return FS_ERROR | FS_ERROR_FILE_NOT_OPEN;
	}

	size_t fileSize;
	fsFileGetSize(fileSize, fileHandle);

	size_t currentPosition = (size_t)tellPos(file->stream, true);

	if (currentPosition + size > fileSize) {
		memset(data, 0, size);
		return FS_ERROR_READING_OUT_OF_BOUNDS;
	}

	//read data;
	file->stream->read((char*)data, size);

	return FS_SUCCESS;
}

FS_RESULT _fileMovePosition(bool sign, size_t amount, FS_FILE_LOCATION location, FsFileHandle* fileHandle, bool read) {
	FileHandle* file = ((FileHandle*)*fileHandle);
	auto& stream = file->stream;


	if (!file->state) {
		return FS_ERROR | FS_ERROR_FILE_NOT_OPEN;
	}
	if (file->stream) {
		long long offset = ((long long)amount) * (sign ? -1 : 1);
		streamoff current = tellPos(stream, read);

		size_t fileSize;
		fsFileGetSize(fileSize, fileHandle);

		ios::seekdir loc = ios::cur;
		if (location == FS_FILE_LOCATION_CURRENT) {
			loc = ios::beg;
			auto begOffset = current + offset;
			if (current + offset > (long long)fileSize) {
				begOffset = fileSize;
			}
			if (current + offset < 0) {
				begOffset = 0;
			}

			offset = begOffset;
		}
		if (location == FS_FILE_LOCATION_BEGIN) {
			loc = ios::beg;
			if (offset > (long long)fileSize) {
				offset = fileSize;
			}
			if (offset < 0) {
				offset = 0;
			}
		}
		if (location == FS_FILE_LOCATION_END) {
			loc = ios::end;
			if (offset > 0) {
				offset = fileSize;
			}
			if ((long long)fileSize + offset < 0) {
				offset = 0;
			}
		}
		seekPos(stream, read, offset, loc);
	}

	return FS_SUCCESS;
}


FS_RESULT fsFileGetSize(size_t& size, FsFileHandle* handle) {
	//retrieve handle;
	FileHandle* file = ((FileHandle*)*handle);

	//check if file is open;
	if (!file->state) {
		return FS_ERROR | FS_ERROR_FILE_NOT_OPEN;
	}

	bool read = (file->state == FS_STREAM_STATE_OPEN_READ) ? true : false;
	streamoff current = tellPos(file->stream, read);

	seekPos(file->stream, read, 0, ios::end);
	size = (size_t)tellPos(file->stream, read);

	seekPos(file->stream, read, current, ios::beg);

	return true;
}

FS_RESULT fsFileMoveReadPosition(bool sign, size_t amount, FS_FILE_LOCATION location, FsFileHandle* fileHandle) {
	return _fileMovePosition(sign, amount, location, fileHandle, true);
}

FS_RESULT fsFileMoveWritePosition(bool sign, size_t amount, FS_FILE_LOCATION location, FsFileHandle* fileHandle) {
	return _fileMovePosition(sign, amount, location, fileHandle, false);
}




void* allocateHeap(size_t size, int line, const char* file) {
	void* ptr = malloc(size);
	if (ptr == nullptr) {
		cout << "Out Of Memory LINE: " << line << " FILE: " << file << endl;
		abort();
	}
	memset(ptr, 0, size);
	return ptr;
}
