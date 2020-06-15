#include "TalkWindowShell.h"
#include"CommonUtils.h"
#include"EmotionWindow.h"
#include"TalkWindow.h"
#include"TalkWindowItem.h"
#include<qsqlquerymodel.h>
#include<QMessageBox>
#include<qfile.h>
#include<QSqlQuery>
#include"WindowManger.h"
#include"ReceiveFile.h"
#include"UserLogin.h"
#include"common.h"

extern QString gLoginEmployeeID;

QString gfileName;
QString gfileData;


TalkWindowShell::TalkWindowShell(QWidget *parent)
	: BasicWindow(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	initControl();
	
	QFile file("Resources/MainWindow/MsgHtml/msgtmpl.js");
	if (!file.size())   //�ļ���СΪ���򴴽��ļ�
	{
		QStringList employeeIDList;
		getEmployeesID(employeeIDList);
		if (!createJSFile(employeeIDList))
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("js�ļ�д������ʧ��"));
		}
	}
	
}

TalkWindowShell::~TalkWindowShell()
{
	/*delete m_emotionWindow;
	m_emotionWindow = nullptr;*/
}

void TalkWindowShell::addTalkWindow(TalkWindow * talkWindow, TalkWindowItem * talkWindowItem, const QString & uid)
{
	ui.rightStackedWidget->addWidget(talkWindow);
	connect(m_emotionWindow, SIGNAL(signalEmotionWindowHide()),
		talkWindow, SLOT(onSetEmotionBtnStatus()));

	QListWidgetItem* aItem = new QListWidgetItem(ui.listWidget);
	m_talkwindowItemMap.insert(aItem, talkWindow);
	aItem->setSelected(true);

	//�ж��ǵ��Ļ���Ⱥ��
	QSqlQueryModel sqlDepModel;
	QString strSql = QString("SELECT picture FROM tab_department WHERE departmentID=%1").arg(uid);
	sqlDepModel.setQuery(strSql);
	int rows = sqlDepModel.rowCount();
	if (rows == 0)	//����
	{
		QString sql = QString("SELECT picture FROM tab_employees WHERE employeeID=%1").arg(uid);
		sqlDepModel.setQuery(sql);
	}
	QModelIndex index;
	index = sqlDepModel.index(0, 0);

	QImage img;
	img.load(sqlDepModel.data(index).toString());
	talkWindowItem->setHeadPixmap(QPixmap::fromImage(img));	//����ͷ��

	ui.listWidget->addItem(aItem);
	ui.listWidget->setItemWidget(aItem, talkWindowItem);

	onTalkWindowItemClicked(aItem);
	connect(talkWindowItem, &TalkWindowItem::signalCloseClocked,
		[talkWindowItem, talkWindow,aItem, this]() {
		m_talkwindowItemMap.remove(aItem);
		talkWindow->close();
		ui.listWidget->takeItem(ui.listWidget->row(aItem));
		delete talkWindowItem;
		ui.rightStackedWidget->removeWidget(talkWindow);
		if (ui.rightStackedWidget->count() < 1) close();

	});
	
}

void TalkWindowShell::setCurrentWidget(QWidget * widget)
{
	ui.rightStackedWidget->setCurrentWidget(widget);
}

const QMap<QListWidgetItem*, QWidget*>& TalkWindowShell::getTalkWindowItemMap() const
{
	return m_talkwindowItemMap;
}

void TalkWindowShell::initControl()
{
	loadStyleSheet("TalkWindow");
	setWindowTitle(QString::fromLocal8Bit("EveryOne"));

	m_emotionWindow = new EmotionWindow;
	m_emotionWindow->hide();

	QList<int> leftWidgetSize;
	leftWidgetSize << 154 << width() - 154;
	ui.splitter->setSizes(leftWidgetSize);		//���������óߴ�

	ui.listWidget->setStyle(new CustomProxyStyle(this));

	connect(ui.listWidget, &QListWidget::itemClicked, this, &TalkWindowShell::onTalkWindowItemClicked);
	connect(m_emotionWindow, SIGNAL(signalEmotionItemClicked(int)), this, SLOT(onEmotionItemClicked(int)));
	connect(&gTcpSocket, &QTcpSocket::readyRead, this, &TalkWindowShell::onProcessTcpData);
}



