// Indicator.cpp: implementation of the CvIndicator class.
//
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <zulib.h>
#include <cv.h>
#include <cvzulib.h>
#include "Indicator.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///
/// Init Font
///

static bool g_HaveFont = false;
static CvFont g_Font;

CvIndicator::CvIndicator()
{
	if (!g_HaveFont)
	{
		cvInitFont(&g_Font, CV_FONT_VECTOR0, 0.5, 0.5);
		g_HaveFont = true;
	}

	m_Label[0] = '\0';
}

///
/// SamTrans DVI
///

#define DVI_LEVEL 7
void CvIndicatorDVI::Display(IplImage *pImage, CvPoint position, double value, bool yellow, bool red)
{
	int v = cvRound(value);

	CvScalar color = CV_RGB(255,255,0);
	if (yellow && !red) color = CV_RGB(255,255,0); // yellow
	else if (yellow && red) color = CV_RGB(255,128,0); // orange
	else if (!yellow && red) color = CV_RGB(255,0,0); // red
	else if (v != 0) color = CV_RGB(255,0,255); // purple (error)

	int box_width = m_XSize;
	double box_height = (m_YSize-1.0)/DVI_LEVEL;
	int l = int(Min(box_width,box_height)/10)+1;

	int i;
	for (i = 0; i < DVI_LEVEL; i++)
	{
		CvPoint p1, p2;
		p1.x = l, p2.x = box_width-l;
		p1.y = cvRound(i*box_height)+l, p2.y = cvRound((i+1)*box_height)-l;
		int line_width = l;
		if (i < v)
		{
			cvRectangle(pImage, position+p1, position+p2, color, CV_FILLED);
		}
		cvRectangle(pImage, position+p1, position+p2, CV_RGB(255,255,0), l);
	}
}

///
/// History Bar
///

CvIndicatorHistoryBar::CvIndicatorHistoryBar(int xSize, int ySize, int numHistory, double max, double min, double guideLine) : CvIndicator()
{
	m_XSize = xSize;
	m_YSize = ySize;
	m_NumHistory = numHistory;
	if (guideLine != 0)
	{
		if (guideLine < min) guideLine = min;
		if (guideLine > max) guideLine = max;
	}
	m_GuideLine = guideLine;
	m_Range = max-min;
	if (m_Range <= 0) m_Range = 1;
	m_MinValue = min;
	m_History = new double[m_NumHistory];
	for (int i = 0; i < m_NumHistory; i++)
	{
		m_History[i] = m_MinValue;
	}
}

CvIndicatorHistoryBar::~CvIndicatorHistoryBar()
{
	delete m_History;
}

void CvIndicatorHistoryBar::Display(IplImage *pImage, CvPoint position, double value_ori)
{
	double value = value_ori;

	if (value < m_MinValue) value = m_MinValue;
	if (value-m_MinValue > m_Range) value = m_MinValue + m_Range;

	CvPoint pt1, pt2;
	int i;
	int text_height = 20;
	if (m_Label[0] == '\0') text_height = 0;
	double bar_width = (m_XSize-1.0)/(m_NumHistory+3);
	int bar_height = m_YSize-text_height;
	int l = int(bar_width/10)+1;

	pt1.x = 0, pt1.y = 0;
	pt2.x = m_XSize, pt2.y = m_YSize;
	cvRectangle(pImage, pt1+position, pt2+position, cvScalar(0,0,0), CV_FILLED);

	if (text_height)
	{
		static char label[256];
		sprintf(label, "%s: %d", m_Label, cvRound(value_ori));
		pt1.x = l+l, pt1.y = text_height-l;
		cvPutText(pImage, label, pt1+position, &g_Font, CV_RGB(255,255,255));
	}

	CvPoint p0 = position;
	p0.y += text_height;
	double color_step = 100.0/m_NumHistory;
	for (i = 0; i < m_NumHistory; i++)
	{
		if (m_History[i] == m_MinValue) continue;

		pt1.x = m_XSize - (cvRound(i*bar_width) + l);
		pt2.x = m_XSize - (cvRound((i+1)*bar_width) - l);
		pt1.y = bar_height - l;
		pt2.y = bar_height - cvRound((bar_height-l-l)*(m_History[i]-m_MinValue)/m_Range);

		int c = cvRound(color_step*(i+1))+25;
		cvRectangle(pImage, pt1+p0, pt2+p0, CV_RGB(c,c,0), CV_FILLED);
	}

	if (value != m_MinValue)
	{
		pt1.x = m_XSize - (cvRound(i*bar_width) + l);
		pt2.x = m_XSize - (cvRound((i+3)*bar_width) - l);
		pt1.y = bar_height - l;
		pt2.y = bar_height - cvRound((bar_height-l-l)*(value-m_MinValue)/m_Range);
		cvRectangle(pImage, pt1+p0, pt2+p0, CV_RGB(255,255,0), CV_FILLED);
	}

	if (m_GuideLine != 0)
	{
		pt1.x = 0;
		pt2.x = cvRound((i+2)*bar_width);
		pt1.y = pt2.y = bar_height - cvRound((bar_height-l-l)*(m_GuideLine-m_MinValue)/m_Range);
		cvLine(pImage, pt1+p0, pt2+p0, CV_RGB(255,128,0), 1);
	}

	for (i = 0; i < m_NumHistory-1; i++)
	{
		m_History[i] = m_History[i+1];
	}
	m_History[i] = value;
}

///
/// Caption Text
///

void CvCaptionText::Display(IplImage *pImage, CvPoint position, char *str)
{
	cvPutText(pImage, str, position, &g_Font, CV_RGB(255,255,255));
}

