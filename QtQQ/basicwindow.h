#pragma once

#include <QDialog>
#include"titlebar.h"

class BasicWindow : public QDialog
{
	Q_OBJECT

public:
	BasicWindow(QWidget *parent=nullptr);
	virtual ~BasicWindow();
public:
	void loadStyleSheet(const QString& sheetName);
	QPixmap getRoundImage(const QPixmap& src, QPixmap &mask, QSize masksize = QSize(0, 0));

private:
	void initBackGroundColor();
protected:
	void paintEvent(QPaintEvent*);

	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

	void initTitleBar(ButtonType buttontype = MIN_BUTTON);
	void setTitleBarTitle(const QString& title, const QString &icon = "");

	
public slots:
	void onShowClose(bool);
	void onShowMin(bool);
	void onShowHide(bool);
	void onShowNormal(bool);
	void onShowQuit(bool);

	void onSinnalSkinChanged(const QColor& color);

	void onButtonMinClicked();
	void onButtonRestoreClicked();
	void onButtonMaxClicked();
	void onButtonCloseClicked();

protected:
	QPoint m_mousePoint;	//���λ��
	bool m_mousePress;	//����ǲ�����
	QColor m_colorBackGround;
	QString m_styleName;	//��ʽ�ļ���
	TitleBar* _titleBar;	//������
};