void TalkWindowShell::getEmployeesID(QStringList& employeeIDList)
{
	QSqlQueryModel queryModel;
	queryModel.setQuery("SELECT employeeID FROM tab_employees WHERE `status`=1");
	//Ա������
	int employeesNum = queryModel.rowCount();
	QModelIndex index;
	for (int i = 0; i < employeesNum; i++)
	{
		index = queryModel.index(i, 0);
		employeeIDList << queryModel.data(index).toString();
	}
}
bool TalkWindowShell::createJSFile(QStringList & employeesList)
{
	//��ȡ�ļ�����
	QString strFileTxt("Resources/MainWindow/MsgHtml/msgtmpl.txt");
	QFile fileRead(strFileTxt);
	QString strFile;
	if (fileRead.open(QIODevice::ReadOnly))
	{
		strFile=fileRead.readAll();
		fileRead.close();
	}
	else
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("msgtmpl.txt��ȡ����ʧ��"));
		return false;
	}
	//�滻(extern0,append10�����Լ�����Ϣʹ��)
	QFile fileWrite("Resources/MainWindow/MsgHtml/msgtmpl.js");
	if(fileWrite.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		//���¿�ֵ
		QString strSourceInitNull = "var external = null;";
		//���³�ʼ��
		QString strSourceInit = "external = channel.objects.external;";
		//����newChannel
		QString strSourceNew =
			"new QWebChannel(qt.webChannelTransport,\
			function(channel) {\
			external = channel.objects.external;\
		}\
		); ";
		QString strSourceRecvHtml;
		QFile fileRecvHtml("Resources/MainWindow/MsgHtml/recvHtml.txt");
		if (fileRecvHtml.open(QIODevice::ReadOnly))
		{
			strSourceRecvHtml = fileRecvHtml.readAll();
			fileRecvHtml.close();
		}
		else
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("recvHtml.txt��ȡʧ��"));
			return false;
		}

		//�����滻��Ľű�
		QString strReplaceInitNull;
		QString strReplaceInit;
		QString strReplaceNew;
		QString strReplaceRecvHtml;
		
		for (int i = 0; i < employeesList.length(); i++)
		{
			// �༭ �滻��Ŀ�ֵ
			QString strInitNull = strSourceInitNull;
			strInitNull.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInitNull += strInitNull;
			strReplaceInitNull += "\n";

			//�༭�滻��ĳ�ʼֵ
			QString strInit = strSourceInit;
			strInit.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceInit += strInit;
			strReplaceInit += "\n";

			//�༭�滻��� newWebChannel
			QString strNew = strSourceNew;
			strNew.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strReplaceNew += strNew;
			strReplaceNew += "\n";

			//�༭�滻���recvHtml
			QString strRecvHtml = strSourceRecvHtml;
			strRecvHtml.replace("external", QString("external_%1").arg(employeesList.at(i)));
			strRecvHtml.replace("recvHtml", QString("recvHtml_%1").arg(employeesList.at(i)));
			strReplaceRecvHtml += strRecvHtml;
			strReplaceRecvHtml += "\n";
		}
		strFile.replace(strSourceInitNull, strReplaceInitNull);
		strFile.replace(strSourceInit, strReplaceInit);
		strFile.replace(strSourceNew, strReplaceNew);
		strFile.replace(strSourceRecvHtml, strReplaceRecvHtml);

		QTextStream stream(&fileWrite);
		stream << strFile;
		return true;
	}
	else
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("��ʾ"),
			QString::fromLocal8Bit("дmsgtmpl.jsʧ��"));
		return false;
	}
}

