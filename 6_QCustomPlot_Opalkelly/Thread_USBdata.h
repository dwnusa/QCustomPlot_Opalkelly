#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qlabel.h>
//#include "myclass.h"
//#include "use_opencv.h"
#include "okFrontPanelDLL.h"
#include <qqueue.h>

#define _DEBUG_THREAD

class Thread_USBdata : public QThread {
	Q_OBJECT

public:
	Thread_USBdata(okCFrontPanel *, QQueue<unsigned int> *);
	void stop();
	void resume();
	QWaitCondition m_waitCondition;
	QMutex m_mutex;
	qint32 m_stopflag;
	
	unsigned char* buffer;
	int blocksize = 16384;
	int buffersize = blocksize * 128;
	QQueue<unsigned int> *m_queue;
	//QCircularBuffer<int> *integerBuffer;
	//QVector<quint8**> ringBuffer;
private:
	void run();
	bool ReleaseQueueAtOverload(QQueue<unsigned int>*, int);
	okCFrontPanel *m_dev;
};