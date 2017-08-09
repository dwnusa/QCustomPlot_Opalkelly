#include "mainwindow.h"



#if defined(_WIN32)
#define strncpy strncpy_s
#define sscanf  sscanf_s
#elif defined(__linux__) || defined(__APPLE__)
#endif
#if defined(__QNX__)
#define clock_t   uint64_t
#define clock()   ClockCycles()
#define NUM_CPS   ((SYSPAGE_ENTRY(qtime)->cycles_per_sec))
#else
#define NUM_CPS   CLOCKS_PER_SEC
#endif

typedef unsigned int UINT32;


#define MIN(a,b)   (((a)<(b)) ? (a) : (b))


#define OK_PATTERN_COUNT         0
#define OK_PATTERN_LFSR          1
#define OK_PATTERN_WALKING1      2
#define OK_PATTERN_WALKING0      3
#define OK_PATTERN_HAMMER        4
#define OK_PATTERN_NEIGHBOR      5

//#define QCUSTOMPLOT_USE_OPENGL

okTDeviceInfo  m_devInfo;
bool           m_bCheck;
bool           m_bInjectError;
int            m_ePattern;
UINT32         m_u32BlockSize;
UINT32         m_u32SegmentSize;
UINT32         m_u32TransferSize;
UINT32         m_u32ThrottleIn;
UINT32         m_u32ThrottleOut;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	

	ui->setupUi(this);
	//launch_kernel();
	

	dev = new okCFrontPanel;

	queue = new QQueue<unsigned int>;

	thread_usbdata = new Thread_USBdata(dev, queue); //DAQ Polling Thread 생성

	setupRealtimeDataDemo(ui->customPlot); //Plot 초기화 및 Timer 함수 Slot 설정
	//ui->customPlot->setOpenGl(true);
	setWindowTitle("QCustomPlot: " + demoName);
	statusBar()->clearMessage();
	//currentDemoIndex = demoIndex;
	ui->customPlot->replot();
}

MainWindow::~MainWindow()
{
	thread_usbdata->quit();
}


//제어 모듈
void MainWindow::Button_FPGA()
{
#ifdef _DEBUG_THREAD
	int CurrentID = GetCurrentThreadId();
#endif
	QFileDialog::Options options;
	options |= QFileDialog::DontUseNativeDialog;
	QString selectedFilter;
	QStringList fileslist = QFileDialog::getOpenFileNames(this,
		tr("QFileDialog::getOpenFileName()"),
		"/",
		tr("All Files (*);;Text Files (*.txt)"),
		&selectedFilter,
		options);

	QByteArray byteName;// = fileslist[0].toLocal8Bit();

	if (fileslist.size() == 1) //파일 하나만 선택했는지 확인
	{
		byteName = fileslist[0].toLocal8Bit();
	}
	else
	{
		FPGA_state = false;
		return;
	}

	char bitfile[128], serial[128];
	char *comm_type = "";//"serial";
	char *filename = byteName.data();
	strncpy(bitfile, filename/*argv[1]*/, 128);
	strncpy(serial, comm_type/*argv[1]*/, 128);

	if (false == InitializeFPGA(dev, bitfile, serial)) {
		printf("FPGA could not be initialized.\n");
		//delete dev;
		FPGA_state = false;
		return;
	}

	FPGA_state = true;
	USB_state = true;
	if (FPGA_state == true)
		ui->USBButton->setEnabled(true);
	else
		ui->USBButton->setEnabled(false);
}

void MainWindow::Button_USBdata()
{
#ifdef _DEBUG_THREAD
	int CurrentID = GetCurrentThreadId();
#endif
	if (USB_state == true)
	{
		ui->USBButton->setText(QApplication::translate("MyClass", "USB Off", 0));
		USB_state = false;
		//ThreadCheck
		
		if (thread_usbdata->isRunning() == false)
			thread_usbdata->start();
		else
			thread_usbdata->resume();

		dataTimer.start(0); // Interval 0 means to refresh as fast as possible
		//FPGA_Button_Manage
		ui->FPGAButton->setEnabled(false);
	}
	else
	{
#if 0
		ui->USBButton->setText(QApplication::translate("MyClass", "USB On", 0));
		USB_state = true;
		//ThreadCheck
		//thread_usbdata->stop();
		//delete[] buffer;
		ui->FPGAButton->setEnabled(true);
		return;
#endif
		ui->USBButton->setText(QApplication::translate("MyClass", "USB On", 0));
		USB_state = true;
		//dataTimer.start(0); // Interval 0 means to refresh as fast as possible
		dataTimer.stop();
		//ThreadCheck
		thread_usbdata->stop();
		ui->FPGAButton->setEnabled(true);
		return;
	}
}



void MainWindow::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
	// give the axes some labels:
	ui->customPlot->xAxis->setLabel("x");
	ui->customPlot->yAxis->setLabel("y");
	// set axes ranges, so we see all data:
	ui->customPlot->xAxis->setRange(-10000, 10*(buffersize / 4) + 10000);
	ui->customPlot->yAxis->setRange(-10000, 260000);
	// setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
	connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
}