void TalkWindowShell::handleReceivedMsg(int senderEmployeeID, int msgType, QString strMsg)
{
	QMsgTextEdit msgTextEdit;
	msgTextEdit.setText(strMsg);

	if (msgType == 1) { //�ı�
		msgTextEdit.toHtml();
	}
	else if (msgType == 0) {//������Ϣ����
		const int emotionWidth = 3;
		int emotionNum = strMsg.length() / emotionWidth;

		for (int i = 0; i < emotionNum; i++) {
			msgTextEdit.addEmotionUrl(strMsg.mid(i * 3, emotionWidth).toInt());
		}
	}
	QString html = msgTextEdit.document()->toHtml();

	//�ı�HTML���û���������������
	if (!html.contains(".png") && !html.contains("</span>"))
	{
		QString fontHtml;
		QFile file(":/Resources/MainWindow/MsgHtml/msgFont.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			fontHtml = file.readAll();
			fontHtml.replace("%1", strMsg);
			file.close();
		}
		else
		{
			QMessageBox::information(this, QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("msgFont.txt ������"));
			return;
		}
		if (!html.contains(fontHtml))
		{
			html.replace(strMsg, fontHtml);
		}
	}

	TalkWindow* talkWindow =dynamic_cast<TalkWindow*> (ui.rightStackedWidget->currentWidget());
	talkWindow->ui.msgWidget->appendMsg(html, QString::number(senderEmployeeID));
}

/*������Ϣ�ۺ���*/
void TalkWindowShell::updateSendTcpMsg(QString & strData, int & msgType, QString fileName)
{
	/*�ı����ݰ���ʽ:
	Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) + ��Ϣ���� +���ݳ���(5λ��)+����
	��������ݸ�ʽ:
	Ⱥ�ı�־+ ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) +��Ϣ���� +�������+images+����
	*/
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.rightStackedWidget->currentWidget());
	QString talkId = curTalkWindow->getTalkId();

	QString strGroupFlag;
	QString strSend;
	if (talkId.length() == 4)
	{
		strGroupFlag = "1";
	}
	else
	{
		strGroupFlag = "0";
	}
	int nstrDataL = strData.length();
	int dataLength = QString::number(nstrDataL).length();
	//const int sourceDataLength = dataLength;
	QString strDataLength;
	if (msgType == 1)	//�����ı���Ϣ
	{
		//�ı���Ϣ���ȹ̶�Ϊ5λ��
		if (dataLength == 1)
		{
			strDataLength = "0000" + QString::number(nstrDataL);
		}
		else if(dataLength == 2)
		{
			strDataLength = "000" + QString::number(nstrDataL);
		}
		else if (dataLength == 3)
		{
			strDataLength = "00" + QString::number(nstrDataL);
		}
		else if (dataLength == 4)
		{
			strDataLength = "0" + QString::number(nstrDataL);
		}
		else if (dataLength == 5)
		{
			strDataLength = QString::number(nstrDataL);
		}
		else
		{
			QMessageBox::information(this,
				QString::fromLocal8Bit("��ʾ"),
				QString::fromLocal8Bit("���ݳ��Ȳ�����!"));
		}
		//Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) + ��Ϣ���� +���ݳ���+����
		strSend = strGroupFlag + gLoginEmployeeID + talkId+"1" + strDataLength + strData;

	}
	else if (msgType == 0) //������Ϣ
	{
		//Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) + ��Ϣ���� + ������� + images + ����
		strSend = strGroupFlag + gLoginEmployeeID + talkId +"0"+ strData;
	}
	else if (msgType == 2) //�ļ�
	{
		//Ⱥ�ı�־+����ϢԱ��QQ��+����ϢԱ��QQ��+��Ϣ����+�ļ�����
		//+bytes+�ļ���+data_begin+�ļ�����
		QByteArray bt = strData.toUtf8();
		QString strLength = QString::number(bt.length());
		strSend = strGroupFlag + gLoginEmployeeID + talkId
			+ "2" + "bytes" + fileName + "data_begin" + strData;
	}
	QByteArray dataBt;
	dataBt.resize(strSend.length());
	dataBt = strSend.toUtf8();
	

	gTcpSocket.write(dataBt);
}
void TalkWindowShell::onEmotionBtnClicked(bool)
{
	m_emotionWindow->setVisible(!m_emotionWindow->isVisible());
	QPoint emotonPoint = this->mapToGlobal(QPoint(0, 0));	//����ǰ�ؼ������λ��װ��Ϊ
	emotonPoint.setX(emotonPoint.x() + 170);
	emotonPoint.setY(emotonPoint.y() + 220);
	m_emotionWindow->move(emotonPoint);
}

