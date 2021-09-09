#include "tfzip2.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QDebug>
#include <QStandardItem>
#include <QFileIconProvider>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QList>
#include <QFileInfo>
#include <QThread>
#include <fstream>
#include <QPair>
#include <iostream>
#include <utility>
#include <chrono>
#include <future>
#include <memory>
#include <algorithm>
#include <QDataStream>
#include <algorithm>
#include <QMessageBox>
#include "BZIP2Compress.h"
#include "BZIP2Decompress.h"
#include "BZProgressBar.h"
#include "ProgressBarLayout.h"
#include <cwchar>
#include "ParallelRunnable.hpp"

TFZip::TFZip(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::TFZipClass)
{
    ui->setupUi(this);

//TODO: Check if remove
#ifdef _WIN32
    //http://doc.qt.io/qt-5/qfileinfo.html#ntfs-permissions
    qt_ntfs_permission_lookup++;
#endif

    //By default, this gets all logical cores
    m_logicals = QThread::idealThreadCount();
    //m_logicals = 1;

    //m_superfuturewatcher = new SuperFutureWatcher();

    //Initialize our progress bar container
    PBarLayout = new ProgressBarLayout();
    //ProgressBarLayout * PBarLayout2 = new ProgressBarLayout();
    //Create a new progress bar for every thread
    qRegisterMetaType<BZProgressBar::State>("BZProgressBar::State");
    for (int i = 0; i < m_logicals; i++)
    {
        //Initialize our custom progress bar to idle
        BZProgressBar * BZPbar = new BZProgressBar();
        BZPbar->setValue(0);
        BZPbar->setRange(0, 1000);
        BZPbar->setState(BZProgressBar::State::Idle);
        PBarLayout->addWidget(BZPbar);
    }

    //Add the container to our UI
    ui->progressContainer->addLayout(PBarLayout);

    pause_token = false;
}

TFZip::~TFZip()
{
    delete ui;
}

void TFZip::closeEvent(QCloseEvent *event)
{
    m_future.cancel();
    m_futureWatcher.cancel();
    qApp->processEvents();

    m_future.setPaused(false);
    m_futureWatcher.setPaused(false);
    //pause_token is atomic.
    pause_token = false;

    //cancellation token is atomic.
    cancellation_token = true;
    m_future.waitForFinished();
    qApp->processEvents();
}

void TFZip::on_sourceBrowse_clicked()
{
    QString UserDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        UserDir,
        QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) ui->sourceInput->setText(dir);

}

void TFZip::on_destBrowse_clicked()
{
    QString UserDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        UserDir,
        QFileDialog::ShowDirsOnly);

    if (!dir.isEmpty()) {
        QDir destdir(dir);

        if (destdir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden).count() > 0)
        {
            QMessageBox messageBox;
            messageBox.setText("Error");
            messageBox.setInformativeText(dir + "\n\nDestination directory should be empty!");
            messageBox.setStandardButtons(QMessageBox::Ok);
            //QAbstractButton *myYesButton = messageBox.addButton(trUtf8("Continue"), QMessageBox::YesRole);
            //QAbstractButton *myNoButton = messageBox.addButton(trUtf8("Go Back"), QMessageBox::NoRole);
            messageBox.setIcon(QMessageBox::Critical);
            int result = messageBox.exec();
            ui->destInput->setText("");

        }
        else {
            ui->destInput->setText(dir);
        }
    }
}

void TFZip::processUserInput()
{
    QString sourceInputString = ui->sourceInput->text();
    QString destInputString = ui->destInput->text();
    ui->sourcePathDisplay->setText(sourceInputString);
    ui->destPathDisplay->setText(destInputString);
}

void TFZip::on_sourceInput_textChanged(const QString &arg1)
{
    processUserInput();
}

void TFZip::on_destInput_textChanged(const QString &arg1)
{
    processUserInput();
}

const int pIncrement = 1;