void MainWindow::realtimeDataSlot()
{
	//if (queue->isEmpty() == false)
	int queuesize = queue->size();
	if (queuesize >= 10)
	{
		int fix_size = 10;
		int size = fix_size * buffersize / 4;
		
		QVector<double> x(size), y(size); // initialize with entries 0..100
		//for (int i = 0; i < size; i++)
		//{
		//	x[i] = i; //y[i] = 0;
		//}
	
		putImage1(queue, &x, &y);

		// create graph and assign data to it:
		//ui->customPlot->setOpenGl(true);

		CStopWatch watch1;
		watch1.Start();
		ui->customPlot->addGraph();
		ui->customPlot->graph(0)->setData(x, y);
		
		ui->customPlot->replot();
		watch1.End();
		float time1 = watch1.GetDurationMilliSecond();
	}
}

void MainWindow::putImage1(QQueue<unsigned int> *queue, QVector<double> *x, QVector<double> *y)
{
#if 0
	int size = 1;// queue->size();

	unsigned char** temp = new unsigned char*[size];
	for (int i = 0; i < size; i++)
	{
		temp[i] = (unsigned char*)queue->dequeue();
	}

	//	#pragma omp parallel num_threads(8)//temp[j]
	{
		//		#pragma omp for
		for (int i = 0; i < (buffersize - 4); i += 4)
		{
			int ii = i / 4;
			int pos = ((temp[0][i + 3] & 0xff) << 24)
				| ((temp[0][i + 2] & 0xff) << 16)
				| ((temp[0][i + 1] & 0xff) << 8)
				| (temp[0][i + 0] & 0xff);
			if (pos >= 0 && pos < 250000) {
				//(*y)[pos]++;
				(*y)[ii] = pos;
			}
			else
				int kk = 1;
		}
	}

	for (int i = 0; i < size; i++)
		delete[] temp[i];
	delete[] temp;
#else // 0
	int fix_size = 10;//1;// queue->size();
	unsigned char** temp;// = gtemp;
	temp = new unsigned char*[fix_size];
	//memswitch = false;

	for (int i = 0; i < fix_size; i++)
		temp[i] = (unsigned char*)queue->dequeue();
	//CStopWatch watch1;
	//CStopWatch watch2;
	//watch1.Start();
//#pragma omp parallel //num_threads(2)//temp[j]
	{
//#pragma omp for
		for(int j = 0; j < fix_size; j++)
			for (int i = 0; i < (buffersize - 3); i += 4)
			{
				int stepwidth = (buffersize / 4);
				int ii = i / 4;
				(*x)[j*stepwidth + ii] = j*stepwidth + ii;
				int pos = ((temp[j][i + 3] & 0xff) << 24)
					| ((temp[j][i + 2] & 0xff) << 16)
					| ((temp[j][i + 1] & 0xff) << 8)
					| (temp[j][i + 0] & 0xff);
				if (pos >= 0 && pos < 250000) {
					//(*y)[pos]++;
					(*y)[j*stepwidth + ii] = pos;
				}
				else
					int kk = 1;
			}
	}
	//watch1.End();
	/////
	//watch2.Start();
	//for (int i = 0; i < (buffersize - 3); i += 4)
	//{
	//	int ii = i / 4;
	//	int pos = ((temp[i + 3] & 0xff) << 24)
	//		| ((temp[i + 2] & 0xff) << 16)
	//		| ((temp[i + 1] & 0xff) << 8)
	//		| (temp[i + 0] & 0xff);
	//	if (pos >= 0 && pos < 250000) {
	//		//(*y)[pos]++;
	//		(*y)[ii] = pos;
	//	}
	//	else
	//		int kk = 1;
	//}
	//watch2.End();
	//float time1 = watch1.GetDurationMilliSecond();
	//float time2 = watch2.GetDurationMilliSecond();
	//if (time1 > time2)
	//	int kkk = 1;
	int kk = 1;
	for (int i = 0; i < fix_size; i++)
		delete[] temp[i];
	delete[] temp;
#endif
}





//초기화 모듈
bool MainWindow::InitializeFPGA(okCFrontPanel *dev, char *bitfile, char *serial)
{
	if (okCFrontPanel::NoError != dev->OpenBySerial(std::string(serial))) {
		printf("Device could not be opened.  Is one connected?\n");
		return(false);
	}
	dev->GetDeviceInfo(&m_devInfo);
	printf("Found a device: %s\n", m_devInfo.productName);

	dev->LoadDefaultPLLConfiguration();

	// Get some general information about the XEM.
	printf("Device firmware version: %d.%d\n", m_devInfo.deviceMajorVersion, m_devInfo.deviceMinorVersion);
	printf("Device serial number: %s\n", m_devInfo.serialNumber);
	printf("Device device ID: %d\n", m_devInfo.productID);

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(bitfile)) {
		printf("FPGA configuration failed.\n");
		return(false);
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (dev->IsFrontPanelEnabled()) {
		printf("FrontPanel support is enabled.\n");
	}
	else {
		printf("FrontPanel support is not enabled.\n");
		return(false);
	}

	return(true);
}