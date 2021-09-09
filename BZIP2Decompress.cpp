#include <fstream>
#include <iostream>
#include <chrono>
#include <future>
#include <QDebug>
#include "../bzip2-1.0.6//bzlib.h"
#include <algorithm>
#include "BZIP2Decompress.h"

//Inline function for our async file reader. Simply reads new chunk of the file into secondary buffers by pointer.
int asyncFileRead2(std::vector<char> &newchunk, std::ifstream &filer, size_t &length, int bigbufsize) {
	length = filer.read(&newchunk[0], bigbufsize).gcount();
	if (length > 0)
	{
		return 1;
	}
	else {
		return 2;
	}
};

int asyncFileWrite2(std::vector<char> &newchunk, std::ofstream &filer, int64_t bufsize) {
	filer.write(newchunk.data(), bufsize);
	filer.flush();

	//TODO error handle
	return 1;
};

bool validateArchive(const QString &inpath, std::function<void(int)> progressCallback, std::string *SourceFileHash)
{
	BZFILE* bzf = NULL;
	int   bzerr, bzerr_dummy, ret, nread, streamNo, i;
	unsigned char   obuf[5000];
	unsigned char   unused[BZ_MAX_UNUSED];
	int   nUnused;
	void*   unusedTmpV;
	unsigned char*  unusedTmp;

	nUnused = 0;
	streamNo = 0;
	int verbosity = 0;
	int smallMode = 0;
	unsigned long long readlen = 0;
	const std::wstring infilepath = inpath.toStdWString();
	FILE * zStream = MyOpenFileRWSeq(infilepath.c_str(), false);
	if (zStream == NULL)
	{
		throw new WorkerException("IOERROR1er");
	}
	if (ferror(zStream)) goto errhandler_io;

	size_t filesize = static_cast<size_t>(MyGetFileSize(zStream));
	size_t readbytes = 0;

	while (true) {

		bzf = BZ2_bzReadOpen(
			&bzerr, zStream, verbosity,
			(int)smallMode, unused, nUnused
		);
		if (bzf == NULL || bzerr != BZ_OK) goto errhandler;
		streamNo++;

		while (bzerr == BZ_OK) {
			nread = TFZ2_bzRead(&bzerr, bzf, obuf, 5000, readlen);
			if (bzerr == BZ_DATA_ERROR_MAGIC) goto errhandler;
			if (nread > 0)
			{
				readbytes += nread;
				int progress = std::max<int>(0, std::min<int>(1000, (readbytes * 1000 / filesize)));
				progressCallback(progress);
			}
		}
		if (bzerr != BZ_STREAM_END) goto errhandler;

		BZ2_bzReadGetUnused(&bzerr, bzf, &unusedTmpV, &nUnused);
		if (bzerr != BZ_OK)
		{
			throw("test:bzReadGetUnused");
		}

		unusedTmp = (unsigned char*)unusedTmpV;
		for (i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

		BZ2_bzReadClose(&bzerr, bzf);
		if (bzerr != BZ_OK)
		{
			throw("test:bzReadGetUnused");
		}
		if (nUnused == 0 && myfeof(zStream)) break;

	}

	if (ferror(zStream)) goto errhandler_io;
	ret = fclose(zStream);
	if (ret == EOF) goto errhandler_io;

	return true;

errhandler:
	BZ2_bzReadClose(&bzerr_dummy, bzf);
	switch (bzerr) {
	case BZ_CONFIG_ERROR:
		//configError(); 
		break;
	case BZ_IO_ERROR:
	errhandler_io:
		//ioError(); 
		break;
	case BZ_DATA_ERROR:
		fprintf(stderr,
			"data integrity (CRC) error in data\n");
		return false;
	case BZ_MEM_ERROR:
		//outOfMemory();
		throw "outofmemory";
	case BZ_UNEXPECTED_EOF:
		fprintf(stderr,
			"file ends unexpectedly\n");
		return false;
	case BZ_DATA_ERROR_MAGIC:
		if (zStream != stdin) fclose(zStream);
		if (streamNo == 1) {
			fprintf(stderr,
				"bad magic number (file not created by bzip2)\n");
			return false;
		}
		else {
			/*if (noisy)
				fprintf(stderr,
					"trailing garbage after EOF ignored\n");*/
			return true;
		}
	default:
		throw("test:unexpected error");
	}

	throw("test:end");
	return true; /*notreached*/
}


  /*
BZFILE* bzReadOpen(
	int*  bzerror,
	int   verbosity,
	int   small,
	void* unused,
	int   nUnused)
{
	bzFile* bzf = new BZFILE();
	int     ret;

	bzf->lastErr = BZ_OK;

	if ((small != 0 && small != 1) ||
		(verbosity < 0 || verbosity > 4) ||
		(unused == NULL && nUnused != 0) ||
		(unused != NULL && (nUnused < 0 || nUnused > BZ_MAX_UNUSED)))
	{
		bzf->lastErr = BZ_PARAM_ERROR;
		return bzf;
	};

	//if (ferror(f))
	//   { BZ_SETERR(BZ_IO_ERROR); return NULL; };

	bzf = (bzFile*)malloc(sizeof(bzFile));
	if (bzf == NULL)
	{
		bzf->lastErr = BZ_MEM_ERROR;
		return bzf;
	};

	bzf->lastErr = BZ_OK;

	bzf->initialisedOk = false;
	//bzf->handle        = f;
	bzf->bufN = 0;
	bzf->writing = false;
	bzf->strm.bzalloc = NULL;
	bzf->strm.bzfree = NULL;
	bzf->strm.opaque = NULL;

	while (nUnused > 0) {
		bzf->buf[bzf->bufN] = *((unsigned char *)(unused)); bzf->bufN++;
		unused = ((void*)(1 + ((unsigned char*)(unused))));
		nUnused--;
	}

	ret = BZ2_bzDecompressInit(&(bzf->strm), verbosity, small);
	if (ret != BZ_OK)
	{
		bzf->lastErr = ret; free(bzf); return bzf;
	};

	bzf->strm.avail_in = bzf->bufN;
	bzf->strm.next_in = bzf->buf;

	bzf->initialisedOk = true;
	return bzf;
}	*/

/*---------------------------------------------------*/
/*
void bzReadClose(int *bzerror, BZFILE *b)
{
	bzFile* bzf = (bzFile*)b;

	bzf->lastErr = BZ_OK;

	if (bzf == NULL)
	{
		bzf->lastErr = BZ_OK;
		return;
	};

	if (bzf->writing)
	{
		bzf->lastErr = BZ_SEQUENCE_ERROR;
		return;
	};

	if (bzf->initialisedOk)
		(void)BZ2_bzDecompressEnd(&(bzf->strm));
	free(bzf);
} */

/*
int bzRead(	int*    bzerror,
	BZFILE* b,
	FILE * filer,
	std::vector<char> &chunkdata,
	unsigned char*   buf,
	int     len)
{
	int   n, ret;
	bzFile* bzf = (bzFile*)b;

	bzf->lastErr = BZ_OK;

	if (bzf == NULL || buf == NULL || len < 0)
	{
		bzf->lastErr = BZ_PARAM_ERROR;
		 return 0;
	};

	if (bzf->writing)
	{
		bzf->lastErr = BZ_SEQUENCE_ERROR;
		 return 0;
	};

	if (len == 0)
	{
		bzf->lastErr = BZ_OK;
		return 0;
	};

	bzf->strm.avail_out = len;
	bzf->strm.next_out = buf;
	char *hier;
	unsigned int position = 0;
	while (true) {

		if (bzf->strm.avail_in == 0) {

			//int restsize = std::min<int>(BZ_MAX_UNUSED, chunkdata.size() - position);
			
			//hier = (char*)chunkdata.data() + position;
			//position += restsize;

			n = fread(bzf->buf, 1,
				BZ_MAX_UNUSED, filer);
			if (ferror(filer))
			{
				bzf->lastErr = BZ_IO_ERROR;
				return 0;
			};

			bzf->bufN = n;
			bzf->strm.avail_in = bzf->bufN;
			bzf->strm.next_in = bzf->buf;
			
		}

		ret = BZ2_bzDecompress(&(bzf->strm));

		if (ret != BZ_OK && ret != BZ_STREAM_END)
		{
			bzf->lastErr = ret;
			return 0;
		};

		if (ret == BZ_OK && bzf->strm.avail_in == 0 && bzf->strm.avail_out > 0)
		{
			bzf->lastErr = BZ_UNEXPECTED_EOF;
			return 0;
		};

		if (ret == BZ_STREAM_END)
		{
			bzf->lastErr = BZ_STREAM_END;
			return len - bzf->strm.avail_out;
		};

		if (bzf->strm.avail_out == 0)
		{
			bzf->lastErr = BZ_OK;
			return len;
		};

	}

	return 0; 
}	*/


/*---------------------------------------------------*/
/*
int BZ2_bzRead3
(int*    bzerror,
	std::ifstream &file,
	BZFILE* b,
	unsigned char*   buf,
	int     len,
	uint64_t * allread)
{
	int   n, ret;
	bzFile* bzf = (bzFile*)b;

	BZ_SETERR(BZ_OK);

	if (bzf == NULL || buf == NULL || len < 0)
	{
		BZ_SETERR(BZ_PARAM_ERROR); return 0;
	};

	if (bzf->writing)
	{
		BZ_SETERR(BZ_SEQUENCE_ERROR); return 0;
	};

	if (len == 0)
	{
		BZ_SETERR(BZ_OK); return 0;
	};

	bzf->strm.avail_out = len;
	bzf->strm.next_out = buf;
	while (true) {
		if (file.fail() && !file.eof())
		{
			BZ_SETERR(BZ_IO_ERROR); return 0;
		}
		//if (ferror(bzf->handle))
		//{
		//	BZ_SETERR(BZ_IO_ERROR); return 0;
		//};	  

		if (bzf->strm.avail_in == 0 && !file.eof()) {

			n = file.read(reinterpret_cast<char *>(bzf->buf), BZ_MAX_UNUSED).gcount();
			*(allread) += n;
			//n = fread(bzf->buf, sizeof(unsigned char),
			//	BZ_MAX_UNUSED, bzf->handle);
			if (file.fail() && !file.eof())
			{
				BZ_SETERR(BZ_IO_ERROR); return 0;
			}
			//if (ferror(bzf->handle))
			//{
			//	BZ_SETERR(BZ_IO_ERROR); return 0;
			//}; 
			bzf->bufN = n;
			bzf->strm.avail_in = bzf->bufN;
			bzf->strm.next_in = bzf->buf;
		}

		ret = BZ2_bzDecompress(&(bzf->strm));

		if (ret != BZ_OK && ret != BZ_STREAM_END)
		{
			BZ_SETERR(ret); return 0;
		};

		if (ret == BZ_OK && file.eof() &&
			bzf->strm.avail_in == 0 && bzf->strm.avail_out > 0)
		{
			BZ_SETERR(BZ_UNEXPECTED_EOF); return 0;
		};

		if (ret == BZ_STREAM_END)
		{
			BZ_SETERR(BZ_STREAM_END);
			return len - bzf->strm.avail_out;
		};
		if (bzf->strm.avail_out == 0)
		{
			BZ_SETERR(BZ_OK); return len;
		};

	}

	return 0;
}	   */


/*---------------------------------------------------*/
/*
int BZ2_bzRead2(int*    bzerror,
	BZFILE* b,
	std::vector<char> &chunkdata,
	uint64_t startlen,
	unsigned char*   buf,
	int     len)
{
	int   n, ret;
	bzFile* bzf = (bzFile*)b;

	BZ_SETERR(BZ_OK);

	if (bzf == NULL || buf == NULL || len < 0)
	{
		BZ_SETERR(BZ_PARAM_ERROR); return 0;
	};

	if (bzf->writing)
	{
		BZ_SETERR(BZ_SEQUENCE_ERROR); return 0;
	};

	if (len == 0)
	{
		BZ_SETERR(BZ_OK); return 0;
	};

	bzf->strm.avail_out = len;
	bzf->strm.next_out = buf;
	unsigned int position = 0;
	while (true) {

		//if (ferror(bzf->handle))
		//{
		//	BZ_SETERR(BZ_IO_ERROR); return 0;
		//}; 

		if (bzf->strm.avail_in == 0 && position < chunkdata.size()) {

			int restsize = std::min<int>(BZ_MAX_UNUSED, chunkdata.size() - position);

			//
			////hier = (char*)chunkdata.data() + position;
			//n = fread(bzf->buf, 1,
			//	BZ_MAX_UNUSED, bzf->handle);
			//if (ferror(bzf->handle))
			//{
			//	BZ_SETERR(BZ_IO_ERROR); return 0;
			//};

			bzf->bufN = restsize;
			bzf->strm.avail_in = bzf->bufN;
			bzf->strm.next_in = (unsigned char*)chunkdata.data() + startlen + position;
			position += restsize;
		}

		ret = BZ2_bzDecompress(&(bzf->strm));

		if (ret != BZ_OK && ret != BZ_STREAM_END)
		{
			BZ_SETERR(ret); return 0;
		};

		//
		//if (ret == BZ_OK && position >= chunkdata.size() &&
		//	bzf->strm.avail_in == 0 && bzf->strm.avail_out > 0)
		//{
		//	BZ_SETERR(BZ_UNEXPECTED_EOF); return 0;
		//};

		if (ret == BZ_STREAM_END)
		{
			BZ_SETERR(BZ_STREAM_END);
			return len - bzf->strm.avail_out;
		};
		if (bzf->strm.avail_out == 0)
		{
			BZ_SETERR(BZ_OK); return len;
		};

	}

	return 0; 
}	*/

/*---------------------------------------------------*/
/*
void BZ2_bzReadGetUnused2(int*    bzerror,
	BZFILE* b,
	unsigned char**  unused,
	int*    nUnused)
{
	bzFile* bzf = (bzFile*)b;
	if (bzf == NULL)
	{
		BZ_SETERR(BZ_PARAM_ERROR); return;
	};
	if (bzf->lastErr != BZ_STREAM_END)
	{
		BZ_SETERR(BZ_SEQUENCE_ERROR); return;
	};
	if (unused == NULL || nUnused == NULL)
	{
		BZ_SETERR(BZ_PARAM_ERROR); return;
	};

	BZ_SETERR(BZ_OK);
	*nUnused = bzf->strm.avail_in;
	*unused = bzf->strm.next_in;
}	   */





   /*
bool uncompressStream(QString filepath)
{
	
	#define BZ_MAX_UNUSED 5000

	std::ifstream zStream;
	zStream.open(filepath.toStdWString(), std::ios::binary | std::ios::in);
	//std::string asdf = filepath.toStdString();
	//FILE *zStream;
	//zStream = fopen(asdf.c_str(), "r+b");

	bzReadFile* bzf = NULL;
	int   bzerr, bzerr_dummy, ret, nread, streamNo, i;
	char   obuf[5000];
	unsigned char   unused[BZ_MAX_UNUSED];
	int   nUnused;
	void*   unusedTmpV;
	unsigned char*  unusedTmp;

	nUnused = 0;
	streamNo = 0;

	//int retVal = setmode(fileno(zStream),O_BINARY);
	//int retVal2 = setmode(fileno(stream), O_BINARY);

	//if (ferror(stream)) goto errhandler_io;
	if (zStream.fail() || zStream.bad() || !zStream.is_open() || zStream.eof())
	{
		goto errhandler_io;
	}

	while (true) {

		bzf = bzReadOpen(
			&bzerr, &zStream, 0,
			0, unused, nUnused
		);
		if (bzf == NULL || bzerr != BZ_OK)
		{
			goto errhandler;
		}
		streamNo++;

		while (bzerr == BZ_OK) {
			nread = bzRead(&bzerr, bzf, obuf, 5000);
			if (bzerr == BZ_DATA_ERROR_MAGIC) goto trycat;
			if ((bzerr == BZ_OK || bzerr == BZ_STREAM_END) && nread > 0)
			{
				
			}
				//fwrite(obuf, sizeof(unsigned char), nread, stream);
			//if (ferror(stream)) goto errhandler_io;
		}
		if (bzerr != BZ_STREAM_END)
		{
			goto errhandler;
		}

		bzReadGetUnused(&bzerr, bzf, &unusedTmpV, &nUnused);
		if (bzerr != BZ_OK)
		{
			throw("decompress:bzReadGetUnused");
		}

		unusedTmp = (unsigned char*)unusedTmpV;
		for (i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

		bzReadClose(&bzerr, bzf);
		if (bzerr != BZ_OK)
		{
			throw("decompress:bzReadGetUnused");
		}

		if (nUnused == 0 && zStream.eof()) break;
	}

closeok:
	if ((zStream.fail() && !zStream.eof()) || zStream.bad()) goto errhandler_io;

	zStream.close();
	return true;

trycat:
	if (true) {
	}

errhandler:
	bzReadClose(&bzerr_dummy, bzf);
	switch (bzerr) {
	case BZ_CONFIG_ERROR:
		 break;
	case BZ_IO_ERROR:
	errhandler_io:
		break;
	case BZ_DATA_ERROR:
		break;
	case BZ_MEM_ERROR:
		break;
	case BZ_UNEXPECTED_EOF:
		break;
	case BZ_DATA_ERROR_MAGIC:
		//if (zStream != stdin) fclose(zStream);
		//if (stream != stdout) fclose(stream);
		if (streamNo == 1) {
			return false;
		}
		else {
			return true;
		}
	default:
		throw("decompress:unexpected error");
	}

	throw("decompress:end");
	return true;
	
}  */
/*	  
void DecompressCleanup(std::ifstream *ifile, bzReadFile *bzf)
{
	
	int bzerr;
	ifile->close();
	bzReadClose(&bzerr, bzf);
	//force free if close doesn't work
	if (bzerr != BZ_OK)
	{
		int ret = BZ2_bzDecompressEnd(&(bzf->strm));
		if (ret != BZ_OK) throw new WorkerException("CANT FREE");
	}
	
}  */


bool simpleBZ2Validate(const QString &file_name, std::function<void(int)> progressCallback, std::string *ExtractedSourceFileHash)
{
	BZFILE* bzf = NULL;
	int32_t   bzerr, bzerr_dummy, ret, nread, streamNo, i;
	unsigned char   obuf[5000];
	unsigned char   unused[BZ_MAX_UNUSED];
	int32_t   nUnused;
	void*   unusedTmpV;
	unsigned char *  unusedTmp;

	int verbosity = 0;
	int smallMode = 0;

	nUnused = 0;
	streamNo = 0;

	const wchar_t * readmode = L"r+bS";

	const std::wstring infilepath = file_name.toStdWString();


	FILE * zStream = _wfsopen(
		infilepath.c_str(),
		readmode,
		_SH_DENYWR
	);
	if (zStream == NULL)
	{
		throw new WorkerException("IOERROR1er");
	}

	if (ferror(zStream)) goto errhandler_io;

	while (true) {

		bzf = BZ2_bzReadOpen(
			&bzerr, zStream, verbosity,
			(int)smallMode, unused, nUnused
		);
		if (bzf == NULL || bzerr != BZ_OK)
		{
			goto errhandler;
		}
		streamNo++;

		while (bzerr == BZ_OK) {
			nread = BZ2_bzRead(&bzerr, bzf, obuf, 5000);
			if (bzerr == BZ_DATA_ERROR_MAGIC)
			{
				goto errhandler;
			}
		}
		if (bzerr != BZ_STREAM_END)
		{
			goto errhandler;
		}

		BZ2_bzReadGetUnused(&bzerr, bzf, &unusedTmpV, &nUnused);
		if (bzerr != BZ_OK)
		{
			throw("test:bzReadGetUnused");
		}

		unusedTmp = (unsigned char*)unusedTmpV;
		for (i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

		BZ2_bzReadClose(&bzerr, bzf);
		if (bzerr != BZ_OK)
		{
			throw("test:bzReadGetUnused");
		}
		if (nUnused == 0 && myfeof(zStream)) break;

	}

	if (ferror(zStream)) goto errhandler_io;
	ret = fclose(zStream);
	if (ret == EOF) goto errhandler_io;

	return true;

errhandler:
	BZ2_bzReadClose(&bzerr_dummy, bzf);
	switch (bzerr) {
	case BZ_CONFIG_ERROR:
		 break;
	case BZ_IO_ERROR:
	errhandler_io:
		break;
	case BZ_DATA_ERROR:
		fprintf(stderr,
			"data integrity (CRC) error in data\n");
		return false;
	case BZ_MEM_ERROR:
		//outOfMemory();
		throw "out of memory";
	case BZ_UNEXPECTED_EOF:
		fprintf(stderr,
			"file ends unexpectedly\n");
		return false;
	case BZ_DATA_ERROR_MAGIC:
		if (zStream != stdin) fclose(zStream);
		if (streamNo == 1) {
			fprintf(stderr,
				"bad magic number (file not created by bzip2)\n");
			return false;
		}
		else {
			/*if (noisy)
				fprintf(stderr,
					"trailing garbage after EOF ignored\n");*/
			return true;
		}
	default:
		throw("test:unexpected error");
	}

	throw("test:end");
	return true; /*notreached*/
}

void simpleBZ2DeCompress(const QString file_name, std::function<void(int)> progressCallback, std::string *ExtractedSourceFileHash)
{
	  		/*
	SHA1 ExtractedSourceFileSha1Digest;

	std::ifstream ifile;
	//Disable buffering, so we're reading straight from disk.
	ifile.rdbuf()->pubsetbuf(0, 0);
	//ifile.rdbuf()->pubsetbuf(Buffer1, 500000);
	ifile.open(file_name.toStdWString(), std::ios::binary | std::ifstream::in);
	
	if (ifile.fail() || ifile.bad() || !ifile.is_open() || ifile.eof())
	{
		//QString oiawee = strerror(errno);
		throw new WorkerException("IOERROR");
		return;
	}


	#define BZ_MAX_UNUSED 100000
	bzReadFile* bzf = NULL;

	char obuf[300000];
	unsigned char unused[BZ_MAX_UNUSED];
	unsigned char* unusedTmp;
	void* unusedTmpV;
	
	int bzerr = 0, nUnused = 0;

	//Get file size quickly for our progress bar calculation
	QFileInfo ifileInfo(file_name);


	qint64 ifileSize64 = ifileInfo.size();
	int64_t storedread = 0;
	while (true) {

		bzf = bzReadOpen(&bzerr, &ifile, 0, 0, unused, nUnused);

		if (bzf == NULL || bzerr != BZ_OK)
		{
			DecompressCleanup(&ifile, bzf);

			//TODO Error handle
			throw new WorkerException("SOMETHING DUN FUCKED");
		}

		int loaded_len = 0;
		int poser = 0;
		
		while (bzerr == BZ_OK)
		{

			int nread = bzRead(&bzerr, bzf, obuf, 30000);
			if (bzerr == BZ_DATA_ERROR_MAGIC)
			{
				DecompressCleanup(&ifile, bzf);
				throw new WorkerException("WE DONT HANDLE GREEN MAGIC");
			}
			if ((bzerr == BZ_OK || bzerr == BZ_STREAM_END) && nread > 0)
			{
				//Successfully decompressed data
				//Add decompressed data to our SHA calculation
				ExtractedSourceFileSha1Digest.add(obuf, nread);
			}
			if (ifile.fail() && !ifile.eof())
			{
				DecompressCleanup(&ifile, bzf);
				throw new WorkerException("IOERROR2");
			}
			int progress = std::max<int>(0, std::min<int>(1000, (storedread * 1000 / ifileSize64)));
			progressCallback(progress);
		}

		if (bzerr != BZ_STREAM_END)
		{
			DecompressCleanup(&ifile, bzf);
			throw new WorkerException(QString(bzerr));
		}

		bzReadGetUnused(&bzerr, bzf, &unusedTmpV, &nUnused);
		
		if (bzerr != BZ_OK)
		{
			DecompressCleanup(&ifile, bzf);
			throw new WorkerException("decompress:bzReadGetUnused");
		}

		unusedTmp = (unsigned char*)unusedTmpV;
		for (int i = 0; i < nUnused; i++) unused[i] = unusedTmp[i];

		bzReadClose(&bzerr, bzf);
		if (bzerr != BZ_OK)
		{
			DecompressCleanup(&ifile, bzf);
			throw new WorkerException("decompress:bzReadGetUnused2");
		}

		if (nUnused == 0 && ifile.eof()) break;

		int progress = std::max<int>(0, std::min<int>(1000, (storedread * 1000 / ifileSize64)));
		progressCallback(progress);
		//emit updateProgressBar(tempnamer, progress);
	}
	ExtractedSourceFileHash->assign(ExtractedSourceFileSha1Digest.getHash());
	ifile.close();

	//noerrors = true;
	return;	
	*/
	
}
			/*
void simpledecompress(const char* file_name, const char* ouerfile)
{
	FILE*   f;
	BZFILE* b;
	int     nBuf;

	//char    buf[900000];
	int     bzerror;
	int     nWritten;

	f = fopen(file_name, "rb");
	if (!f) {
		throw "COULDNT OPEN BITCH";
	}

	std::string out_file_name2 = std::string(ouerfile) + ".iso2";

	std::string out_file_name = std::string(ouerfile) + ".iso";

	//Open output file in binary
	std::ofstream ofile;
	std::vector<char> Bufa(100000);
	ofile.rdbuf()->pubsetbuf(Bufa.data(), 100000);
	ofile.open(out_file_name, std::ofstream::out | std::ios::binary);
	if (!ofile) {
		throw "COULDNT OPEN WBITCH";
	}

	b = BZ2_bzReadOpen(&bzerror, f, 0, 0, NULL, 0);
	if (bzerror != BZ_OK) {
		BZ2_bzReadClose(&bzerror, b);
		throw "COULDNT OPEN BITCH2";
	}

	//MULTIPLE OF 4096
	
	std::vector<char> chunk_data1(2621440);
	std::vector<char> chunk_data2(327680);

	int poq = fread(chunk_data1.data(), 1, chunk_data1.size(), f);

	//Chunk size we can pass to bzip2 compressor
	int chunksize = (size_t)500 * 1024;

	//Get smallest of our chunk size or length of file.
	int poser = std::min<size_t>(chunksize, poq);

	//Use string_view shim (Replaceable with C++17 version).
	//Gets a smaller chunk of data by reference.
	//Data must be smaller for C's stack size to not die, and for our reader to still read large amounts at a time.
	//bpstd::string_view chunkdata(chunk_data.data(), poser);

	//char buf[8192];

	bzerror = BZ_OK;
	uint64_t pos = 0;
	while (bzerror == BZ_OK && poser > 0) {

		int nBuf2 = BZ2_bzRead2(&bzerror, b, chunk_data1, pos, chunk_data2.data(), chunk_data2.size());
		if (nBuf2 > 0)
		{
			if (bzerror == BZ_OK) {
				ofile.write(chunk_data2.data(), nBuf2);
				pos += nBuf2;
			}
			else {
				qDebug() << bzerror;
			}
		}
		else {
			pos = 0;
			poq = fread(chunk_data1.data(), 1, chunk_data1.size(), f);

			poser = std::min<size_t>(327680, poq);
		}
		
		// = bzRead(&bzerror, b, f, chunk_data, buf.data(), buf.size());
		//std::vector<char>(1000000).swap(buf);

		//int nBuf = BZ2_bzRead(&bzerror, b, chunk_data2.data(), chunk_data2.size());
		//BZ2_bzReadGetUnused2(&bzerror, b, (char**)buf3434r2w.data(), &thinger);



	}
	if (bzerror != BZ_STREAM_END) {
		BZ2_bzReadClose(&bzerror, b);
		qDebug() << bzerror;
		throw bzerror;
	}
	else {
		void * oieieie;
		int unused = 0;
		BZ2_bzReadGetUnused(&bzerror, b, &oieieie, &unused);
		BZ2_bzReadClose(&bzerror, b);
	}
	ofile.flush();
	ofile.close();
}			  */