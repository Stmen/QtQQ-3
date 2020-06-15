#pragma once

#include <QWidget>
#include<QLabel>
#include<QPushButton>
enum ButtonType
{
	MIN_BUTTON = 0,
	MIN_MAX_BUTTON,
	ONLY_CLOSE_BUTTON
};
class TitleBar : public QWidget
{
	Q_OBJECT

public:
	TitleBar(QWidget *parent=nullptr);
	~TitleBar();
	void setTitleIcon(const QString filePath);
	void setTitleContent(const QString &titleContent);
	void setTitleWidth(int width);
	void setButtonType(ButtonType buttonType);
	
	//���洰�����ǰ���ڵ�λ�ü���С
	void saveRestoreInfo(const QPoint& point, const QSize& size);
	void getResoreInfo(QPoint& point, QSize& size);

private:
	
	void paintEvent(QPaintEvent* event);
	//����¼�
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

	void initControl();
	void initConnections();
	void loadStyleSheet(const QString& sheetName);

signals:
	void signalButtonMinClicked();
	void signalButtonRestoreClicked();
	void signalButtonMaxClicked();
	void signalButtonCloseClicked();

private slots:
	void onButtonMinClicked();
	void onButtonRestoreClicked();
	void onButtonMaxClicked();
	void onButtonCloseClicked();
private:
	QLabel* m_pIcon;
	QLabel* m_pTitleContent;
	QPushButton* m_pButtonMin;
	QPushButton* m_pButtonMax;
	QPushButton* m_pButtonRestore;
	QPushButton* m_pButtonClose;

	//��󻯻�ԭ��ť����
	bool m_isPressed;
	QPoint m_startMovePos;
	QPoint m_restorePos;
	QSize m_restoreSize;

	QString m_titleContent;
	ButtonType m_buttonType;
};
