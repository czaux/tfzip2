#include "BZProgressBar.h"
#include <qDebug>

void BZProgressBar::setState(State statetype)
{
	QString cstyle =
R"(
QProgressBar {
    border: 1px solid grey;
    border-radius: 4px;
    text-align: center;
}

QProgressBar::chunk:after{
    background-color: #CD96CD;
    width: 10px;
}

QProgressBar::chunk:before{
    background-color: #CD96CD;
    width: 10px;
    margin: 0.5px;
}
)";

	QString vstyle =
		R"(
QProgressBar {
    border: 1px solid grey;
    border-radius: 4px;
    text-align: center;
}

QProgressBar::chunk:after{
    background-color: rgb(171, 160, 255);
    width: 10px;
}

QProgressBar::chunk:before{
    background-color: rgb(171, 160, 255);
    width: 10px;
    margin: 0.5px;
}
)";

	this->setFormat(stateformat[statetype]);
	switch (statetype) {
	case Idle:
		break;
	case Compressing:
		this->setStyleSheet(cstyle);
		break;
	case Validating:
		this->setStyleSheet(vstyle);
		break;
	case Paused:
		break;
	case Stopped:
		break;
	}
}

   /*
void BZProgressBar::changeAllPBarStates(State statetype)
{
	for (auto& bar : this->bars) {
		changePBarState(bar, statetype);
	}
}


void BZProgressBar::changePBarState(std::pair<QProgressBar *, ValueState> bar, State statetype)
{
	const char *style =
		R"(QProgressBar{
border : 1px solid grey;
border - radius: 4px;
text - align: center;
}

QProgressBar::chunk:after{
background - color: #CD96CD;
width: 10px;

}
QProgressBar::chunk : before{
background - color: #CD96CD;
width: 10px;
margin:0.5px;
}")";
	bar.first->setFormat(stateformat[statetype]);
	bar.first->setValue(0);
	switch (statetype) {
	case Idle:
		break;
	case Compressing:
		bar.first->setStyleSheet(style);
		break;
	case Validating:
		break;
	case Paused:
		break;
	case Stopped:
		break;
	}
}
  */
/* Finds a progress bar that has an empty value, and fills it with the given path
* Returns false is could not find an empty bar.
*/
/*
bool BZProgressBar::reserveProgressPathID(QString path) {
	for (auto& bar : this->bars) {
		if (bar.second.uniquepath.isEmpty())
		{
			bar.second.uniquepath = path;
			return true;
		}
	}
	return false;
} */

/* Finds path in progress bars map that matches given string, and sets the progress bars value to empty */
/*
bool BZProgressBar::releaseProgressPathID(QString path) {
	for (auto& bar : this->bars) {
		if (path.compare(bar.second.uniquepath) == 0)
		{
			//set it back to empty
			bar.second.uniquepath = "";
			return true;
		}
	}
	return false;
}

void BZProgressBar::setProgressValue(QString path, int value) {
	for (auto& bar : this->bars) {
		if (path.compare(bar.second.uniquepath) == 0)
		{
			bar.first->setValue(value);
			return;
		}
	}
}

void BZProgressBar::setProgressFormatTest(QString path, QString format)
{
	for (auto& bar : this->bars) {
		if (path.compare(bar.second.uniquepath) == 0)
		{
			bar.first->setFormat(format);
			return;
		}
	}
}	   */