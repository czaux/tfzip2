#pragma once
#include <QObject>
#include <QRunnable>
#include <atomic>
#include "BZProgressBar.h"
#include "ProgressBarLayout.h"
#include "../hash-library/sha1.h"
#include <QThread>
#include "WorkerException.h"
#include "BZIP2Compress.h"
#include "BZIP2Decompress.h"
class ParallelRunnable : public QObject, public QRunnable
{
	Q_OBJECT
signals:
	void updateProg(int);

public:
	BZProgressBar * BZPbar;
	ProgressBarLayout * PBarContain;
	QString sourcefile;
	QString destfile;
	std::atomic<bool> * cancellation_token;
	std::atomic<bool> * pause_token;
	ParallelRunnable(ProgressBarLayout * PBarContain, BZProgressBar* progressbar, QString sourcefile, QString destfile, std::atomic<bool> * cancellation_token, std::atomic<bool> * pause_token) :QRunnable() {
		this->PBarContain = PBarContain;
		this->BZPbar = progressbar;
		this->sourcefile = sourcefile;
		this->destfile = destfile;
		this->cancellation_token = cancellation_token;
		this->pause_token = pause_token;
	}
	void run() {
		if (*(cancellation_token))
		{
			return;
		}

		while (*(pause_token))
		{
			QThread::msleep(200);
		}
		//SHA1 of our source file thats calculated during compression.
		std::string SourceFileSha1Digest;
		QObject::connect(this, &ParallelRunnable::updateProg, BZPbar, &BZProgressBar::setValue);
		auto progressfunc = [=](int val) { 
			emit updateProg(val); 
		};
		BZipCompressor * bzipobject = new BZipCompressor(cancellation_token, pause_token);
		destfile = destfile + ".bz2";
		QString faa2 = "D:\\store2\\er\\maps\\dr_industrial_v2b.bsp";
		QString faa3 = "D:\\store2\\er\\maps\\dr_industrial_v2b_.bsp.bz2";
		//bzipobject->simpleBZ2Compress(sourcefile, destfile, progressfunc, &SourceFileSha1Digest);
		bzipobject->compressStream(sourcefile, destfile, progressfunc, &SourceFileSha1Digest);

		//bzipobject->simpleBZ2Compress(faa2, faa3, progressfunc, &SourceFileSha1Digest);
		if (*(cancellation_token))
		{
			QMetaObject::invokeMethod(BZPbar, "setState", Qt::BlockingQueuedConnection, Q_ARG(BZProgressBar::State, BZProgressBar::State::Validating));
			QObject::disconnect(this, &ParallelRunnable::updateProg, BZPbar, &BZProgressBar::setValue);
			QMetaObject::invokeMethod(BZPbar, "setValue", Qt::BlockingQueuedConnection, Q_ARG(int, 0));
			delete bzipobject;
			QMetaObject::invokeMethod(PBarContain, "releaseProgressPathID", Qt::BlockingQueuedConnection, Q_ARG(QString, sourcefile));
			//TODO: delete file
			return;
		}

		while (*(pause_token))
		{
			QThread::msleep(200);
		}

		QMetaObject::invokeMethod(BZPbar, "setValue", Qt::QueuedConnection, Q_ARG(int, 0));
		QMetaObject::invokeMethod(BZPbar, "setState", Qt::QueuedConnection, Q_ARG(BZProgressBar::State, BZProgressBar::State::Validating));

		//SHA1 of our decompressed bz2 file during decompression
		std::string ExtractedSourceFileSha1Digest;
		//simpleBZ2DeCompress(destfile, progressfunc, &ExtractedSourceFileSha1Digest);
		QString faa = "D:\\store2\\er\\maps\\dr_industrial_v2b.bsp.bz2";
		//simpleBZ2DeCompress(destfile, progressfunc, &ExtractedSourceFileSha1Digest);
		simpleBZ2DeCompress(faa3, progressfunc, &ExtractedSourceFileSha1Digest);
		//uncompressStream(faa);
		bool valid = validateArchive(destfile, progressfunc, &ExtractedSourceFileSha1Digest);
		if (!valid)
		{
			throw "hi";
		}
		if (*(cancellation_token))
		{
			QObject::disconnect(this, &ParallelRunnable::updateProg, BZPbar, &BZProgressBar::setValue);
			QMetaObject::invokeMethod(BZPbar, "setValue", Qt::BlockingQueuedConnection, Q_ARG(int, 0));
			delete bzipobject;
			QMetaObject::invokeMethod(PBarContain, "releaseProgressPathID", Qt::BlockingQueuedConnection, Q_ARG(QString, sourcefile));
			//TODO: delete file
			return;
		}

		while (*(pause_token))
		{
			QThread::msleep(200);
		}
		 
		if (ExtractedSourceFileSha1Digest.size() != 40)
		{
			throw new WorkerException("NOT RIGHT SIZE");
		}

		//Compare SHA1 digests.
		if (SourceFileSha1Digest.compare(ExtractedSourceFileSha1Digest) != 0)
		{
			throw new WorkerException("SHAS DONT MATCH");
		} 

		QObject::disconnect(this, &ParallelRunnable::updateProg, BZPbar, &BZProgressBar::setValue);
		QMetaObject::invokeMethod(BZPbar, "setValue", Qt::BlockingQueuedConnection, Q_ARG(int, 0));
		//bzipobject->asyncBZ2Compress(sourcefile, destfile, progressfunc);
		
		/*if (!bzipobject->noerrors)
		{
			QFile file(destfile);
			file.remove();
		}*/
		
		delete bzipobject;
		 
		QMetaObject::invokeMethod(PBarContain, "releaseProgressPathID", Qt::BlockingQueuedConnection, Q_ARG(QString, sourcefile));
		
		return;

	}
};
