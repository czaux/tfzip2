#pragma once

#ifndef TFZIP_HEADER
#define TFZIP_HEADER

#include <QtWidgets/QMainWindow>
#include "ui_tfzip2.h"
#include <QFileInfo>
#include <QFuture>
#include <QtCore/QFutureWatcher>
#include <QObject>
#include <QProgressBar>
#include <atomic>
#include <memory>
#include "ProgressBarLayout.h"
#include "BZProgressBar.h"


#ifdef _WIN32
    extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

namespace Ui {
    class TFZipClass;
    class FileErrorDialog;
}

typedef QMap<QString, QString> SuperMap;

class TFZip : public QMainWindow
{
    Q_OBJECT

public:
    explicit TFZip(QWidget *parent = 0);
    ~TFZip();

    //TFZip(QWidget *parent = Q_NULLPTR);

    uint64_t m_filescancount;

    void doathing43(std::pair<std::wstring, std::wstring> files);

    std::atomic<bool> cancellation_token{ false };

    std::atomic<bool> pause_token{ false };

    QFutureWatcher<void> m_futureWatcher;

    QFuture<void> m_future;

    ProgressBarLayout * PBarLayout;

    //SuperFutureWatcher * m_superfuturewatcher;

signals:
    void reservePathID(QString);
    void releasePathID(QString);

private slots:

    void allfilesProgressValueSlot(int value);

    void on_sourceBrowse_clicked();

    void on_destBrowse_clicked();

    void fileScanProgress(int filenum);

    //void newdata2(std::vector<std::pair<std::wstring, std::wstring>> * files);

    //void DoParallel();

    void closeEvent(QCloseEvent *event);

    void on_sourceInput_textChanged(const QString &arg1);

    void on_destInput_textChanged(const QString &arg1);

    void processUserInput();

    //void doathing43(const QFileInfo &filepath);

    QList<QFileInfo> Dothings(const QString path);

    void on_startButton_clicked();

    void on_pauseButton_clicked();

    void on_stopButton_clicked();
    //void doathing43(QString &filepath);

private:
    //http://blackberry.github.io/Qt2Cascades-Samples/docs/qtconcurrent-progressdialog-src-progressdialog-hpp.html

    int m_logicals;

    bool m_FileSearchRunning = false;

    Ui::TFZipClass *ui;
};
#endif TFZIP_HEADER
