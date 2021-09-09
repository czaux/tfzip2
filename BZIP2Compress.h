#pragma once

#include "TFZip.h"
#include <QObject>
#include <atomic>
#include <functional>
#include "../hash-library/sha1.h"
#include "WorkerException.h"
#include "IOUtils.h"
#include "tfbzlib.h"
class BZipCompressor : public QObject
{
	Q_OBJECT

public:
	explicit BZipCompressor(std::atomic<bool> * cancellation_token, std::atomic<bool> * pause_token, QObject *parent = 0) : QObject(parent) {
		this->cancellation_token = cancellation_token;
		this->pause_token = pause_token;
	}

	//void newLabel(const QString &image) { emit requestNewLabel2(image); }

	void asyncBZ2Compress(std::wstring file_name, std::wstring ouerfile);

	void doathing43(QList<QString> files);

	void simpleBZ2Compress(const QString file_name, const QString out_name, std::function<void(int)> progressCallback, std::string *SourceFileHash);
	
	void compressStream(QString &inpath, QString &outpath, std::function<void(int)> progressCallback, std::string *SourceFileHash);

	std::atomic<bool> * cancellation_token;

	std::atomic<bool> * pause_token;

	bool noerrors = false;

	/*BZipCompressor(TFZip* tfzip)
	{
		this->tfzip = tfzip;
	}*/

	//int asyncFileRead(std::vector<char> &newchunk, std::ifstream &filer, size_t &length, int bigbufsize);
	//int asyncFileWrite(std::vector<char> &newchunk, std::ofstream &filer, int64_t bufsize);

private slots:

private:

	  /*
	void SuperBZWriteClose(
		int*          bzerror,
		bzFile*       b,
		std::vector<unsigned char> &storestuff,
		int           abandon,
		unsigned int* nbytes_in_lo32,
		unsigned int* nbytes_in_hi32,
		unsigned int* nbytes_out_lo32,
		unsigned int* nbytes_out_hi32);



	void SuperBzipWrite(bzFile* out_bz2,
		const std::atomic_bool* cancelled,
		std::vector<unsigned char> &storestuff,
		int len,
		unsigned char* buf);   */

	TFZip* tfzip;

signals:
	void updateProgressBar(QString path, int progress);
};

