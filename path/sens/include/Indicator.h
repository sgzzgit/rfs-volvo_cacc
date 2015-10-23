// Indicator.h: interface for the CvIndicator class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __indicator_h__
#define __indicator_h__

#include <cv.h>

class CvIndicator
{
public:
	CvIndicator();
	virtual void SetLabel(char *label) { strcpy(m_Label, label); }
	virtual void Display(IplImage *pImage, CvPoint position, double value) {}

protected:
	char m_Label[256];
};

class CvIndicatorDVI : CvIndicator
{
public:
	CvIndicatorDVI(int xSize, int ySize) : CvIndicator()
	{
		m_XSize = xSize;
		m_YSize = ySize;
	}

	void Display(IplImage *pImage, CvPoint position, double value, bool yellow = true, bool red = false);

protected:
	int m_XSize;
	int m_YSize;
};

class CvIndicatorHistoryBar : public CvIndicator
{
public:
	CvIndicatorHistoryBar(int xSize, int ySize, int numHistory, double max, double min = 0, double guideLine = 0);
	~CvIndicatorHistoryBar();

	void Display(IplImage *pImage, CvPoint position, double value);

protected:
	int m_XSize;
	int m_YSize;
	int m_NumHistory;
	double m_GuideLine;
	double m_Range;
	double m_MinValue;

	double *m_History;
};


class CvCaptionText : public CvIndicator
{
public:
	void Display(IplImage *pImage, CvPoint position, char *str);
};

#endif // __indicator_h__
