#pragma once
#include <QObject>
#include <QLayout>
#include "BZProgressBar.h"

class ProgressBarLayout : public QBoxLayout {
	Q_OBJECT

public:
	ProgressBarLayout() : QBoxLayout(Direction::TopToBottom, 0)
	{
	}
	~ProgressBarLayout();

	void addBZPBar(QString path);

public slots:

	void changeAllPBarStates(BZProgressBar::State statetype);

	void GetPBarByID(QString path);

	BZProgressBar * reserveProgressPathID(QString path);

	BZProgressBar * releaseProgressPathID(QString path);

	void setProgressValue(QString path, int value);

	void setProgressFormatTest(QString path, QString format);

private:
	Q_DISABLE_COPY(ProgressBarLayout)

};