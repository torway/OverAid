#ifndef CUSTOMMENU_H
#define CUSTOMMENU_H

#include "QtWidgets/qpushbutton.h"
#include <QWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QDebug>

class CustomMenu : public QMenu
{
    Q_OBJECT
public:
    explicit CustomMenu(QPushButton *button, QWidget *parent = nullptr);

    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;

public slots:
    void activateAll();
    void activateNone();

private:
    QPushButton* b;

signals:

};

#endif // CUSTOMMENU_H
