#ifndef MANAGEACCOUNT_H
#define MANAGEACCOUNT_H

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
class ManageAccount;
}

class ManageAccount : public QWidget
{
    Q_OBJECT

public:
    explicit ManageAccount(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event);
    ~ManageAccount();

    void actu();

private slots:

    void on_pushButton_close_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_modify_clicked();

    void on_listWidget_currentRowChanged(int currentRow);

    void on_comboBox_devise_currentTextChanged(const QString &arg1);
    void on_comboBox_devise_2_currentTextChanged(const QString &arg1);

    void on_pushButton_defaultAccount_clicked();

private:
    Ui::ManageAccount *ui;

    QList<int> id_account;

signals :
    void actualiser();
    void actuSolde();
};

#endif // MANAGEACCOUNT_H
