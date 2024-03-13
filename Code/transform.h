#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class TransForm;
}

class TransForm : public QWidget
{
    Q_OBJECT

public:
    explicit TransForm(QWidget *parent = nullptr, QString command = nullptr);
    Ui::TransForm *ui;

    int id_compte = -1;
    QList<int> id_categories;

    void actu_devise();
    void actu_projet();

    ~TransForm();

public slots:
    void on_tabWidget_Line_currentChanged(int index);
    void on_tabWidget_Line_tabCloseRequested(int index);

    void on_pushButton_deletePDF_clicked();

    void on_pushButton_out_clicked();
    void on_pushButton_in_clicked();

    void on_comboBox_devise_currentTextChanged(const QString &arg1);

    void totalAmount();

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonAdd_PDF_clicked();

    void on_checkBox_inOut_stateChanged(int arg1);

private:
    QLocale locale;
    QString commande;

signals:
    void actu();
};

#endif // TRANSFORM_H
