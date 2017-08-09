#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "okFrontPanelDLL.h"
#include "Thread_USBdata.h"
#include <qqueue.h>
#include "StopWatch.h"
#include <omp.h>

#define _DEBUG_THREAD
//extern "C" void launch_kernel();

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();
	//Opalkelly
	bool memswitch;
	int pval;
	int blocksize = 16384;
	int buffersize = blocksize * 128;
	bool FPGA_state = false;
	bool USB_state = false;
	okCFrontPanel *dev;
	QQueue<unsigned int> *queue;
	unsigned char* buffer;
	void setupRealtimeDataDemo(QCustomPlot *customPlot);
	void putImage1(QQueue<unsigned int> *, QVector<double> *, QVector<double> *);
	//unsigned char** gtemp;
private:
	
	Thread_USBdata *thread_usbdata;
	QString demoName;
	QTimer dataTimer;
	Ui::MainWindow *ui;
	private slots:
	//Opalkelly
	void Button_FPGA();
	void Button_USBdata();
	bool InitializeFPGA(okCFrontPanel *, char *, char *); //FrontPanel 초기화 함수
	//QT
	void realtimeDataSlot();
	
};



#endif // MAINWINDOW_H
