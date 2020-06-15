#pragma once

#include"basicwindow.h"
#include "ui_TalkWindowShell.h"
#include<qmap.h>
#include<QTcpSocket>

class TalkWindow;
class TalkWindowItem;
class QListWidgetItem;
class EmotionWindow;

enum GroupType
{
	COMPANY,
	PERSONGROUP,	//
	DEVELOPMENTGROUP,	//������
	MARKETGROUP,	//�г���
	PTOP		//��������
};
class  TalkWindowShell: public BasicWindow
{
	Q_OBJECT

public:
	TalkWindowShell(QWidget *parent = Q_NULLPTR);
	~TalkWindowShell();

public:
	//����µ����촰��
	void addTalkWindow(TalkWindow* talkWindow, TalkWindowItem* talkWindowItem, const QString & uid/*,GroupType grouptype*/);
	//���õ�ǰ���촰��
	void setCurrentWidget(QWidget* widget);

	const QMap<QListWidgetItem*, QWidget*>& getTalkWindowItemMap() const;

private:
	void initControl();
	void getEmployeesID(QStringList& employeeIDList);	//��ȡ����Ա��QQ��

	bool createJSFile(QStringList& employeesList);
	void handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg);
public slots:
	void onEmotionBtnClicked(bool);
	//�ͻ��˷���Tcp����
	void updateSendTcpMsg(QString& strData, int &msgType, QString fileName = "");
private slots:
	//����б�����ִ�еĺ���
	void onTalkWindowItemClicked(QListWidgetItem* item);
	void onEmotionItemClicked(int emotionNum);
	void onProcessTcpData();	//�����յ�������
private:
	Ui::TalkWindowClass ui;
	QMap<QListWidgetItem*, QWidget*> m_talkwindowItemMap; //�򿪵����촰��
	EmotionWindow* m_emotionWindow;	

private:
};
