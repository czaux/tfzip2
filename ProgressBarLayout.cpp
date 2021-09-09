#include "ProgressBarLayout.h"
#include <QProgressBar>

/*ProgressBarLayout::ProgressBarLayout()
{

} */

ProgressBarLayout::~ProgressBarLayout()
{
}

void ProgressBarLayout::addBZPBar(QString path)
{
}


void ProgressBarLayout::changeAllPBarStates(BZProgressBar::State statetype)
{
	//Iterate through all children QObjects in this layout
	for (int i = 0; i < this->count(); i++)
	{
		//Get our QObject from the list
		QLayoutItem * BZPbarqObj = this->itemAt(i);
		//Get our QObject from the list
		//Cast to our progress bar type. the cast understands BZProgressBar is a subclass of QProgressBar, which is a QObject.
		if (BZProgressBar * BZPbar1 = qobject_cast<BZProgressBar *>(BZPbarqObj->widget()))
		{
			//simply set the state
			BZPbar1->setState(statetype);
		}
	}


}

void ProgressBarLayout::GetPBarByID(QString path)
{

}

BZProgressBar * ProgressBarLayout::reserveProgressPathID(QString path)
{
	/* Finds a progress bar that has an empty value, and fills it with the given path
	* Returns false is could not find an empty bar.
	*/

	//Iterate through all children QObjects in this layout
	for (int i = 0; i < this->count(); i++)
	{
		//Get our QObject from the list
		QLayoutItem * BZPbarqObj = this->itemAt(i);
		
		//Cast to our progress bar type. the cast understands BZProgressBar is a subclass of QProgressBar, which is a QObject.
		if (BZProgressBar * BZPbar1 = qobject_cast<BZProgressBar *>(BZPbarqObj->widget()))
		{
			if (BZPbar1->pathid.isEmpty())
			{
				BZPbar1->pathid = path;

				return BZPbar1;
			}
		}
	}

	return false;
}



BZProgressBar * ProgressBarLayout::releaseProgressPathID(QString path)
{
	/* Finds path in progress bars map that matches given string, and sets the progress bars value to empty */
	
	//Iterate through all children QObjects in this layout

	for (int i = 0; i < this->count(); i++)
	{
		QLayoutItem * BZPbarqObj = this->itemAt(i);

		//Cast to our progress bar type. the cast understands BZProgressBar is a subclass of QProgressBar, which is a QObject.
		if (BZProgressBar * BZPbar1 = qobject_cast<BZProgressBar *>(BZPbarqObj->widget()))
		{
			if (path.compare(BZPbar1->pathid) == 0)
			{
				BZPbar1->pathid = "";

				return BZPbar1;
			}
		}
	}

	return false;
}

void ProgressBarLayout::setProgressValue(QString path, int value)
{

}

void ProgressBarLayout::setProgressFormatTest(QString path, QString format)
{

}