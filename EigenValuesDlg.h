
// EigenValuesDlg.h: файл заголовка
//

#pragma once
#include <vector>
#include "afxwin.h"


// Диалоговое окно CEigenValuesDlg
class CEigenValuesDlg : public CDialogEx
{
// Создание
public:
	CEigenValuesDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EIGENVALUES_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// ** Picture **
	CWnd* PicWnd;
	CDC* PicDc;
	CRect Pic;
	//\\//\\//\\

	// ** CPen **
	CPen axes_pen;
	CPen grid_pen;
	CPen graph_pen;
	CPen wall;
	CPen k_line;
	//\\//\\//\\

	// ** DOTS/Scale **
	double xp = 0, yp = 0,				//коэфициенты пересчета
		xmin = 0, xmax = 0,				//максисимальное и минимальное значение х 
		ymin = 0, ymax = 0;				//максисимальное и минимальное значение y
	//\\//\\//\\


	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:

	// ** Methods ** 
	void Mashtab(double arr[], int dim, double* mmin, double* mmax);
	double RungeKuttaMethodFi(double* mas);
	double RungeKuttaMethodR(double* mas);
	double dfi(double fi, double e);
	double dr(double r, double fik, double ek);
	double fiR(double E);
	void r(double E);
	double MPDsolve(double a, double b);
	void DrawGraphic(double*, CDC*, CRect, CPen*, double);
	//\\//\\//\\

	// ** Afx **
	afx_msg void OnBnClickedRadioPotPit();
	afx_msg void OnBnClickedRadioPhaseFunc();
	afx_msg void OnBnClickedRadioWaveFunc2();
	afx_msg void OnEnChangeEditKLine();
	afx_msg void OnBnClickedButtonGraph();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonInfo();
	//\\//\\//\\

	// ** Global Vectors **
	std::vector<double> fir_vec;
	std::vector<double> psi;
	std::vector<double> fi_mas;
	//\\//\\//\\

	// ** Interface`s variables **
	double timestep;
	double espstep;
	double MPDesp;
	double epsilonK;
	double a;
	int k;
	CButton m_potentialPit;
	CButton m_phaseFunc;
	CButton m_waveFunc;
	//\\//\\//\\

	// ** Global variables **
	double Pi = 3.1415926535897;
	double fik = 0;
	//\\//\\//\\

};
