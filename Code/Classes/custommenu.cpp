#include "custommenu.h"
#include "QtWidgets"

CustomMenu::CustomMenu(QPushButton *button, QWidget *parent) : QMenu(parent), b(button)
{
    this->setStyleSheet("*{menu-scrollable:1;}");
}

void CustomMenu::showEvent(QShowEvent *event)
{
    QPoint buttonSize = b->mapToGlobal(QPoint(0,0));

    if(this->pos().y() == buttonSize.y()+b->height()-8)
        this->move(buttonSize.x(),buttonSize.y()+b->height()-8);
    else if(buttonSize.x()+b->width()+this->width() > QGuiApplication::primaryScreen()->geometry().width())
        this->move(buttonSize.x()-this->width(),this->pos().y());
    else
        this->move(buttonSize.x()+b->width(),this->pos().y());
}

void CustomMenu::mouseReleaseEvent(QMouseEvent *event)
{
    QAction *actionAtEvent = actionAt(event->pos());

    if(actionAtEvent)
    {
        actionAtEvent->trigger();
        event->ignore();
    }
    else QMenu::mouseReleaseEvent(event);
}

void CustomMenu::activateAll()
{
    foreach(QAction *action, this->actions())
        if(action->isCheckable()) action->setChecked(true);
}

void CustomMenu::activateNone()
{
    foreach(QAction *action, this->actions())
        if(action->isCheckable()) action->setChecked(false);
}
