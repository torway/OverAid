#include "addform.h"
#include "ui_addform.h"
#include "lineform.h"
#include "ui_lineform.h"

AddForm::AddForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddForm)
{
    ui->setupUi(this);

    ui->dateEditAdd->setDate(QDate::currentDate());
}

AddForm::~AddForm()
{
    delete ui;
}

void AddForm::on_pushButtonAdd_clicked()
{
    //Ajout d'une transaction
    bool descriptionOk = true;
    for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
    {
        if(ui->tabWidget_Line->widget(i)->findChild<QLineEdit *>("lineEditAdd_description")->text().isEmpty()) descriptionOk = false;
    }

    if(!descriptionOk) QMessageBox::warning(this, "Erreur", "Une description n'est renseignée.", "Fermer");
    else
    {
        QString type;
        if(ui->comboBoxAdd_InOut->currentIndex() == 0) type = "Dépense";
        else type = "Entrée d'argent";

        QString moyen;
        if(ui->radioButtonCarte->isChecked()) moyen = "Carte bancaire";
        else if(ui->radioButtonEspece->isChecked()) moyen = "Éspèces";
        else if(ui->radioButtonCheque->isChecked()) moyen = "Chèque";
        else if(ui->radioButtonVirement->isChecked()) moyen = "Virement";
        else if(ui->radioButtonPrelevement->isChecked()) moyen = "Prélèvement";

        QString montant, cat2, cat, desc, ope;
        for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
        {
            if(i != 0)
            {
                montant += ";";
                cat2 += ";";
                cat += ";";
                desc += ";";
                ope += ";";
            }

            QWidget *tab = ui->tabWidget_Line->widget(i);
            if(tab->findChild<QRadioButton *>("radioButtonFixe")->isChecked()) montant += QString::number(tab->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_fixe")->value(), 'f', 2);
            if(tab->findChild<QRadioButton *>("radioButtonOperation")->isChecked()) montant += tab->findChild<QLabel *>("label_operationResult")->text().split(" €").at(0);

            if(tab->findChild<QComboBox *>("comboBoxAdd_Cat2")->currentIndex() == 0) cat2 += "";
            else cat2 += tab->findChild<QLabel *>("label_idCat2")->text();

            cat += QString::number(id_categories.at(tab->findChild<QComboBox *>("comboBoxAdd_Cat")->currentIndex()));
            desc += tab->findChild<QLineEdit *>("lineEditAdd_description")->text().replace("'","''");
            ope += tab->findChild<QLineEdit *>("lineEditAdd_Operation")->text().replace("'","''");
        }

        QSqlQuery ajouter;
        ajouter.prepare("INSERT INTO Transactions (id_compte, date, type, moyen, categorie, sous_categorie, description, montant, detail_montant, fichier) "
                          "VALUES ('"+emit OverAid_id_compte()+"', '"+ui->dateEditAdd->date().toString("dd/MM/yyyy")+"', '"+type.replace("'","''")+"', '"+moyen+"', '"+cat+"', "
                          "'"+cat2+"', '"+desc+"', '"+montant+"', '"+ope+"',:pdf);");
        if(ui->lineEditPDF->text().isEmpty())
        {
            ajouter.bindValue(":pdf","");
            ajouter.exec();
        }
        else
        {
            ajouter.bindValue(":pdf", PDFInByteArray);
            ajouter.exec();
        }

        QMessageBox::information(this, tr("Transaction ajoutée"), tr("La transaction a bien été ajoutée."), tr("Fermer"));
        actu();
    }
}

//Lignes comptables
void AddForm::on_tabWidget_Line_currentChanged(int index)
{
    if(index == ui->tabWidget_Line->count()-1)
    {
        lineForm *newTab = new lineForm();
        ui->tabWidget_Line->insertTab(ui->tabWidget_Line->count()-1, newTab, tr("Ligne ") + QString::number(ui->tabWidget_Line->count()));
        ui->tabWidget_Line->setCurrentIndex(index);
        connect(newTab, SIGNAL(OverAid_id_compte()), this, SLOT(getId_compte()));
        connect(newTab, SIGNAL(OverAid_amountChanged()), this, SLOT(totalAmount()));
        newTab->actu_categorie();
        newTab->actu_desc();

        for(int i = 0; i < ui->tabWidget_Line->count(); i++) ui->tabWidget_Line->setTabText(i, "Ligne "+QString::number(i+1));
        ui->tabWidget_Line->setTabText(ui->tabWidget_Line->count()-1,"+");

        totalAmount();
    }
}

void AddForm::totalAmount()
{
    double montant = 0;
    for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
    {
        if(ui->tabWidget_Line->widget(i)->findChild<QRadioButton *>("radioButtonFixe")->isChecked()) montant += ui->tabWidget_Line->widget(i)->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_fixe")->value();
        if(ui->tabWidget_Line->widget(i)->findChild<QRadioButton *>("radioButtonOperation")->isChecked()) montant += ui->tabWidget_Line->widget(i)->findChild<QLabel *>("label_operationResult")->text().split(" €").at(0).toDouble();
    }
    if(ui->tabWidget_Line->count() == 2) ui->label_totalAmount->clear();
    else ui->label_totalAmount->setText(tr("Montant total : ")+tr("%1 €").arg(montant));
}

void AddForm::on_tabWidget_Line_tabCloseRequested(int index)
{
    if(index != 0) ui->tabWidget_Line->setCurrentIndex(index-1);
    if(ui->tabWidget_Line->tabText(index) != "+") ui->tabWidget_Line->removeTab(index);

    for(int i = 0; i < ui->tabWidget_Line->count(); i++) ui->tabWidget_Line->setTabText(i, "Ligne "+QString::number(i+1));
    ui->tabWidget_Line->setTabText(ui->tabWidget_Line->count()-1,"+");

    totalAmount();
}
