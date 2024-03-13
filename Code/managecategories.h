#ifndef MANAGECATEGORIES_H
#define MANAGECATEGORIES_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDateTime>
#include <QCompleter>
#include <QListWidgetItem>
#include <QtCharts>

namespace Ui {
class ManageCategories;
}

class ManageCategories : public QWidget
{
    Q_OBJECT

public:
    explicit ManageCategories(QWidget *parent = nullptr);
    Ui::ManageCategories *ui;

    int id_compte = -1;

    void closeEvent(QCloseEvent *event);
    ~ManageCategories();

    void actu_categorie();
    void actu_sousCategorie();

public slots:
    void listWidget_itemDoubleClicked_finished(QString catType, QListWidget *listWidget, QListWidgetItem *item);

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_addCat_clicked();
    void on_pushButton_addCat2_clicked();
    void on_listWidget_cat_currentRowChanged();

    void on_pushButton_deleteCat_clicked();
    void on_pushButton_deleteCat2_clicked();

    void on_listWidget_subcat_itemDoubleClicked(QListWidgetItem *item);
    void on_listWidget_cat_itemDoubleClicked(QListWidgetItem *item);

private:
    QList<int> id_cats;
    QList<int> id_subcats;

signals :
    void actualiser();
};

#endif // MANAGECATEGORIES_H
