#pragma once

#include <QLabel>
#include<qpropertyanimation.h>

class RootContatItem : public QLabel
{
	Q_OBJECT

	//��ͷ�Ƕ�
	Q_PROPERTY(int rotation READ rotation WRITE setRotation)
public:
	RootContatItem(bool hasArrow=true,QWidget *parent=nullptr);
	~RootContatItem();
public:
	void setText(const QString& title);
	void setExpanded(bool expand);

private:
	int rotation();
	void setRotation(int rotation);
protected:
	void paintEvent(QPaintEvent* event);

private:
	QPropertyAnimation* m_animation;
	QString m_titleText;	//�ı�
	int m_rotation;			//�Ƕ�
	bool m_hasArrow;      

};
