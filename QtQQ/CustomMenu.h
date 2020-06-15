#pragma once

#include <QMenu>
#include<QMap>

/*�Զ���˵�*/
class CustomMenu : public QMenu
{
	Q_OBJECT

public:
	CustomMenu(QWidget *parent=nullptr);
	~CustomMenu();

	void addCustomMenu(const QString& text, const QString& icon, const QString& name);
	QAction* getAction(const QString& text);
private:
	QMap<QString, QAction*> m_menuActionMap;
};