void TFZip::doathing43(std::pair<std::wstring, std::wstring> files)
{
    QString sourcefile2 = QString::fromStdWString(files.first);
    QThread::currentThread()->setPriority(QThread::LowPriority);

    QString sourcefile = QString::fromStdWString(files.first);
    QString destfile = QString::fromStdWString(files.second);
    QMetaObject::invokeMethod(PBarLayout, "reserveProgressPathID", Q_ARG(QString, sourcefile));

    //QTime myTimer2;
    //myTimer2.start();

    BZipCompressor * aybb = new BZipCompressor(&cancellation_token, &pause_token);

    connect(aybb, SIGNAL(updateProgressBar(QString, int)), PBarLayout, SLOT(setProgressValue(QString, int)));

    //aybb->simpleBZ2Compress(sourcefile.toStdWString(), destfile.toStdWString());

    //Try to delete file if we got errors.
    if (!aybb->noerrors)
    {
        QFile file(destfile);
        file.remove();
    }

    QMetaObject::invokeMethod(PBarLayout, "releaseProgressPathID", Q_ARG(QString, sourcefile));
    delete aybb;
    return;
    //int nMilliseconds = myTimer2.elapsed();
    //qDebug() << nMilliseconds << " | " << sourcefile.toStdString().c_str();
}

void TFZip::allfilesProgressValueSlot(int value) {
    //int current = ui->filesProgressBar->value();
    ui->filesProgressBar->setValue(value += 1);
    //qApp->processEvents();
}

void TFZip::on_pauseButton_clicked()
{
    pause_token = true;
    ui->startButton->setEnabled(true);
}

void TFZip::on_stopButton_clicked()
{
    //disconnect(&m_futureWatcher, 0, ui->filesProgressBar, 0);
    //disconnect(&m_futureWatcher, 0, ui->filesProgressBar, 0);
    //disconnect(&m_futureWatcher, 0, this, 0);
    ui->pauseButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    cancellation_token = true;
    pause_token = false;

    //Wait until threads are gone before re-enabling start.
    while (QThreadPool::globalInstance()->activeThreadCount() > 0)
    {
        QThread::msleep(100);
        qApp->processEvents();
    }
    ui->startButton->setEnabled(true);
    /*
    if(m_FileSearchRunning)
    {
        cancellation_token = true;
        pause_token = false;
    }
    else {
        m_future.setPaused(false);
        m_futureWatcher.setPaused(false);
        m_futureWatcher.cancel();
        qApp->processEvents();
        //cancellation token is atomic.
        cancellation_token = true;
        //pause token is atomic.
        pause_token = false;
        m_futureWatcher.waitForFinished();
        qApp->processEvents();
        cancellation_token = false;
    }

    ui->filesProgressBar->reset();
    ui->filesProgressBar->setFormat("Stopped");
    ui->filesProgressBar->setValue(0);
    PBarLayout->changeAllPBarStates(BZProgressBar::State::Idle);
    */
}

QList<QFileInfo> TFZip::Dothings(const QString path)
{
    QList<QFileInfo> filelist;

    uint64_t i = 0;
    QDirIterator it(path, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isFile())
        {
            if (i % 10 == 0) ui->filesProgressBar->setValue(i);
            QFileInfo thing = it.fileInfo();
            filelist.append(thing);
        }
    }

    return filelist;
}

