
// EigenValuesDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
#include "EigenValues.h"
#include "EigenValuesDlg.h"
#include "afxdialogex.h"

#include <vector>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Диалоговое окно CEigenValuesDlg

#define DOTS(x,y) (xp*((x)-xmin)),(yp*((y)-ymax))
using namespace std;

void CEigenValuesDlg::Mashtab(double arr[], int dim, double* mmin, double* mmax)		//определяем функцию масштабирования
{
	*mmin = *mmax = arr[0];

	for (int i = 0; i < dim; i++)
	{
		if (*mmin > arr[i]) *mmin = arr[i];
		if (*mmax < arr[i]) *mmax = arr[i];
	}
}

CEigenValuesDlg::CEigenValuesDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EIGENVALUES_DIALOG, pParent)
	, a(5)
	, k(0)
	, epsilonK(0)
	, timestep(0.01)
	, espstep(0.01)
	, MPDesp(1.e-10)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEigenValuesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WIDTH, a);
	DDX_Control(pDX, IDC_RADIO_POT_PIT, m_potentialPit);
	DDX_Control(pDX, IDC_RADIO_PHASE_FUNC, m_phaseFunc);
	DDX_Text(pDX, IDC_EDIT_K_LINE, k);
	DDX_Text(pDX, IDC_EDIT_EK, epsilonK);
	DDX_Control(pDX, IDC_RADIO_WAVE_FUNC2, m_waveFunc);
	DDX_Text(pDX, IDC_EDIT_TIMESTEP, timestep);
	DDX_Text(pDX, IDC_EDIT_ESPSTEP, espstep);
	DDX_Text(pDX, IDC_EDIT_ACCURACY, MPDesp);
}

BEGIN_MESSAGE_MAP(CEigenValuesDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_GRAPH, &CEigenValuesDlg::OnBnClickedButtonGraph)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CEigenValuesDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_RADIO_POT_PIT, &CEigenValuesDlg::OnBnClickedRadioPotPit)
	ON_BN_CLICKED(IDC_RADIO_PHASE_FUNC, &CEigenValuesDlg::OnBnClickedRadioPhaseFunc)
	ON_BN_CLICKED(IDC_RADIO_WAVE_FUNC2, &CEigenValuesDlg::OnBnClickedRadioWaveFunc2)
	ON_EN_CHANGE(IDC_EDIT_K_LINE, &CEigenValuesDlg::OnEnChangeEditKLine)
	ON_BN_CLICKED(IDC_BUTTON_INFO, &CEigenValuesDlg::OnBnClickedButtonInfo)
END_MESSAGE_MAP()


// Обработчики сообщений CEigenValuesDlg

