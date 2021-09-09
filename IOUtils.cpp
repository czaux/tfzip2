#include <share.h> 
#include <io.h>
#include <stdio.h>
#include "IOUtils.h"

bool myfeof(FILE * f)
{
	int c = fgetc(f);
	if (c == EOF) return true;
	ungetc(c, f);
	return false;
}

#ifdef _WIN32
long long MyGetFileSize(FILE * handle)
{
	int fd = _fileno(handle);
	long long filesize = _filelengthi64(fd);
	return filesize;
}

FILE * MyOpenFileRWSeq(const wchar_t * filepath, bool overwrite)
{
	wchar_t * mode = L"r+bS";
	if (overwrite) mode = L"w+bS";

	FILE * stream = _wfsopen(
		filepath,
		mode,
		_SH_DENYWR
	);
	return stream;
}
#endif

#ifdef linux
size_t GetFileSize(FILE * handle)
{
	//TODO
	throw "NOT IMPLIMENTED";
}
#endif