#ifndef LINEFORM_H
#define LINEFORM_H

#include <QWidget>

namespace Ui {
class lineForm;
}

class lineForm : public QWidget
{
    Q_OBJECT

public:
    explicit lineForm(QWidget *parent = nullptr);
    Ui::lineForm *ui;

    int id_compte = -1;
    QList<int> id_categories;

    QList<int> id_sousCategories;

    const char * expressionToParse = "";
    double expression();

    void actu_categorie();
    void actu_sousCategorie();
    void actu_desc();
    void actu_desc_placeholder();

    ~lineForm();

public slots:
    void on_radioButtonFixe_clicked();
    void on_radioButtonOperation_clicked();

    void set_sousCategorie(QString *id);

private slots:
    //Calcul opération
    char peek();
    char get();
    double factor();
    double term();
    double number();

    void on_comboBoxAdd_Cat_currentTextChanged();
    void on_comboBoxAdd_Cat2_currentTextChanged();

    void on_lineEditAdd_Operation_textChanged();

    void on_lineEditAdd_description_textChanged();

    void on_doubleSpinBoxAdd_fixe_valueChanged(double arg1);

signals:
    void OverAid_amountChanged();

private:
};

#endif // LINEFORM_H
