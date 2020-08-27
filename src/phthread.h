#ifndef PHTREAD_H
#define PHTREAD_H

#include <QThread>

class PhThread : public QThread {
	Q_OBJECT
public:
	virtual ~PhThread();

	virtual void run() override;

signals:
	void phTerminated(int result);
};

#endif
