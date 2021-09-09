#pragma once
#include <QException>
class WorkerException : public QException
{
	public:
	WorkerException(QString const& message) :
		message(message)
	{}

	virtual ~WorkerException()
	{

	}

	void raise() const { 
		throw *this; 
	}
	WorkerException *clone() const { return new WorkerException(*this); }

	QString getMessage() const
	{
		return message;
	}
	private:
		QString message;
};