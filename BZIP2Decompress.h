#pragma once
#include "../hash-library/sha1.h"
#include "WorkerException.h"
#include "IOUtils.h"
#include "tfbzlib.h"

extern void asyncBZ2DeCompress(const char* file_name, const char* ouerfile);

extern void simpledecompress(const char* file_name, const char* ouerfile);

extern void simpleBZ2DeCompress(const QString file_name, std::function<void(int)> progressCallback, std::string *ExtractedSourceFileHash);

extern bool uncompressStream(QString filepath);

extern bool validateArchive(const QString &inpath, std::function<void(int)> progressCallback, std::string *SourceFileHash);