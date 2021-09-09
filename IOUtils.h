#pragma once


extern bool myfeof(FILE * f);

#ifdef _WIN32
long long MyGetFileSize(FILE * handle);

FILE * MyOpenFileRWSeq(const wchar_t * filepath, bool overwrite);
#endif

#ifdef linux
size_t GetFileSize(FILE * handle);
#endif