void TFZip::fileScanProgress(int filenum) {
    m_filescancount += filenum;
    ui->filesProgressBar->setFormat(QString("Getting Files: " + QString::number(m_filescancount)));
}
/*
void TFZip::DoParallel() {
    if (cancellation_token)
    {
        return;
    }
    while (pause_token)
    {
        QThread::msleep(200);
    }

    //Take ownership of pointer

    FileErrorDialog *about = new FileErrorDialog(this);
    */
    /*
    QStandardItemModel *model;
    model = new QStandardItemModel(about);
    QFileIconProvider iconprovider;
    QIcon icon = iconprovider.icon(QFileIconProvider::File);
    model->appendRow(new QStandardItem(icon, "Test"));
    about->listView->setModel(model);
    about->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    about->exec();
    int result = about->result();
    qDebug() << result;
    QThread::msleep(10000);
    */

    /*
    bool stopped = false;

    QThreadPool::globalInstance()->setMaxThreadCount(m_logicals);

    PBarLayout->changeAllPBarStates(BZProgressBar::State::Compressing);

    //m_files must be in the class, or else when this function ends the variable will be freed and bad things happen.
    //m_future = QtConcurrent::map(files, [this](QList<QString> files2) {doathing43(files2); });
    connect(&m_futureWatcher, SIGNAL(started()), ui->filesProgressBar, SLOT(reset()));

    connect(&m_futureWatcher, &QFutureWatcher<void>::finished, this, [=]() {});

    connect(&m_futureWatcher, SIGNAL(progressValueChanged(int)), this, SLOT(allfilesProgressValueSlot(int)));

    //connect(this, &TFZip::reservePathID, PBars, &ProgressBars::reserveProgressPathID, Qt::BlockingQueuedConnection);
    //connect(this, &TFZip::releasePathID, PBars, &ProgressBars::releaseProgressPathID, Qt::BlockingQueuedConnection);
    //When future watcher is finished, clear file list;
    connect(&m_futureWatcher, &QFutureWatcher<void>::finished, this, [=]() { m_Files.reset(); });
    m_futureWatcher.setFuture(m_future);
    m_future = QtConcurrent::map(m_Files->begin(), m_Files->end(), [this](std::pair<std::wstring, std::wstring> files2) {doathing43(files2); });
    m_futureWatcher.setFuture(m_future);
    //connect(&m_futureWatcher, SIGNAL(progressRangeChanged(int min, int max)), ui->filesProgressBar, SLOT(setRange(int min, int max)));


    //RANGE
    //ui->filesProgressBar->setValue(0);

    //ui->filesProgressBar->setRange(0, m_files->size());

    //ui->filesProgressBar->setFormat("Processed Files: %v / %m");
}	 */

/*
void TFZip::newdata2(std::vector<std::pair<std::wstring, std::wstring>> * files)
{
    auto blah = std::make_unique<std::vector<std::pair<std::wstring, std::wstring>>>();
    size_t filecount = files->size();
    //for (size_t i = filecount; i --> 0;)  fun way of doing this in one line. pre-decrements i and also prevents overflow
    for (size_t i = filecount - 1; i < filecount; --i)
    {
        std::pair<std::wstring, std::wstring> oiereer;
        oiereer.first = files->at(i).first;
        oiereer.second = files->at(i).second;
        files->pop_back();
        blah->push_back(oiereer);
    }
    std::reverse(blah->rbegin(), blah->rend());

    //TODO: Duplicates
    m_Files.reset();
    m_Files.swap(blah);
    blah.reset();

    this->DoParallel();
}	*/

//Q_DECLARE_METATYPE(QList<QPair<QString, QString>>);


void doaaaaaaaaa(QString sourcefile)
{

}


