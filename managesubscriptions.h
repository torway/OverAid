#ifndef MANAGESUBSCRIPTIONS_H
#define MANAGESUBSCRIPTIONS_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <transform.h>

namespace Ui {
class ManageSubscriptions;
}

class ManageSubscriptions : public QWidget
{
    Q_OBJECT

public:
    explicit ManageSubscriptions(QWidget *parent = nullptr);
    Ui::ManageSubscriptions *ui;

    int id_compte = -1;
    QList<int> id_categories;

    ~ManageSubscriptions();

public slots:
    void actu();

    void modify_sub();
    void delete_sub();
    void duplicate();

private slots:
    void treeWidgetMenu(const QPoint &po);

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    TransForm *trans = new TransForm(nullptr, "Abonnement");
};

#endif // MANAGESUBSCRIPTIONS_H
