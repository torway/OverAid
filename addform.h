#ifndef ADDFORM_H
#define ADDFORM_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDateTime>
#include <QCompleter>

QT_BEGIN_NAMESPACE
namespace Ui { class AddForm; }
QT_END_NAMESPACE

class AddForm : public QWidget
{
    Q_OBJECT

public:
    explicit AddForm(QWidget *parent = nullptr);
    Ui::AddForm *ui;
    ~AddForm();

    void totalAmount();

private slots:
    void on_pushButtonAdd_clicked();

    void on_tabWidget_Line_tabCloseRequested(int index);
    void on_tabWidget_Line_currentChanged(int index);

signals:
    QString OverAid_id_compte();

private:
    QByteArray PDFInByteArray;
};

#endif // ADDFORM_H