void TFZip::on_startButton_clicked()
{
    if (pause_token == true)
    {
        pause_token = false;
        return;
    }
    cancellation_token = false;
    ui->stopButton->setEnabled(true);
    ui->pauseButton->setEnabled(true);
    ui->startButton->setEnabled(false);

    QString sourceInputText = ui->sourceInput->text();
    QString destInputText = ui->destInput->text();

    //Always just use / internally.
    QString sourcepath = QDir::cleanPath(sourceInputText) + "/";
    QString destpath = QDir::cleanPath(destInputText);

    QDirIterator SourceDirIterator(sourcepath, QDir::NoDotAndDotDot | QDir::NoSymLinks
        | QDir::AllEntries, QDirIterator::Subdirectories);

    //File list is pre-pended to, so it is in reverse order so we can use pop_back for perf.
    auto filelist = new std::vector<std::pair<QString, QString>>();
    int iteri = 0;
    m_filescancount = 0;

    //QFutureSynchronizer<void> _synchronizer;

    while (SourceDirIterator.hasNext()) {

        SourceDirIterator.next();

        if ((cancellation_token))
        {
            delete filelist;
            //emit finished();
            return;
        }

        while ((pause_token))
        {
            qApp->processEvents();
            QThread::msleep(10);
        }

        QFileInfo srcpathinfo = QFileInfo(QDir::cleanPath(SourceDirIterator.fileInfo().filePath()));
        srcpathinfo.setCaching(false);

        //Skip if somehow not or file
        if (!srcpathinfo.isFile()) continue;

        iteri++;
        m_filescancount++;
        if (iteri % 1 == 0) {
            //emit updatebar(1);
            //QMetaObject::invokeMethod(ui->filesProgressBar, "setFormat", Qt::QueuedConnection, Q_ARG(QString, "Getting Files: " + QString::number(m_filescancount)));
            ui->filesProgressBar->setFormat(QString("Getting Files: " + QString::number(m_filescancount)));
            //QMetaObject::invokeMethod(this, "fileScanProgress", Qt::QueuedConnection, Q_ARG(int, 1));
            qApp->processEvents();
            iteri = 0;
        }

        //Remove beginning root path only
        //Don't use replace with a string,
        //or else we may remove linux dirs of the same names
        QString destfilepath = srcpathinfo.filePath();
        int loc = destfilepath.indexOf(sourcepath);
        destfilepath.replace(loc, sourcepath.length(), "");

        QFileInfo destfileinfo = QFileInfo(QDir::cleanPath(destpath + "/" + destfilepath));
        destfileinfo.setCaching(false);
        std::pair<QString, QString> filepair;
        QString blah = destfileinfo.filePath();
        QString blah2 = srcpathinfo.filePath();

        filepair.first = srcpathinfo.filePath();
        filepair.second = destfileinfo.filePath();
        //TODO: Optimize with views
        filelist->insert(filelist->begin(), filepair);

        //auto pool = new QThreadPool(this);
        int maxthreads = m_logicals;
        QThreadPool::globalInstance()->setMaxThreadCount(maxthreads);
        int poola2 = QThreadPool::globalInstance()->activeThreadCount();
        if (!(QThreadPool::globalInstance()->activeThreadCount() >= maxthreads) && SourceDirIterator.hasNext())
        {
            size_t filecount = filelist->size();
            for (size_t i = filecount - 1; i < filecount; --i)
            {
                qApp->processEvents();
                if (cancellation_token)
                {
                    return;
                }
                while (pause_token)
                {
                    qApp->processEvents();
                    QThread::msleep(10);
                }
                //Break out of loop if we've saturated all threads
                //This allows the file searcher to keep going
                if (QThreadPool::globalInstance()->activeThreadCount() >= maxthreads)
                {
                    break;
                }
                std::pair<QString, QString> *fromlist1 = &filelist->at(i);
                QString srcfromlist1 = fromlist1->first;
                QString destfromlist1 = fromlist1->second;

                filelist->pop_back();
                BZProgressBar * BZPbar = PBarLayout->reserveProgressPathID(srcfromlist1);
                if (BZPbar == false)
                {
                    //TODO: This isn't supposed to happen, be a better grammer pls.
                    //TODO: idea: Impliment a lock
                    //TODO: That didn't work the way I expected it
                    //TODO: Time wasted implimenting this: 10 hours
                    //TODO: Why even
                    //We'll send the progress bar along anyways. The runnable understands a false Pbar. We just won't have a Pbar for this runnable.
                }
                else {
                    BZPbar->setState(BZProgressBar::State::Compressing);
                }
                QRunnable* BzipRunnable1 = new ParallelRunnable(PBarLayout, BZPbar, srcfromlist1, destfromlist1, &cancellation_token, &pause_token);

                //QFuture<void> future = QtConcurrent::run(std::bind(&ConcurrentWorker::run, PBarLayout, BZPbar, srcfromlist1, destfromlist1, &cancellation_token, &pause_token));
                //m_superfuturewatcher->addFuture(future);

                //QFutureWatcher<int> watcher;
                //connect(&watcher, SIGNAL(finished()), this, SLOT(handleFinished()));
                //QObject::connect(BzipRunnable1, &ParallelRunnable::updateProg, this, [=]() { emit updatesbar(12); });

                BzipRunnable1->setAutoDelete(true);
                QThreadPool::globalInstance()->start(BzipRunnable1);
            }

        }

        //If there's no more files to find, forever loop over the list of files until done or interrupted.
        if (!SourceDirIterator.hasNext())
        {
            size_t filecount = filelist->size();
            for (size_t i = filecount - 1; i < filecount; --i)
            {
                //Keep waiting until we can process the file.
                while (QThreadPool::globalInstance()->activeThreadCount() >= maxthreads)
                {
                    qApp->processEvents();
                    if (cancellation_token)
                    {
                        return;
                    }
                    while (pause_token)
                    {
                        qApp->processEvents();
                        QThread::msleep(10);
                    }
                    int poola3 = QThreadPool::globalInstance()->activeThreadCount();
                    QThread::msleep(10);
                }
                qApp->processEvents();
                if (cancellation_token)
                {
                    return;
                }
                while (pause_token)
                {
                    qApp->processEvents();
                    QThread::msleep(10);
                }
                std::pair<QString, QString> *fromlist1 = &filelist->at(i);
                QString srcfromlist1 = fromlist1->first;
                QString destfromlist1 = fromlist1->second;
                filelist->pop_back();
                BZProgressBar * BZPbar = PBarLayout->reserveProgressPathID(srcfromlist1);
                if (BZPbar == false)
                {
                    //TODO: This isn't supposed to happen, be a better grammer pls.
                    //TODO: idea: Impliment a lock
                    //TODO: That didn't work the way I expected it
                    //TODO: Time wasted implimenting this: 10 hours
                    //TODO: Why even
                    //We'll send the progress bar along anyways. The runnable understands a false Pbar. We just won't have a Pbar for this runnable.
                    //TODO: Waste more time on this
                }
                else {
                    BZPbar->setState(BZProgressBar::State::Compressing);
                }
                QRunnable* BzipRunnable1 = new ParallelRunnable(PBarLayout, BZPbar, srcfromlist1, destfromlist1, &cancellation_token, &pause_token);

                BzipRunnable1->setAutoDelete(true);
                QThreadPool::globalInstance()->start(BzipRunnable1);
            }
        }
        //BZipCompressor * aybb = new BZipCompressor(&cancellation_token, &pause_token);

        //aybb->simpleBZ2Compress(blah2, blah);

        //bool started = pool->tryStart(BzipRunnable);

        //future = QtConcurrent::run(aybb, &BZipCompressor::simpleBZ2Compress, blah2, blah);

        //Try to delete file if we got errors.
        /*if (!aybb->noerrors)
        {
        QFile file(QString::fromStdWString(blah));
        file.remove();
        }  */

    }
    qApp->processEvents();
    //qRegisterMetaType<std::list<std::pair<std::wstring, std::wstring>>>("std::list<std::pair<std::wstring, std::wstring>>");
    //qRegisterMetaType<std::vector<std::pair<std::wstring, std::wstring>> *>("std::vector<std::pair<std::wstring, std::wstring>> *");
    //Yes, I know. This is ugly, but as far as I know its the only way to pass a large set(over 100MB) of data, without QT automatically copying it.
    //This MUST be under a BlockingQueuedConnection!
    //emit updatedata(filelist);
    delete filelist;


}
 /*
void TFZip::on_startButton_clicked()
{
    if (pause_token) {
        m_future.setPaused(false);
        m_futureWatcher.setPaused(false);
        pause_token = false;
        PBars->changeAllPBarStates(ProgressBars::State::Compressing);
        return;
    }

    QString sourceInputText = ui->sourceInput->text();
    QString destInputText = ui->destInput->text();

    //Always just use / internally.
    QString sourcepath = QDir::cleanPath(sourceInputText) + "/";
    QString destpath = QDir::cleanPath(destInputText);

    fileSearchThread = new QThread;
    auto thread = fileSearchThread;
    thread->wait();
    FileSearchWorker* worker = new FileSearchWorker(sourcepath, destpath, &pause_token, &cancellation_token);
    worker->moveToThread(thread);
    connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(process()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(updatebar(int)), this, SLOT(fileScanProgress(int)));
    //blocking, because our data is a pointer. If you change this, you'll break a lot of things.
    //connect(worker, SIGNAL(updatedata(std::vector<std::pair<std::wstring, std::wstring>> *)), this, SLOT(newdata2(std::vector<std::pair<std::wstring, std::wstring>> *)), Qt::BlockingQueuedConnection);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, &QThread::finished, this, [=]() { m_FileSearchRunning = false; ui->startButton->setEnabled(true); }, Qt::BlockingQueuedConnection);
    thread->start();

     */
    /*
    QDirIterator SourceDirIterator(sourcepath, QDir::NoDotAndDotDot | QDir::NoSymLinks
        | QDir::AllEntries, QDirIterator::Subdirectories);

    //File list is pre-pended to, so it is in reverse order so we can use pop_back for perf.
    auto filelist = new std::vector<std::pair<std::wstring, std::wstring>>();
    int iteri = 0;
    while (SourceDirIterator.hasNext()) {

        SourceDirIterator.next();

        if ((cancellation_token))
        {
            delete filelist;
            //emit finished();
            return;
        }

        while ((pause_token))
        {
            QThread::msleep(200);
        }

        QFileInfo srcpathinfo = QFileInfo(QDir::cleanPath(SourceDirIterator.fileInfo().filePath()));
        srcpathinfo.setCaching(false);

        //Skip if somehow not or file
        if (!srcpathinfo.isFile()) continue;

        iteri++;
        if (iteri % 2 == 0) {
            //emit updatebar(2);
            //QMetaObject::invokeMethod(this, "fileScanProgress", Qt::QueuedConnection, Q_ARG(int, 25));
            iteri = 0;
        }

        //Remove beginning root path only
        //Don't use replace with a string,
        //or else we may remove linux dirs of the same names
        QString destfilepath = srcpathinfo.filePath();
        int loc = destfilepath.indexOf(sourcepath);
        destfilepath.replace(loc, sourcepath.length(), "");

        QFileInfo destfileinfo = QFileInfo(QDir::cleanPath(destpath + "/" + destfilepath));
        destfileinfo.setCaching(false);
        std::pair<std::wstring, std::wstring> filepair;
        std::wstring blah = destfileinfo.filePath().toStdWString();
        std::wstring blah2 = srcpathinfo.filePath().toStdWString();

        filepair.first = srcpathinfo.filePath().toStdWString();
        filepair.second = destfileinfo.filePath().toStdWString();
        //TODO: Optimize with views
        filelist->insert(filelist->begin(), filepair);

        auto pool = new QThreadPool(this);
        int maxthreads = 8;
        QThreadPool::globalInstance()->setMaxThreadCount(maxthreads);
        int poola2 = QThreadPool::globalInstance()->activeThreadCount();
        bool isbusy = QThreadPool::globalInstance()->activeThreadCount() >= maxthreads;
        qApp->processEvents();
        if (!(QThreadPool::globalInstance()->activeThreadCount() >= maxthreads) && SourceDirIterator.hasNext())
        {
            size_t filecount = filelist->size();
            for (size_t i = filecount - 1; i < filecount; --i)
            {
                //Break out of loop if we've saturated all threads
                //This allows the file searcher to keep going
                if (QThreadPool::globalInstance()->activeThreadCount() >= maxthreads)
                {
                    break;
                }
                int poola1 = QThreadPool::globalInstance()->activeThreadCount();
                std::pair<std::wstring, std::wstring> *fromlist1 = &filelist->at(i);
                std::wstring srcfromlist1 = fromlist1->first;
                std::wstring destfromlist1 = fromlist1->second;
                filelist->pop_back();
                QRunnable* BzipRunnable1 = new ParallelRunnable(srcfromlist1, destfromlist1, &cancellation_token, &pause_token);
                BzipRunnable1->setAutoDelete(true);
                QThreadPool::globalInstance()->start(BzipRunnable1);
            }

        }

        //If we're on the last file, iterate over the list of files
        if (!SourceDirIterator.hasNext())
        {
            //If our loop is done,
            size_t filecount = filelist->size();
            for (size_t i = filecount - 1; i < filecount; --i)
            {
                //Keep waiting until we can process the file.
                while (QThreadPool::globalInstance()->activeThreadCount() >= maxthreads)
                {
                    qApp->processEvents();
                    int poola3 = QThreadPool::globalInstance()->activeThreadCount();
                    QThread::msleep(10);
                }
                std::pair<std::wstring, std::wstring> *fromlist1 = &filelist->at(i);
                std::wstring srcfromlist1 = fromlist1->first;
                std::wstring destfromlist1 = fromlist1->second;
                filelist->pop_back();
                QRunnable* BzipRunnable1 = new ParallelRunnable(srcfromlist1, destfromlist1, &cancellation_token, &pause_token);
                BzipRunnable1->setAutoDelete(true);
                QThreadPool::globalInstance()->start(BzipRunnable1);
            }
        }
        //BZipCompressor * aybb = new BZipCompressor(&cancellation_token, &pause_token);

        //aybb->simpleBZ2Compress(blah2, blah);

        //bool started = pool->tryStart(BzipRunnable);

        //future = QtConcurrent::run(aybb, &BZipCompressor::simpleBZ2Compress, blah2, blah);
        */
        //Try to delete file if we got errors.
        /*if (!aybb->noerrors)
        {
            QFile file(QString::fromStdWString(blah));
            file.remove();
        }  */

    //}

     /*
    if (!m_FileSearchRunning)
    {
        m_Files.reset();
        ui->startButton->setEnabled(false);
        PBars->changeAllPBarStates(ProgressBars::State::Idle);
        m_FileSearchRunning = true;
        try {
            ui->filesProgressBar->setFormat("Enumerated Files: %v");

            fileSearchThread = new QThread;
            auto thread = fileSearchThread;
            thread->wait();
            FileSearchWorker* worker = new FileSearchWorker(sourcepath, destpath, &pause_token, &cancellation_token);
            worker->moveToThread(thread);
            connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
            connect(thread, SIGNAL(started()), worker, SLOT(process()));
            connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
            connect(worker, SIGNAL(updatebar(int)), this, SLOT(fileScanProgress(int)));
            //blocking, because our data is a pointer. If you change this, you'll break a lot of things.
            connect(worker, SIGNAL(updatedata(std::vector<std::pair<std::wstring, std::wstring>> *)), this, SLOT(newdata2(std::vector<std::pair<std::wstring, std::wstring>> *)), Qt::BlockingQueuedConnection);
            connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
            connect(thread, &QThread::finished, this, [=]() { m_FileSearchRunning = false; ui->startButton->setEnabled(true); }, Qt::BlockingQueuedConnection);
            thread->start();
        }
        catch (...)
        {
            m_FileSearchRunning = false;
            PBars->changeAllPBarStates(ProgressBars::State::Idle);
            ui->startButton->setEnabled(true);
        }
    }
    else {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Somehow you are trying to scan for more files, but file searching isn't false yet!"));
    }
    */
    /*
    return;
    ui->filesProgressBar->setFormat("Enumerated Files: %v");
    Task::future<void>& f1 = Task::run([sourcepath, destpath, this]() {
    });
    //connect(f1, SIGNAL("oi"), ui->filesProgressBar, SLOT(setValue));
    f1.start();
}	*/
