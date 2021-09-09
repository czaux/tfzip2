#include <fstream>
#include <iostream>
#include <chrono>
#include <future>
#include <QDebug>
#include "bzip2-1.0.6\bzlib.h"
#include <algorithm>
#include "BZIP2Compress.h"
#include "tfzip2.h"
#include <QThread>
#include <QTime>
#include <QDir>


void BZipCompressor::doathing43(QList<QString> files)
{
	QThread::currentThread()->setPriority(QThread::LowPriority);

	QString sourcefile = files.at(0);
	QString destfile = files.at(1);
	//TODO WIDECHARS
	//asyncBZ2Compress(sourcefile.toStdString().c_str(), destfile.toStdString().c_str());
	//asyncBZ2DeCompress(sourcefile.toStdString().c_str(), destfile.toStdString().c_str());
	QTime myTimer2;
	myTimer2.start();

	this->asyncBZ2Compress(sourcefile.toStdWString(), destfile.toStdWString());
	//simpledecompress(sourcefile.toStdString().c_str(), destfile.toStdString().c_str());

	int nMilliseconds = myTimer2.elapsed();
	qDebug() << nMilliseconds << " | " << sourcefile.toStdString().c_str();
	//QFileInfo ay = filepath;
	//qDebug() << "hi";
	//QThread thisthread = QThread::currentThread();
	//qDebug() << name;
}


	//Inline function for our async file reader. Simply reads new chunk of the file into secondary buffers by pointer.
	int asyncFileRead(const std::atomic_bool& cancelled, std::vector<char> &newchunk, std::ifstream &filer, size_t &length, int bigbufsize) {
		int64_t fullcount = 0;
		int smallerbufsize = 100000;
		for (int64_t i = 0; i < bigbufsize;)
		{
			if (cancelled) return 2;
			int64_t size = std::min<int64_t>(bigbufsize, i);
			length = filer.read(&newchunk[0] + i, size).gcount();
			if (length > 0)
			{
				i += length;
			}
			else {
				break;
			}

		}
		length = filer.read(&newchunk[0], bigbufsize).gcount();
		
		if (length > 0)
		{
			return 1;
		}
		else {
			return 2;
		}
	};

	 int asyncFileWrite(const std::atomic_bool& cancelled, std::vector<char> &newchunk, std::ofstream &filer, int bufsize) {
		 int fullcount = 0;
		 int smallerbufsize = 100000;
		 for (int64_t i = 0; i < bufsize; i += smallerbufsize)
		 {
			 //FIXME
			 if (cancelled) return 2;
			 int oie = std::min<int>(i, bufsize);
			 filer.write(newchunk.data() + oie, smallerbufsize);
		 }

		filer.flush();

		//TODO error handle
		return 1;
	};
   /*
	void BZipCompressor::SuperBZWriteClose(
		int*          bzerror,
		bzFile*       b,
		std::vector<unsigned char> &storestuff,
		int           abandon,
		unsigned int* nbytes_in_lo32,
		unsigned int* nbytes_in_hi32,
		unsigned int* nbytes_out_lo32,
		unsigned int* nbytes_out_hi32)
	{
		int   n, n2, ret;
		bzFile* bzf = (bzFile*)b;

		if (bzf == NULL)
		{
			bzf->lastErr = BZ_OK;
			return;
		};
		if (!(bzf->writing))
		{
			bzf->lastErr = BZ_SEQUENCE_ERROR;
			throw "BZ_SEQUENCE_ERROR";
		};

		if (nbytes_in_lo32 != NULL) *nbytes_in_lo32 = 0;
		if (nbytes_in_hi32 != NULL) *nbytes_in_hi32 = 0;
		if (nbytes_out_lo32 != NULL) *nbytes_out_lo32 = 0;
		if (nbytes_out_hi32 != NULL) *nbytes_out_hi32 = 0;

		if ((!abandon) && bzf->lastErr == BZ_OK) {

			while (true) {
				bzf->strm.avail_out = BZ_MAX_UNUSED;
				bzf->strm.next_out = bzf->buf;
				ret = BZ2_bzCompress(&(bzf->strm), BZ_FINISH);
				if (ret != BZ_FINISH_OK && ret != BZ_STREAM_END)
				{
					bzf->lastErr = ret;
					throw ret;
				};

				if (bzf->strm.avail_out < BZ_MAX_UNUSED) {
					n = BZ_MAX_UNUSED - bzf->strm.avail_out;
					storestuff.insert(storestuff.end(), (const char*)(bzf->buf), (const char*)(bzf->buf) + n);
				}

				if (ret == BZ_STREAM_END) {
					break;
				}
			}
		}

		if (nbytes_in_lo32 != NULL)
			*nbytes_in_lo32 = bzf->strm.total_in_lo32;
		if (nbytes_in_hi32 != NULL)
			*nbytes_in_hi32 = bzf->strm.total_in_hi32;
		if (nbytes_out_lo32 != NULL)
			*nbytes_out_lo32 = bzf->strm.total_out_lo32;
		if (nbytes_out_hi32 != NULL)
			*nbytes_out_hi32 = bzf->strm.total_out_hi32;
		//OK
		BZ2_bzCompressEnd(&(bzf->strm));
		free(bzf);
	}  */
	/*
	void BZipCompressor::SuperBzipWrite(bzFile* out_bz2,
		const std::atomic_bool* cancelled,
		std::vector<unsigned char> &storestuff,
		int len,
		unsigned char* buf)
	{
		int n, n2, ret;
		bzFile* bzf = out_bz2;
		//OK
		if (bzf == NULL || buf == NULL || len < 0)
		{
			bzf->lastErr = BZ_PARAM_ERROR;
			//BZ_PARAM_ERROR
			throw "BZ_PARAM_ERROR";
		};
		if (!(bzf->writing))
		{
			bzf->lastErr = BZ_SEQUENCE_ERROR;
			throw "BZ_SEQUENCE_ERROR";
		};
		if (len == 0)
		{
			//OK
			bzf->lastErr = BZ_OK;
			return;
		};

		bzf->strm.avail_in = len;
		bzf->strm.next_in = buf;
		while (true) {
			bzf->strm.avail_out = BZ_MAX_UNUSED;
			bzf->strm.next_out = bzf->buf;
			ret = BZ2_bzCompress(&(bzf->strm), BZ_RUN);
			if (ret != BZ_RUN_OK)
			{
				bzf->lastErr = ret;
				throw ret;
			};

			if (bzf->strm.avail_out < BZ_MAX_UNUSED) {
				n = BZ_MAX_UNUSED - bzf->strm.avail_out;
				qApp->processEvents();
				if (*(cancelled))
				{
					return;
				}
				//Insert compressed data into our char vector.
				storestuff.insert(storestuff.end(), (const unsigned char*)(bzf->buf), (const unsigned char*)(bzf->buf) + n);
			}

			if (bzf->strm.avail_in == 0)
			{
				bzf->lastErr = BZ_OK;
				//OK
				break;
			};
		}
	}  */


