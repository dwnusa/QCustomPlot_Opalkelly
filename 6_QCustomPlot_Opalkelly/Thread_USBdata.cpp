#include "Thread_USBdata.h"
#include <qqueue.h>

#define STOP 0
#define PLAY 1

Thread_USBdata::Thread_USBdata(okCFrontPanel *dev, QQueue<unsigned int> *queue)
{
	m_dev = (okCFrontPanel *)dev;
	m_stopflag = PLAY;
	m_queue = (QQueue<unsigned int> *)queue;
	//buffer = new unsigned char[buffersize];
}

void Thread_USBdata::run()
{
#ifdef _DEBUG_THREAD
	int CurrentID = (int)currentThreadId();//QThread::currentThreadId();
#endif
	for (int count = 0;;) {
		m_mutex.lock();
		if (m_stopflag == STOP)
			m_waitCondition.wait(&m_mutex);
		m_mutex.unlock();
#if 1
		// µ•¿Ã≈Õ »πµÊ Ω««Ë
		
		buffer = new unsigned char[buffersize];
		okCFrontPanel::ErrorCode error_mess;

		if (m_dev->IsOpen() == true)
		{
			error_mess = (okCFrontPanel::ErrorCode)m_dev->ReadFromBlockPipeOut(
				0xA0, blocksize, buffersize, buffer); //µ•¿Ã≈Õ »πµÊ
			if (error_mess >= 0)
			{
				m_queue->enqueue((unsigned int)buffer);
				ReleaseQueueAtOverload(m_queue, 700);
			}
		}
		//delete[] buffer;
		//int kk = 1;
		//unsigned char val, red, green, blue;
		//val = 255;
		//for (int i = 0; i < buffersize - 4; i += 4)
		//{
		//	int pos = ((buffer[i + 7] & 0xff) << 24)
		//		| ((buffer[i + 6] & 0xff) << 16)
		//		| ((buffer[i + 5] & 0xff) << 8)
		//		| (buffer[i + 4] & 0xff);
		//	int x = pos % 500;
		//	int y = pos / 500;
		//	//image.setPixel(x, y, qRgb(val, val, val));
		//}
		//ui.label->setPixmap(QPixmap::fromImage(image));

		//delete[] buffer;
		// µ•¿Ã≈Õ »πµÊ Ω««Ë   
#endif // 1
	}
}

void Thread_USBdata::stop()
{
	m_stopflag = STOP;
}

void Thread_USBdata::resume()
{
	m_mutex.lock();
	m_stopflag = PLAY;
	m_waitCondition.wakeAll();
	m_mutex.unlock();
}

bool Thread_USBdata::ReleaseQueueAtOverload(QQueue<unsigned int>* queue, int threshold)
{
	if (queue->size() >= threshold)
	{
		int size = queue->size();
		unsigned char** temp = new unsigned char*[size];
		for (int i = 0; i < size; i++)
		{
			temp[i] = (unsigned char*)queue->dequeue();
		}
		for (int i = 0; i < size; i++)
			delete[] temp[i];
		delete[] temp;
		return true;
	}
	return false;
}