BOOL CEigenValuesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	PicWnd = GetDlgItem(IDC_PICTURE);			//связываем с ID окон
	PicDc = PicWnd->GetDC();
	PicWnd->GetClientRect(&Pic);

	grid_pen.CreatePen(		//для сетки
		PS_DOT,
		1,
		RGB(85, 85, 85));

	graph_pen.CreatePen(	//для графика
		PS_SOLID,
		6,
		RGB(255, 255, 0));

	axes_pen.CreatePen(		//для сетки
		PS_DOT,
		3,
		RGB(255, 255, 255));

	wall.CreatePen(		//для сетки
		PS_DOT,
		3,
		RGB(255, 0, 55));

	k_line.CreatePen(		//для сетки
		PS_SOLID,
		3,
		RGB(150, 0, 10));

	UpdateData(false);
	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CEigenValuesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CEigenValuesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEigenValuesDlg::DrawGraphic(double* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, double AbsMax)
{
	// поиск максимального и минимального значения
	if (m_potentialPit.GetCheck() == BST_CHECKED)
	{
		ymax = 1.1 * 10;
		ymin = -1.1 * 10;
		xmax = a * 1.2;
		xmin = -a * 1.2;

		// создание контекста устройства
		CBitmap bmp;
		CDC* MemDc;
		MemDc = new CDC;
		MemDc->CreateCompatibleDC(WinDc);

		double window_signal_width = WinPic.Width();
		double window_signal_height = WinPic.Height();
		xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
		yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

		bmp.CreateCompatibleBitmap(WinDc, window_signal_width, window_signal_height);
		CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
		// заливка фона графика белым цветом
		MemDc->FillSolidRect(WinPic, RGB(0, 0, 0));
		// отрисовка сетки координат
		MemDc->SelectObject(&axes_pen);

		//создаём Ось Y
		MemDc->MoveTo(DOTS(0, ymax));
		MemDc->LineTo(DOTS(0, ymin));
		//создаём Ось Х
		MemDc->MoveTo(DOTS(xmin, 0));
		MemDc->LineTo(DOTS(xmax, 0));

		//создаем бесконечно высокие стенки
		MemDc->SelectObject(&wall);
		MemDc->MoveTo(DOTS(-a, ymax));
		MemDc->LineTo(DOTS(-a, 0));

		MemDc->MoveTo(DOTS(a, ymax));
		MemDc->LineTo(DOTS(a, 0));

		MemDc->MoveTo(DOTS(-a, 0));
		MemDc->LineTo(DOTS(a, 0));

		MemDc->SelectObject(&grid_pen);
		//отрисовка сетки по y
		for (double x = xmin; x <= xmax; x += xmax / 6)
		{
			if (x != 0) {
				MemDc->MoveTo(DOTS(x, ymax));
				MemDc->LineTo(DOTS(x, ymin));
			}
		}
		//отрисовка сетки по x
		for (double y = 0; y < ymax; y += ymax / 4)
		{
			if (y != 0) {
				MemDc->MoveTo(DOTS(xmin, y));
				MemDc->LineTo(DOTS(xmax, y));
			}
		}
		for (double y = 0; y > ymin; y -= ymax / 4)
		{
			if (y != 0) {
				MemDc->MoveTo(DOTS(xmin, y));
				MemDc->LineTo(DOTS(xmax, y));
			}
		}

		// установка прозрачного фона текста
		MemDc->SetBkMode(TRANSPARENT);
		// установка шрифта
		CFont font;
		font.CreateFontW(18.5, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
		MemDc->SetTextColor(RGB(155, 155, 155));
		MemDc->SelectObject(&font);

		//подпись осей
		MemDc->TextOutW(DOTS(-0.07 * xmax, ymax), _T("φ(x)")); //Y
		MemDc->TextOutW(DOTS(xmax - xmax * 0.03, 0.1 * ymax), _T("x")); //X

		//по Y с шагом 5
		for (double i = 0; i <= ymax; i += ymax / 4)
		{
			CString str;
			if (i != 0)
			{
				str.Format(_T("%.1f"), i);
				MemDc->TextOutW(DOTS(0.01 * xmax, i), str);
			}
		}
		for (double i = 0; i >= ymin; i -= ymax / 4)
		{
			CString str;
			if (i != 0)
			{
				str.Format(_T("%.1f"), i);
				MemDc->TextOutW(DOTS(0.01 * xmax, i), str);
			}
		}
		//по X с шагом 0.5
		for (double j = xmin; j <= xmax; j += xmax / 6)
		{
			CString str;
			if (j == 0.0)
			{
				str.Format(_T("%.0f"), j);
				MemDc->TextOutW(DOTS(j + xmax / 55, -0.01 * ymax), str);
			}
			else
			{
				str.Format(_T("%.1f"), j);
				MemDc->TextOutW(DOTS(j - xmax / 50, -0.01 * ymax), str);
			}
		}

		// отрисовка
		/*MemDc->SelectObject(graphpen);
		MemDc->MoveTo(DOTS(0, Mass[0]));
		for (int i = 0; i < AbsMax; i++)
		{
			MemDc->LineTo(DOTS(i, Mass[i]));
		}*/

		// вывод на экран
		WinDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
		delete MemDc;
	}
	if (m_phaseFunc.GetCheck() == BST_CHECKED)
	{
		Mashtab(Mass, AbsMax, &ymin, &ymax);
		double from = -5;

		ymax = -0.2 * ymin;
		ymin = 1.2 * ymin;
		xmax = 30;
		xmin = from;

		// создание контекста устройства
		CBitmap bmp;
		CDC* MemDc;
		MemDc = new CDC;
		MemDc->CreateCompatibleDC(WinDc);

		double window_signal_width = WinPic.Width();
		double window_signal_height = WinPic.Height();
		xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
		yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

		bmp.CreateCompatibleBitmap(WinDc, window_signal_width, window_signal_height);
		CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
		// заливка фона графика белым цветом
		MemDc->FillSolidRect(WinPic, RGB(0, 0, 0));
		// отрисовка сетки координат
		MemDc->SelectObject(&axes_pen);

		//создаём Ось Y
		MemDc->MoveTo(DOTS(0, ymax));
		MemDc->LineTo(DOTS(0, ymin));
		//создаём Ось Х
		MemDc->MoveTo(DOTS(xmin, 0));
		MemDc->LineTo(DOTS(xmax, 0));

		MemDc->SelectObject(&grid_pen);
		//отрисовка сетки по y
		for (double x = xmin; x <= xmax; x += xmax / 6)
		{
			if (x != 0) {
				MemDc->MoveTo(DOTS(x, ymax));
				MemDc->LineTo(DOTS(x, ymin));
			}
		}
		//отрисовка сетки по x
		for (double y = 0; y < ymax; y += -ymin / 6)
		{
			if (y != 0) {
				MemDc->MoveTo(DOTS(xmin, y));
				MemDc->LineTo(DOTS(xmax, y));
			}
		}
		for (double y = 0; y > ymin; y -= -ymin / 6)
		{
			if (y != 0) {
				MemDc->MoveTo(DOTS(xmin, y));
				MemDc->LineTo(DOTS(xmax, y));
			}
		}

		// установка прозрачного фона текста
		MemDc->SetBkMode(TRANSPARENT);
		// установка шрифта
		CFont font;
		font.CreateFontW(18.5, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
		MemDc->SetTextColor(RGB(155, 155, 155));
		MemDc->SelectObject(&font);

		//подпись осей
		MemDc->TextOutW(DOTS(-0.05 * xmax, ymax), _T("φ(x)")); //Y
		MemDc->TextOutW(DOTS(xmax - xmax * 0.025, 0.35 * ymax), _T("x")); //X

		//по Y с шагом 5
		for (double i = 0; i <= ymax; i += -ymin / 6)
		{
			CString str;
			if (i != 0)
			{
				str.Format(_T("%.1f"), i);
				MemDc->TextOutW(DOTS(0.01 * xmax, i), str);
			}
		}
		for (double i = 0; i > ymin; i -= -ymin / 6)
		{
			CString str;
			if (i != 0)
			{
				str.Format(_T("%.1f"), i);
				MemDc->TextOutW(DOTS(0.01 * xmax, i), str);
			}
		}
		//по X с шагом 0.5
		for (double j = xmin; j <= xmax; j += xmax / 6)
		{
			CString str;
			if (j == 0.0)
			{
				str.Format(_T("%.0f"), j);
				MemDc->TextOutW(DOTS(j + xmax / 55, -0.01 * ymax), str);
			}
			else
			{
				str.Format(_T("%.1f"), j);
				MemDc->TextOutW(DOTS(j - xmax / 50, -0.01 * ymax), str);
			}
		}

		for (int k = 0; k < -(int)ymin; k++)
		{
			MemDc->SelectObject(&k_line);
			double y = -((2 * k + 1) * Pi) / 2;
			MemDc->MoveTo(DOTS(xmin, y));

			CString str;
			str.Format(_T("k=%i"), k);
			MemDc->TextOutW(DOTS(xmin, y), str);
			MemDc->LineTo(DOTS(xmax, y));
		}

		// отрисовка
		MemDc->SelectObject(graphpen);
		MemDc->MoveTo(DOTS(from, Mass[0]));
		for (int i = 0; i < AbsMax; i++)
		{
			MemDc->LineTo(DOTS(from + i, Mass[i]));
		}

		// вывод на экран
		WinDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
		delete MemDc;
	}
	if (m_waveFunc.GetCheck() == BST_CHECKED)
	{
		Mashtab(Mass, AbsMax, &ymin, &ymax);

		ymax = 1.1 * ymax;
		ymin = -1.1 * ymax;
		xmax = a * 1.2;
		xmin = -a * 1.2;

		// создание контекста устройства
		CBitmap bmp;
		CDC* MemDc;
		MemDc = new CDC;
		MemDc->CreateCompatibleDC(WinDc);

		double window_signal_width = WinPic.Width();
		double window_signal_height = WinPic.Height();
		xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
		yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

		bmp.CreateCompatibleBitmap(WinDc, window_signal_width, window_signal_height);
		CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
		// заливка фона графика белым цветом
		MemDc->FillSolidRect(WinPic, RGB(0, 0, 0));
		// отрисовка сетки координат
		MemDc->SelectObject(&axes_pen);

		//создаём Ось Y
		MemDc->MoveTo(DOTS(0, ymax));
		MemDc->LineTo(DOTS(0, ymin));
		//создаём Ось Х
		MemDc->MoveTo(DOTS(xmin, 0));
		MemDc->LineTo(DOTS(xmax, 0));

		//создаем бесконечно высокие стенки
		MemDc->SelectObject(&wall);
		MemDc->MoveTo(DOTS(-a, ymax));
		MemDc->LineTo(DOTS(-a, ymin));

		MemDc->MoveTo(DOTS(a, ymax));
		MemDc->LineTo(DOTS(a, ymin));

		MemDc->SelectObject(&grid_pen);
		//отрисовка сетки по y
		for (double x = xmin; x <= xmax; x += xmax / 6)
		{
			if (x != 0) {
				MemDc->MoveTo(DOTS(x, ymax));
				MemDc->LineTo(DOTS(x, ymin));
			}
		}
		//отрисовка сетки по x
		for (double y = 0; y < ymax; y += ymax / 4)
		{
			if (y != 0) {
				MemDc->MoveTo(DOTS(xmin, y));
				MemDc->LineTo(DOTS(xmax, y));
			}
		}
		for (double y = 0; y > ymin; y -= ymax / 4)
		{
			if (y != 0) {
				MemDc->MoveTo(DOTS(xmin, y));
				MemDc->LineTo(DOTS(xmax, y));
			}
		}

		// установка прозрачного фона текста
		MemDc->SetBkMode(TRANSPARENT);
		// установка шрифта
		CFont font;
		font.CreateFontW(18.5, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
		MemDc->SetTextColor(RGB(155, 155, 155));
		MemDc->SelectObject(&font);

		//подпись осей
		MemDc->TextOutW(DOTS(-0.07 * xmax, ymax), _T("φ(x)")); //Y
		MemDc->TextOutW(DOTS(xmax - xmax * 0.03, 0.1 * ymax), _T("x")); //X

		//по Y с шагом 5
		for (double i = 0; i <= ymax; i += ymax / 4)
		{
			CString str;
			if (i != 0)
			{
				str.Format(_T("%.1f"), i);
				MemDc->TextOutW(DOTS(0.01 * xmax, i), str);
			}
		}
		for (double i = 0; i >= ymin; i -= ymax / 4)
		{
			CString str;
			if (i != 0)
			{
				str.Format(_T("%.1f"), i);
				MemDc->TextOutW(DOTS(0.01 * xmax, i), str);
			}
		}
		//по X с шагом 0.5
		for (double j = xmin; j <= xmax; j += xmax / 6)
		{
			CString str;
			if (j == 0.0)
			{
				str.Format(_T("%.0f"), j);
				MemDc->TextOutW(DOTS(j + xmax / 55, -0.01 * ymax), str);
			}
			else
			{
				str.Format(_T("%.1f"), j);
				MemDc->TextOutW(DOTS(j - xmax / 50, -0.01 * ymax), str);
			}
		}

		// отрисовка
		MemDc->SelectObject(graphpen);
		MemDc->MoveTo(DOTS(-a, Mass[0]));
		for (double i = 0; i <= 2 * a; i += timestep)
		{
			MemDc->LineTo(DOTS(-a + i, Mass[(int)(i / timestep)]));
		}

		// вывод на экран
		WinDc->BitBlt(0, 0, window_signal_width, window_signal_height, MemDc, 0, 0, SRCCOPY);
		delete MemDc;
	}
}

double CEigenValuesDlg::dfi(double fi, double e)	// fishechka
{
	return -e * cos(fi) * cos(fi) - sin(fi) * sin(fi);
}

double CEigenValuesDlg::dr(double r, double fik, double ek)		// erochka
{
	return r * (1 - ek) * cos(fik) * sin(fik);
}

double CEigenValuesDlg::RungeKuttaMethodFi(double* mas)		// runge for fi
{
	double fi = mas[0];
	double e = mas[1];
	double dtt = mas[2];

	double k1x = dfi(fi, e);
	double k2x = dfi(fi + k1x * dtt / 2, e);
	double k3x = dfi(fi + k2x * dtt / 2, e);
	double k4x = dfi(fi + k3x * dtt, e);
	mas[0] = fi + (dtt / 6) * (k1x + 2 * k2x + 2 * k3x + k4x);
	return *mas;
}

double CEigenValuesDlg::RungeKuttaMethodR(double* mas)		// same runge for r
{
	double r = mas[0];
	double e = mas[1];
	double fi = mas[2];
	double dtt = mas[3];

	double k1x = dr(r, fi, e);
	double k2x = dr(r + k1x * dtt / 2, fi, e);
	double k3x = dr(r + k2x * dtt / 2, fi, e);
	double k4x = dr(r + k3x * dtt, fi, e);
	mas[0] = r + (dtt / 6) * (k1x + 2 * k2x + 2 * k3x + k4x);
	return *mas;
}

double CEigenValuesDlg::fiR(double E)		// 
{
	UpdateData(true);
	vector<double> fi;
	double minusR = Pi / 2;
	fi.push_back(minusR);

	for (double i = -a; i <= a; i += timestep )
	{
		double mas[3] = { minusR, E, espstep };
		RungeKuttaMethodFi(mas);
		minusR = mas[0];
		fi.push_back(minusR);
	}
	return fi.back();
}

void CEigenValuesDlg::r(double E)
{
	UpdateData(true);
	fi_mas.clear();
	psi.clear();
	double minusR = 10;
	double minusRfi = Pi / 2;
	//psi.push_back(minusR);
	//fi_mas.push_back(minusRfi);

	for (double i = -a; i <= a; i += timestep)
	{
		double mas_r[4] = { minusR, E, minusRfi, espstep };
		double mas_fi[3] = { minusRfi, E, espstep };
		RungeKuttaMethodR(mas_r);
		RungeKuttaMethodFi(mas_fi);
		minusR = mas_r[0];
		minusRfi = mas_fi[0];
		psi.push_back(minusR);
		fi_mas.push_back(minusRfi);
	}
}

double CEigenValuesDlg::MPDsolve(double a, double b)
{
	double x = 0;
	while (fabs(a - b) > MPDesp)
	{
		double c = (a + b) / 2;
		double d = (fiR(a) + (2 * k + 1) * Pi / 2) * (fiR(c) + (2 * k + 1) * Pi / 2);
		if (d > 0) a = c;
		else b = c;
		x = (a + b) / 2.;
	}
	return x;
}

void CEigenValuesDlg::OnBnClickedButtonGraph()		// расчет всего и вся
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(true);
	fir_vec.clear();
	// считаем вид фазовой функции
	double from = -5;
	double to = 30;
	for (double i = from; i <= to; i += 1)
	{
		fir_vec.push_back(fiR(i));
	}

	// считаем Ek
	epsilonK = MPDsolve(-1000, 1000);
	UpdateData(false);

	// считаем Фи_k(z)
	fik = fiR(epsilonK);

	// считаем радиальную часть
	r(epsilonK);
}


void CEigenValuesDlg::OnBnClickedButtonClose()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnCancel();
}


void CEigenValuesDlg::OnBnClickedRadioPotPit()
{
	// TODO: добавьте свой код обработчика уведомлений
	m_potentialPit.SetCheck(BST_CHECKED);
	m_phaseFunc.SetCheck(BST_UNCHECKED);
	m_waveFunc.SetCheck(BST_UNCHECKED);

	double mass[1] = { 0 };
	DrawGraphic(mass, PicDc, Pic, &graph_pen, 1);
}


void CEigenValuesDlg::OnBnClickedRadioPhaseFunc()
{
	// TODO: добавьте свой код обработчика уведомлений
	m_phaseFunc.SetCheck(BST_CHECKED);
	m_potentialPit.SetCheck(BST_UNCHECKED);
	m_waveFunc.SetCheck(BST_UNCHECKED);

	double size = fir_vec.size();
	double* fir_mass = new double[size];
	for (int i = 0; i < size; i++)
	{
		fir_mass[i] = 0;
		fir_mass[i] = fir_vec[i];
	}
	DrawGraphic(fir_mass, PicDc, Pic, &graph_pen, size);
	delete[] fir_mass;
}


void CEigenValuesDlg::OnBnClickedRadioWaveFunc2()
{
	// TODO: добавьте свой код обработчика уведомлений
	m_waveFunc.SetCheck(BST_CHECKED);
	m_potentialPit.SetCheck(BST_UNCHECKED);
	m_phaseFunc.SetCheck(BST_UNCHECKED);

	double size_fi = fi_mas.size();
	double size_psi = psi.size();
	if (size_fi == size_psi)
	{
		double* psi_mass = new double[size_psi];
		for (int i = 0; i < size_psi; i++)
		{
			psi_mass[i] = 0;
			psi_mass[i] = psi[i] * cos(fi_mas[i]);
		}
		DrawGraphic(psi_mass, PicDc, Pic, &graph_pen, size_psi);
		delete[] psi_mass;
	}
}


void CEigenValuesDlg::OnEnChangeEditKLine()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	UpdateData(true);
	OnBnClickedButtonGraph();
}


void CEigenValuesDlg::OnBnClickedButtonInfo()
{
	// TODO: добавьте свой код обработчика уведомлений
	MessageBox(L"     Привет,\n\nдля получения результатов необходимо нажать кнопку \"Рассчитать\".\
 После того, как решение просчитано, нажимая на соответствующую радио-кнопку,\
 можно нарисовать выбранный вид графика!\n\n     При смене уровня энергии (k)\
 пересчёт данных происходит автоматически, поэтому вы сразу можете выбрать нужный\
 radio-button!\n\n\t\t\t\t\tУдачи!",
L"Руководство по эксплуатации", MB_OK | MB_ICONINFORMATION);
}