#pragma once

#include <QWebEngineView>
#include<QDomNode>
/*
	������ʾ��ҳ����
*/

class MsgHtmlObj : public QObject
{
	Q_OBJECT
	//��̬����
	Q_PROPERTY(QString msgLHtmlTmpl MEMBER m_msgLHtmlTmpl NOTIFY signalMsgHtml)
	Q_PROPERTY(QString msgRHtmlTmpl MEMBER m_msgRHtmlTmpl NOTIFY signalMsgHtml)

public:
	MsgHtmlObj(QObject* parent,QString msgLPicPath="");

signals:
	void signalMsgHtml(const QString& html);
private:
	QString m_msgLHtmlTmpl;	//���˷�����Ϣ
	QString m_msgRHtmlTmpl;	//�ҷ�����Ϣ
	QString m_msgPicPath;	//ͼƬ·��
private:
	void initHtmlTmpl();	//��ʼ��������ҳ
	QString getMsgTmplHtml(const QString& code);
};

class MsgWebPage : public QWebEnginePage
{
public:
	MsgWebPage(QObject* parent = nullptr) :	QWebEnginePage(parent) {}
	

protected:
	bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
private:

};


class MsgWebView : public QWebEngineView
{
	Q_OBJECT

public:
	MsgWebView(QWidget *parent);
	~MsgWebView();

	void appendMsg(const QString& html,QString strObj="0");
private:
	QList<QStringList> parseHtml(const QString& html);
	//Qt ������dom �ڵ� ������ʹ��DOM ���в���
	QList<QStringList> parseDocNode(const QDomNode& node);

signals:
	void signalSendMsg(QString& strData, int &msgType, QString sFile = "");
private:
	MsgHtmlObj* m_msgHtmlObj;
	QWebChannel* m_channel;	//����ͨ��
};
