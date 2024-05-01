#include "lineform.h"
#include "ui_lineform.h"
#include "overaid.h"
#include "ui_overaid.h"

lineForm::lineForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lineForm)
{
    ui->setupUi(this);

    //-----
    ui->doubleSpinBoxAdd_fixe->setValue(0);
    ui->lineEditAdd_Operation->clear();
    ui->label_operationResult->clear();
    ui->lineEditAdd_description->clear();

    ui->radioButtonFixe->setChecked(true);
    on_radioButtonFixe_clicked();

    ui->label_idCat2->hide();
    ui->label_deviseCompte->hide();
    ui->doubleSpinBoxAdd_deviseCompte->hide();
}

lineForm::~lineForm()
{
    delete ui;
}

void lineForm::on_comboBoxAdd_Cat_currentTextChanged()
{
    actu_sousCategorie();
}

void lineForm::on_comboBoxAdd_Cat2_currentTextChanged()
{
    ui->label_idCat2->setText(QString::number(id_sousCategories.at(ui->comboBoxAdd_Cat2->currentIndex())));
    actu_desc_placeholder();
}

void lineForm::on_radioButtonFixe_clicked()
{
    ui->doubleSpinBoxAdd_fixe->setValue(ui->label_operationResult->text().split(" ").at(0).toDouble());
    ui->doubleSpinBoxAdd_fixe->setEnabled(true);
    ui->lineEditAdd_Operation->setEnabled(false);
    ui->lineEditAdd_Operation->clear();
}

void lineForm::on_radioButtonOperation_clicked()
{
    if(ui->doubleSpinBoxAdd_fixe->value() != 0) ui->lineEditAdd_Operation->setText(QString::number(ui->doubleSpinBoxAdd_fixe->value(),'f',2).replace(".",","));
    ui->doubleSpinBoxAdd_fixe->setEnabled(false);
    ui->doubleSpinBoxAdd_fixe->setValue(0);
    ui->lineEditAdd_Operation->setEnabled(true);
}

void lineForm::on_lineEditAdd_Operation_textChanged()
{
    if(ui->lineEditAdd_Operation->text().contains("%"))
    {
        int cursor = ui->lineEditAdd_Operation->cursorPosition();
        ui->lineEditAdd_Operation->setText(ui->lineEditAdd_Operation->text().replace("%","/100"));
        ui->lineEditAdd_Operation->setCursorPosition(cursor+3);
    }

    int cursor = ui->lineEditAdd_Operation->cursorPosition();
    int oldNbCar = ui->lineEditAdd_Operation->text().size();
    ui->lineEditAdd_Operation->setText(ui->lineEditAdd_Operation->text().remove(QRegularExpression("[^0-9\\+\\,\\(\\)\\*\\.\\/\\-]")));
    int newNbCar = ui->lineEditAdd_Operation->text().size();
    if(newNbCar < oldNbCar) ui->lineEditAdd_Operation->setCursorPosition(cursor-1);
    else ui->lineEditAdd_Operation->setCursorPosition(cursor);

    double result = 0;
    if(!ui->lineEditAdd_Operation->text().endsWith(".") && !ui->lineEditAdd_Operation->text().endsWith(","))
    {
        QString exp = ui->lineEditAdd_Operation->text();
        QByteArray expBA = exp.toUtf8();
        expressionToParse = expBA.constData();
        result = expression();
        if(result == INFINITY) result = 0;
    }

    ui->label_operationResult->setText(QString::number(result, 'f', 2)+ui->doubleSpinBoxAdd_fixe->suffix());
    if(ui->lineEditAdd_Operation->text().count() == 0) ui->label_operationResult->clear();

    emit OverAid_amountChanged();
}

void lineForm::on_doubleSpinBoxAdd_fixe_valueChanged(double arg1)
{
    emit OverAid_amountChanged();
}

void lineForm::on_lineEditAdd_description_textChanged()
{
    if(ui->lineEditAdd_description->text().contains(";"))
    {
        int cursor = ui->lineEditAdd_description->cursorPosition();
        ui->lineEditAdd_description->setText(ui->lineEditAdd_description->text().remove(";"));
        ui->lineEditAdd_description->setCursorPosition(cursor-1);
    }

    foreach(QObject *object, this->parent()->children())
        if(qobject_cast<lineForm*>(object))
            if(this != qobject_cast<lineForm*>(object)) qobject_cast<lineForm*>(object)->actu_desc();
}

void lineForm::actu_categorie()
{
    disconnect(ui->comboBoxAdd_Cat, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBoxAdd_Cat_currentTextChanged()));

    id_categories.clear();
    ui->comboBoxAdd_Cat->clear();
    QSqlQuery categorie("SELECT * FROM Catégories WHERE type='0' AND id_compte='"+QString::number(id_compte)+"' ORDER BY nom");
    while(categorie.next())
    {
        id_categories.append(categorie.value("id_cat").toInt());
        ui->comboBoxAdd_Cat->addItem(categorie.value("nom").toString());
    }

    connect(ui->comboBoxAdd_Cat, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBoxAdd_Cat_currentTextChanged()));
    on_comboBoxAdd_Cat_currentTextChanged();
}

