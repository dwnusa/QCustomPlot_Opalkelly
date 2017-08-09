//#include "stdafx.h"
#include "StopWatch.h"

CStopWatch::CStopWatch()
{
	m_swFreq.LowPart = m_swFreq.HighPart = 0;
	m_swStart = m_swFreq;
	m_swEnd = m_swFreq;
	m_fTimeforDuration = 0;

	QueryPerformanceFrequency(&m_swFreq);
}

CStopWatch::~CStopWatch()
{
}

void CStopWatch::Start()
{
	QueryPerformanceCounter(&m_swStart);
}

void CStopWatch::End()
{
	QueryPerformanceCounter(&m_swEnd);
	m_fTimeforDuration = (m_swEnd.QuadPart - m_swStart.QuadPart) / (float)m_swFreq.QuadPart;
}