void BZipCompressor::compressStream(QString &inpath, QString &outpath, std::function<void(int)> progressCallback, std::string *SourceFileHash)
{
	BZFILE* bzf = NULL;
	bool overwrite = true;
	
	SHA1 SourceFileSHA1Digest;

	unsigned char   ibuf[20000];
	int   nIbuf;
	unsigned int  nbytes_in_lo32, nbytes_in_hi32;
	unsigned int  nbytes_out_lo32, nbytes_out_hi32;
	int   bzerr, bzerr_dummy, ret;

	int workFactor = 30;
	int blockSize100k = 9;
    QString ppath = QDir::toNativeSeparators(inpath);
    const std::wstring infilepath = ppath.toStdWString();
	FILE * stream = MyOpenFileRWSeq(infilepath.c_str(), false);
	if (stream == NULL)
	{
		throw new WorkerException("IOERROR1er");
	}
	
     QString ppath2 = QDir::toNativeSeparators(outpath);
    const std::wstring outfilepath = ppath2.toStdWString();
    FILE * zStream = MyOpenFileRWSeq(outfilepath.c_str(), overwrite);
	if (zStream == NULL)
	{
		throw new WorkerException("IOERROR1er");
	}
	if (ferror(stream)) goto errhandler_io;
	if (ferror(zStream)) goto errhandler_io;

	bzf = BZ2_bzWriteOpen(&bzerr, zStream,
		blockSize100k, 0, workFactor);

	if (bzerr != BZ_OK) goto errhandler;
	size_t filesize = static_cast<size_t>(MyGetFileSize(stream));
	size_t readbytes = 0;

	while (true) {

		if (myfeof(stream)) break;
		nIbuf = fread(ibuf, sizeof(unsigned char), 20000, stream);
		SourceFileSHA1Digest.add(ibuf, nIbuf);
		if (ferror(stream)) goto errhandler_io;
		if (nIbuf > 0) BZ2_bzWrite(&bzerr, bzf, (void*)ibuf, nIbuf);
		if (bzerr != BZ_OK) goto errhandler;
		if (nIbuf > 0)
		{
			readbytes += nIbuf;
			int progress = std::max<int>(0, std::min<int>(1000, (readbytes * 1000 / filesize)));
			progressCallback(progress);
		}

	}

	BZ2_bzWriteClose64(&bzerr, bzf, 0,
		&nbytes_in_lo32, &nbytes_in_hi32,
		&nbytes_out_lo32, &nbytes_out_hi32);
	if (bzerr != BZ_OK) goto errhandler;

	if (ferror(zStream)) goto errhandler_io;
	ret = fflush(zStream);
	if (ret == EOF) goto errhandler_io;
	ret = fclose(zStream);
	if (ret == EOF) goto errhandler_io;
	if (ferror(stream)) goto errhandler_io;
	ret = fclose(stream);
	if (ret == EOF) goto errhandler_io;
	SourceFileHash->assign(SourceFileSHA1Digest.getHash());
	return;

errhandler:
	BZ2_bzWriteClose64(&bzerr_dummy, bzf, 1,
		&nbytes_in_lo32, &nbytes_in_hi32,
		&nbytes_out_lo32, &nbytes_out_hi32);
	switch (bzerr) {
	case BZ_CONFIG_ERROR:
			break;
	case BZ_MEM_ERROR:
			break;
	case BZ_IO_ERROR:
	errhandler_io:
			break;
	default:
		throw("compress:unexpected error");
	}

	throw("compress:end");
	/*notreached*/
}

	void BZipCompressor::simpleBZ2Compress(const QString file_name, const QString out_name, std::function<void(int)> progressCallback, std::string *SourceFileHash)
	{			   /*
		SHA1 SourceFileSHA1Digest;

		std::fstream ifile;
		char Buffer1[500000];
		ifile.rdbuf()->pubsetbuf(Buffer1, 500000);
		ifile.open(file_name.toStdWString() , std::ios::binary | std::ifstream::in);

		if (ifile.fail())
		{
			throw new WorkerException("IOERROR");
		}

		//Create directory
		QFileInfo outinfo(out_name);
		outinfo.path();
		QDir dir1(outinfo.path());
		//qDebug() << outinfo.path();
		if (!dir1.exists())
		{
			QDir dir = QDir::root();
			dir.mkpath(dir1.path());
		}

		//Open output file in binary
		std::fstream ofile;

		//Use a giant buffer. Unsure if this performs any better.
		std::vector<char> Bufa(10000000);
		ofile.rdbuf()->pubsetbuf(Bufa.data(), 10000000);
		ofile.open(out_name.toStdWString(), std::ofstream::out | std::ios::binary);

		if (ofile.fail())
		{
			throw new WorkerException("IOERROR2");
		}

		#define BZ_MAX_UNUSED 900000

		//Open our Bzip2 file. Notice how we are NOT handling our actual file through bzip2lib
		int bz2error;
		bzWriteFile* out_bz2 = bzWriteOpen(&bz2error, 9, 0, 0);
		if (BZ_OK != bz2error)
		{
			ifile.close();
			ofile.close();
			//TODO Error handle
			throw new WorkerException(QString(bz2error));
		}

		//Get file size quickly for our progress bar calculation
		QFileInfo eoqwerio(file_name);
		qint64 wepo = eoqwerio.size();

		std::vector<char> storestuff;
		std::vector<char> chunk_data(100000);

		int loaded_len = 0;
		int poser = 0;
		uint64_t overallprocessed = 0;
		uint64_t storedpos = 0;
		while (true)
		{
			//Some magic to turn the pointer back to proper value. Is this proper?
			if (*(cancellation_token))
			{
				//Clean up, abandon data.
				bzWriteClose64(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
				//SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
				ifile.close();
				ofile.flush();
				ofile.close();
				return;
			}

			while (*(pause_token))
			{
				QThread::msleep(200);
			}

			loaded_len = ifile.read(&chunk_data[0], chunk_data.size()).gcount();

			//Add our read data chunk to our SHA1 calc.
			SourceFileSHA1Digest.add(&chunk_data[0], loaded_len);

			if (loaded_len > 0)
			{
				//Actual BZip2 function that compresses our data buffer.
				//This is modified from the original version and writes data to our char vector instead
				//SuperBzipWrite(out_bz2, cancellation_token, storestuff, loaded_len, chunk_data.data());
				bzWrite(&bz2error, out_bz2, storestuff, chunk_data.data(), loaded_len);
				//Some magic to turn the pointer back to proper value. Is this proper?
				if (*(cancellation_token))
				{
					//Clean up, abandon data.
					bzWriteClose64(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
					ifile.close();
					ofile.flush();
					ofile.close();
					return;
				}

				while (*(pause_token))
				{
					QThread::msleep(1000);
				}

				//If something goes wrong!
				if (BZ_OK != bz2error)
				{
					bzWriteClose64(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
					ifile.close();
					ofile.flush();
					ofile.close();
					throw new WorkerException(QString(bz2error));
					//throw out_bz2->lastErr;
					//TODO error handle
					//return;
				}

				//storestuff is sized to however much data was added
				ofile.write(storestuff.data(), storestuff.size());

				overallprocessed += loaded_len;
				int progress = std::max<int>(0, std::min<int>(1000, (overallprocessed * 1000 / wepo)));
				progressCallback(progress);
				//emit updateProgressBar(tempnamer, progress);

				//Reset vector, or else it will continue to store info ever y pass. #memoryleak
				std::vector<char>().swap(storestuff);

			}

			//Break if we reached EOF.
			if (ifile.eof()) break;
		}

		unsigned int total_in, total_cmp;   // how many bytes we read, and resulting compressed size

		//Store finished stream here
		std::vector<char> storestuff2;

		//Finish stream and cleanup
		bzWriteClose64(&bz2error, out_bz2, storestuff2, 1, &total_in, NULL, &total_cmp, NULL);
		//SuperBZWriteClose(&bz2error, out_bz2, storestuff2, 0, &total_in, NULL, &total_cmp, NULL);

		if (bz2error != BZ_OK)
		{
			ofile.close();
			ifile.close();
			throw new WorkerException(QString(bz2error));
		}
		//Write the finished stream to file.
		ofile.write(storestuff2.data(), storestuff2.size());

		//Cleanup. Maybe not needed
		std::vector<char>().swap(storestuff2);
		std::vector<char>().swap(storestuff);
		std::vector<char>().swap(chunk_data);

		ofile.flush();
		ofile.close();
		ifile.close();

		noerrors = true;
		SourceFileHash->assign(SourceFileSHA1Digest.getHash());
		//Remove progress bar.
		//emit unsignProgressBar(QString::fromStdWString(file_name));
		//qApp->processEvents();
						   */
	}


						 
	void BZipCompressor::asyncBZ2Compress(std::wstring file_name, std::wstring ouerfile)
	{
		/*
		//emit signProgressBar(QString(file_name));

		//Open input file in binary
		//ifstream should map file to memory like mmap
		std::ifstream ifile;
		char Buffer1[500000];
		ifile.rdbuf()->pubsetbuf(Buffer1, 500000);
		ifile.open(file_name, std::ios::binary);

		std::wstring out_file_name = std::wstring(ouerfile) + L".bz2";

		//Open output file in binary
		std::ofstream ofile;
		std::vector<char> Bufa(20000000);
		ofile.rdbuf()->pubsetbuf(Bufa.data(), 20000000);
		ofile.open(out_file_name, std::ofstream::out | std::ios::binary);

		//Open our Bzip2 file. Notice how we are NOT handling our actual file through bzip2lib
		int bz2error;
		bzFile* out_bz2 = BZ2_bzWriteOpen(&bz2error, 9, 0, 0);
		if (BZ_OK != bz2error)
		{
			ifile.close();
			ofile.close();
			//TODO Error handle
			return;
		}

		//10MB 'BIG' buffer size for our async read.
		const int bigbufsize = 25000000;
		int readsize = 500000;

		//Initial store our big read into a char vector.
		std::vector<char> chunk_data(bigbufsize);
		size_t loaded_len;

		//Secondary 'BIG' buffer to store data in with our async request while compression iteration is still going.
		std::vector<char> newchunkcopy(bigbufsize);
		size_t newlengthcopy = 0;

		int writebufsize;

		bool firstrun = true;

		//Keep going until our async read has no more data to return
		int ier = 1;
		int writestatus = 0;

		std::future<int> writereturn;
		std::vector<char> storestuff;
		std::vector<char> storestuff2;

		QFileInfo eoqwerio(QString::fromStdWString(file_name));
		qint64 wepo = eoqwerio.size();
		  */
	
		/*
		don't keep
		auto const start_pos = ifile.tellg();
		if (std::streamsize(-1) == start_pos)
			throw std::ios_base::failure{ "error" };

		//this takes forever, but maybe we should keep it just in case
		//if (!ifile.ignore((std::numeric_limits<std::streamsize>::max)()))
		//	throw std::ios_base::failure{ "error" };

		uint64_t char_count = ifile.gcount();

		if (!ifile.seekg(start_pos))
			throw std::ios_base::failure{ "error" };
*/
	/*
		while (*(pause_token))
		{
			QThread::msleep(200);
		}

		bool ranwrite = false;
		//emit requestNewLabel2("hi");

		uint64_t overallprocessed = 0;
		while (ier == 1) {
			while (*(pause_token))
			{
				QThread::msleep(200);
			}
			//Check if first run. Only use async if not.
			if (newlengthcopy < 1) {
				//Should be first run only
				loaded_len = ifile.read(&chunk_data[0], readsize).gcount();
			}
			else {
				//Update our buffers from last loops async big chunk read.
				chunk_data.swap(newchunkcopy);
				loaded_len = newlengthcopy;
			}

			//Send our async request to get more data from input file.
			std::future<int> ret;
			ret = std::async(std::launch::async, asyncFileRead, &cancellation_token, std::ref(newchunkcopy), std::ref(ifile), std::ref(newlengthcopy), readsize);
			
			//Chunk size we can pass to bzip2 compressor
			size_t chunksize = (size_t)500 * 1024;

			//Get smallest of our chunk size or length of file.
			size_t poser = std::min<size_t>(chunksize, loaded_len);

			//Use string_view shim (Replaceable with C++17 version).
			//Gets a smaller chunk of data by reference.
			//Data must be smaller for C's stack size to not die, and for our reader to still read large amounts at a time.
			bpstd::string_view chunkdata(chunk_data.data(), poser);

			//accumulated position from source file.
			size_t poseint = 0;

			//accumulated length of returned bzip2 compressed data
			int64_t bigbziplen = 0;

			if (*(cancellation_token)) {
				if (ret.valid()) ret.wait();
				if (writereturn.valid()) writereturn.wait();
				SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
				ifile.close();
				ofile.flush();
				ofile.close();
				return;
			}

			//Loop until our length left is 0.
			while (poser > 0)
			{
				while (*(pause_token))
				{
					QThread::msleep(200);
				}
				if (*(cancellation_token)) {
					if (ret.valid()) ret.wait();
					if (writereturn.valid()) writereturn.wait();
					SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
					ifile.close();
					ofile.flush();
					ofile.close();
					return;
				}

				//Actual BZip2 function that compresses our data buffer.
				//This is modified from the original version and writes data to our char vector instead
				SuperBzipWrite(out_bz2, cancellation_token, storestuff, poser, (char*)chunkdata.c_str());

				if (*(cancellation_token)) {
					if (ret.valid()) ret.wait();
					if (writereturn.valid()) writereturn.wait();
					SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
					ifile.close();
					ofile.flush();
					ofile.close();
					return;
				}

				//If something goes wrong!
				if (BZ_OK != out_bz2->lastErr)
				{
					if (ret.valid()) ret.wait();
					if (writereturn.valid()) writereturn.wait();
					SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
					ifile.close();
					ofile.flush();
					ofile.close();
					throw out_bz2->lastErr;
					//TODO error handle
					return;
				}

				//Increment by chunk size for next pass
				poseint += chunksize;
				overallprocessed += chunksize;
				//Handle when the rest of our file is smaller than the chunk size
				std::int64_t restfilelength = (loaded_len - poseint);

				//Set our position, the smallest out of either our chunksize, or the rest of the length of our file.
				poser = std::max<std::int64_t>((std::int64_t)0, std::min<std::int64_t>((std::int64_t)chunksize, restfilelength));

				//Reset string_view to our new data.
				chunkdata = bpstd::string_view(chunk_data.data() + poseint, poser);

				int progress = std::max<int>(0, std::min<int>(1000, (overallprocessed * 1000 / wepo)));
				//qDebug() << progress;
				//emit updateProgressBar(QString(file_name), progress);

				if (readsize < bigbufsize) readsize = std::min<int>(readsize *= 1.15, bigbufsize);
			}

			if (*(cancellation_token)) {
				if (ret.valid()) ret.wait();
				if (writereturn.valid()) writereturn.wait();
				SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
				ifile.close();
				ofile.flush();
				ofile.close();
				return;
			}

			if (storestuff.size() >= 2000000 && readsize >= bigbufsize)
			{
				if (ranwrite) {
					//if (!writereturn._Is_ready()) {
					//	qDebug() << "Write Delayed!";
					//}
					writereturn.get();
					ranwrite = false;
				}
				//Free buffers and swap pointers from this loops data with our second buffer
				std::vector<char>().swap(storestuff2);
				storestuff2.swap(storestuff);
				std::vector<char>().swap(storestuff);

				writereturn = std::async(std::launch::async, asyncFileWrite, &cancellation_token, std::ref(storestuff2), std::ref(ofile), storestuff2.size());
				ranwrite = true;
			}

			//Get our data asynchronously. This will wait if it isn't ready yet. If its waiting, you're IO capped!
			//If there's no more data, falsify the loop condition.
			//if (!ret._Is_ready()) qDebug() << "Read Isn't ready!";
			ret.wait();
			ier = ret.get();
		}

		//time_point<Clock> start2 = Clock::now();
		//time_point<Clock> end2 = Clock::now();
		//milliseconds diff2 = duration_cast<milliseconds>(end2 - start2);
		//qDebug() << "read: " << diff2.count() << "ms";

		if (*(cancellation_token)) {
			if (writereturn.valid()) writereturn.wait();
			SuperBZWriteClose(&bz2error, out_bz2, storestuff, 1, 0, NULL, 0, NULL);
			ifile.close();
			ofile.flush();
			ofile.close();
			return;
		}

		//wait for last looped write if we have one
		if (ranwrite) {
			//if (!writereturn._Is_ready()) {
			//	qDebug() << "Write Delayed!";
			//}
			writereturn.get();
			ranwrite = false;
		}

		//Get rest of buffer data and write it
		if (storestuff.size() > 0)
		{
			ofile.write(storestuff.data(), storestuff.size());
		}

		// close in file
		ifile.close();

		// close out bzip2 stream and the file
		unsigned int total_in, total_cmp;   // how many bytes we read, and resulting compressed size

		//clear buffer for our last potential write
		std::vector<char>().swap(storestuff2);

		SuperBZWriteClose(&bz2error, out_bz2, storestuff2, 0, &total_in, NULL, &total_cmp, NULL);
		ofile.write(storestuff2.data(), storestuff2.size());
		ofile.flush();
		ofile.close();

		noerrors = true;

		//emit unsignProgressBar(QString(file_name));

		return;
		*/
	}
