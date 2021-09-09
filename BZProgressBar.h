#pragma once
#include <QObject>
#include <QWidget>
#include <QBoxLayout>
#include <QProgressBar>
#include <QList>

class BZProgressBar : public QProgressBar
{
	Q_OBJECT

public:

	int minrange = 0;
	int maxrange = 1000;
	int numbars;

	enum State {
		Idle,
		Compressing,
		Validating,
		Paused,
		Stopped
	};

	struct ValueState {
		State state;
		QString uniquepath;
	};

	QString pathid;

	//Each progress bar is associated with a string, in implimentation it is a file path.
	std::map<QProgressBar *, ValueState> bars;

	std::map<State, QString> stateformat;

	//Create progress bars into layout element on construction
	explicit BZProgressBar(QWidget *parent = 0)
	{
		pathid = "";
		stateformat[Idle]        = "Idle";
		stateformat[Compressing] = "Compressing: %p%";
		stateformat[Validating]  = "Validating: %p%";
		stateformat[Paused]      = "Paused: %p%";
		stateformat[Stopped]     = "Stopped";

	};

public slots:

	void setState(BZProgressBar::State statetype);

};