void lineForm::actu_sousCategorie()
{
    disconnect(ui->comboBoxAdd_Cat2, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBoxAdd_Cat2_currentTextChanged()));

    id_sousCategories.clear();
    ui->comboBoxAdd_Cat2->clear();
    id_sousCategories.append(0);
    ui->comboBoxAdd_Cat2->addItem("");
    if(ui->comboBoxAdd_Cat->count() > 0)
    {
        QSqlQuery sousCategorie("SELECT * FROM Catégories WHERE type='1' AND id_compte='"+QString::number(id_compte)+"' AND cat0='"+QString::number(id_categories.at(ui->comboBoxAdd_Cat->currentIndex()))+"' ORDER BY nom");
        while(sousCategorie.next())
        {
            id_sousCategories.append(sousCategorie.value("id_cat").toInt());
            ui->comboBoxAdd_Cat2->addItem(sousCategorie.value("nom").toString());
        }
    }

    connect(ui->comboBoxAdd_Cat2, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBoxAdd_Cat2_currentTextChanged()));
    on_comboBoxAdd_Cat2_currentTextChanged();
}

void lineForm::set_sousCategorie(QString *id)
{
    ui->comboBoxAdd_Cat2->setCurrentIndex(id_sousCategories.indexOf(id->toInt()));
}

void lineForm::actu_desc()
{
    //Autocompletion Description
    QStringList descriptionList;
    QSqlQuery desc("SELECT DISTINCT description FROM Transactions WHERE id_compte='"+QString::number(id_compte)+"' ORDER BY description ASC");
    while(desc.next())
    {
        if(desc.value("description").toString().contains(";"))
        {
            for(int i = 0; i <= desc.value("description").toString().count(";"); i++)
                if(!descriptionList.contains(desc.value("description").toString().split(";").at(i))) descriptionList.append(desc.value("description").toString().split(";").at(i));
        }
        else if(!descriptionList.contains(desc.value("description").toString())) descriptionList.append(desc.value("description").toString());
    }

    foreach(QObject *object, this->parent()->children())
    {
        if(qobject_cast<lineForm*>(object))
        {
            lineForm *line = qobject_cast<lineForm*>(object);
            if(!descriptionList.contains(line->ui->lineEditAdd_description->text()) && line->ui->lineEditAdd_description->text() != "") descriptionList.append(line->ui->lineEditAdd_description->text());
        }
    }

    QCompleter *com(new QCompleter(descriptionList, this));
    com->setCaseSensitivity(Qt::CaseInsensitive);
    com->setFilterMode(Qt::MatchContains);
    ui->lineEditAdd_description->setCompleter(com);
}

void lineForm::actu_desc_placeholder()
{
    //Placeholder Description
    QString cat, cat2;

    if(id_categories.count() > 0)
    {
        if(id_categories.at(ui->comboBoxAdd_Cat->currentIndex()) == 0) cat = "";
        else cat = QString::number(id_categories.at(ui->comboBoxAdd_Cat->currentIndex()));
    }

    if(id_sousCategories.count() > 0)
    {
        if(id_sousCategories.at(ui->comboBoxAdd_Cat2->currentIndex()) == 0) cat2 = "";
        else cat2 = QString::number(id_sousCategories.at(ui->comboBoxAdd_Cat2->currentIndex()));
    }

    QSqlQuery desc("SELECT description, COUNT(description) FROM Transactions WHERE categorie='\""+cat+"\"' AND sous_categorie='\""+cat2+"\"' GROUP BY description ORDER BY COUNT(description) DESC");
    if(desc.next()) ui->lineEditAdd_description->setPlaceholderText(desc.value("description").toString());
    else ui->lineEditAdd_description->setPlaceholderText("Carrefour Drive");
}


//Calcul
char lineForm::peek()
{
    return *expressionToParse;
}

char lineForm::get()
{
    return *expressionToParse++;
}

double lineForm::number()
{
    double result = get() - '0';
    while (peek() >= '0' && peek() <= '9') result = 10*result + get() - '0';

    if(peek() == '.' || peek() == ',')
    {
        get();
        double dec = get() - '0';
        if (peek() >= '0' && peek() <= '9')
        {
            dec = 10*dec + get() - '0';
            result += dec / 100;
        }
        else result += dec / 10;
    }

    return result;
}

double lineForm::factor()
{
    if (peek() >= '0' && peek() <= '9')
        return number();
    else if (peek() == '(')
    {
        get(); // '('
        double result = expression();
        get(); // ')'
        return result;
    }
    else if (peek() == '-')
    {
        get();
        return -factor();
    }
    return 0;
}

double lineForm::term()
{
    double result = factor();
    while (peek() == '*' || peek() == '/')
        if (get() == '*')
            result *= factor();
        else
            result /= factor();
    return result;
}

double lineForm::expression()
{
    double result = term();
    while (peek() == '+' || peek() == '-')
        if (get() == '+')
            result += term();
        else
            result -= term();
    return result;
}