void TalkWindowShell::onTalkWindowItemClicked(QListWidgetItem * item)
{
	QWidget* talkwindowWidget = m_talkwindowItemMap.find(item).value();
	ui.rightStackedWidget->setCurrentWidget(talkwindowWidget);

}

void TalkWindowShell::onEmotionItemClicked(int emotionNum)
{
	TalkWindow* curTalkWindow = dynamic_cast<TalkWindow*>(ui.rightStackedWidget->currentWidget());
	if (curTalkWindow)
	{
		curTalkWindow->addEmotionImage(emotionNum);
	}
}

/*
	���ݰ��ĸ�ʽ:
	Ⱥ�ı�־:1 λ 0���� 1Ⱥ��
	��Ϣ����:1 λ 0���� 1�ı� 2�ļ�
	Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) + ��Ϣ����(1) +���ݳ���+����
	Ⱥ�ı�־ + ����ϢԱ��QQ�� + ����ϢԱ��QQ��(ȺQQ��) + ��Ϣ����(0) + ������� + images + ����
	Ⱥ�ı�־+����ϢԱ��QQ��+����ϢԱ��QQ��+��Ϣ����(2)+�ļ�����+bytes+�ļ���+data_begin+�ļ�����
*/


void TalkWindowShell::onProcessTcpData()
{

	const static int groupFlagWidth = 1;
	const static int groupWidth = 4;
	const static int employeeWidth = 5;
	const static int msgTypeWidth = 1;
	const static int msgLengthWidth = 5;
	const static int pictureWidth = 3;

	//��ȡUDP����
	QByteArray btData;
	btData = gTcpSocket.readAll();

	QString strData = btData.data();
	QString strWindowID;
	QString strSendEmployeeID, strReceieEmployeeeID;
	QString strMsg;	//����

	int msgLen;		//���ݳ���
	int msgType;	//��������

	strSendEmployeeID = strData.mid(groupFlagWidth, employeeWidth);
	if (strSendEmployeeID == gLoginEmployeeID) return;
		
	if (btData[0] == '1') //Ⱥ��
	{
		strWindowID = strData.mid(groupFlagWidth + employeeWidth, groupWidth);
		QChar cMsgType = btData[groupFlagWidth + employeeWidth + groupWidth];

		if (cMsgType == '1') {//�ı�
			msgType = 1;
			msgLen = strData.mid(groupFlagWidth + employeeWidth +
				groupWidth + msgTypeWidth ,msgLengthWidth).toInt();
			strMsg = strData.mid(groupFlagWidth + employeeWidth +
				groupWidth + msgTypeWidth + msgLengthWidth, msgLen);
		}
		else if (cMsgType == '0')	//ͼƬ
		{
			msgType = 0;
			int posImage = strData.indexOf("images");
			strMsg = strData.right(strData.length() - posImage - QString("iamges").length());
		}
		else if (cMsgType == '2')	//�ļ�
		{
			msgType = 2;
			//�ļ�����
			int bytesWidth = QString("bytes").length();
			int posBytes = strData.indexOf("bytes");
			int posData_begin = strData.indexOf("data_begin");

			QString fileName = strData.mid(posBytes + bytesWidth,
				posData_begin - bytesWidth - posBytes);
			gfileName = fileName;

			//�ļ�����
			int dataLengthWidth;
			int posData = posData_begin + QString("data_begin").length();
			strMsg = strData.mid(posData);
			gfileData = strMsg;

			//��ȡ����������
			QString sender;
			int employeeID = strSendEmployeeID.toInt();
			QSqlQuery querySenderName(QString("SELECT employee_name FROM tab_employees WHERE employeeID=%1").arg(employeeID));
			querySenderName.exec();
				
			if (querySenderName.next()) {
				sender = querySenderName.value(0).toString();
			}
			//�����ļ����� .....
			ReceiveFile* receiveFile = new ReceiveFile(this);
			connect(receiveFile, &ReceiveFile::refuseFile, [this]() {
				return;
			});
			QString msgLebel = QString::fromLocal8Bit("�յ�����") + sender +
				QString::fromLocal8Bit("�������ļ�.�Ƿ����?");
			receiveFile->setMsg(msgLebel);
			receiveFile->show();

		}

	}
	else if (btData[0] == '0') //����
	{
		strReceieEmployeeeID = strData.mid(groupFlagWidth + employeeWidth, employeeWidth);
		strWindowID = strSendEmployeeID;
			
		//���Ƿ����ҵ���Ϣ��������
		if (strReceieEmployeeeID != gLoginEmployeeID) return;

		QChar cMsgType = btData[groupFlagWidth + employeeWidth + employeeWidth];
		if (cMsgType == '1')//�ı���Ϣ
		{
			msgType = 1;
			msgLen = strData.mid(groupFlagWidth + employeeWidth+employeeWidth
			+msgTypeWidth,msgLengthWidth).toInt();

			strMsg = strData.mid(groupFlagWidth + employeeWidth + employeeWidth
				+ msgTypeWidth + msgLengthWidth, msgLen);

		}
		else if (cMsgType == '0')  //������Ϣ
		{
			msgType = 0;
			int posImags = strData.indexOf("images");
			int imagesWidth = QString("images").length();
			strMsg = strData.mid(posImags + imagesWidth);
		}
		else if (cMsgType == '2')
		{
				
			msgType = 2;
			//�ļ�����
			int bytesWidth = QString("bytes").length();
			int posBytes = strData.indexOf("bytes");
			int posData_begin = strData.indexOf("data_begin");
			//11000520022bytesexit.txtdata_beginAre you ok?
			QString fileName = strData.mid(posBytes + bytesWidth,
				posData_begin - bytesWidth - posBytes);
			gfileName = fileName;
			//�ļ�����
			int dataLengthWidth;
			int posData = posData_begin + QString("data_begin").length();
			strMsg = strData.mid(posData);
			gfileData = strMsg;

			//��ȡ����������
			QString sender;
			int employeeID = strSendEmployeeID.toInt();
			QSqlQuery querySenderName(QString("SELECT employee_name FROM tab_employees WHERE employeeID=%1").arg(employeeID));
			querySenderName.exec();

			if (querySenderName.next()) {
				sender = querySenderName.value(0).toString();
			}
			//�����ļ����� 
			ReceiveFile* receiveFile = new ReceiveFile(this);
			connect(receiveFile, &ReceiveFile::refuseFile, [this]() {
				return;
			});
			QString msgLebel = QString::fromLocal8Bit("�յ�����") + sender +
				QString::fromLocal8Bit("�������ļ�.�Ƿ����?");
			receiveFile->setMsg(msgLebel);
			receiveFile->show();
		}
	}

	//�����촰������Ϊ���
	QWidget* widget = WindowManger::getInstance()->findWindowName(strWindowID);
	if (widget)  //���촰�ڴ���
	{
		this->setCurrentWidget(widget);
		//ͬ������������촰��
		QListWidgetItem* item=m_talkwindowItemMap.key(widget);
		item->setSelected(true);

	}
	else //���촰��δ��
	{
		return;
	}
	//�ļ���Ϣ��������
	if (msgType != 2) {
		handleReceivedMsg(strSendEmployeeID.toInt(), msgType, strMsg);
	}

}

