#include "transform.h"
#include "ui_transform.h"
#include "lineform.h"
#include "ui_lineform.h"
#include "overaid.h"

TransForm::TransForm(QWidget *parent, QString command) :
    QWidget(parent),
    ui(new Ui::TransForm)
{
    ui->setupUi(this);

    commande = command;
    ui->dateEditAdd->setDate(QDate::currentDate());

    if(commande == "Modifier")
    {
        this->setWindowTitle(tr("OverAid © - Modifier la transaction"));
        ui->pushButtonAdd->setText("Modifier");
        ui->groupBoxAdd->setTitle("Modifier la transaction");

        this->setMaximumWidth(16777215);
        ui->groupBoxAdd->setMaximumWidth(16777215);
    }
    else if(commande == "Abonnement")
    {
        this->setWindowTitle(tr("OverAid © - Abonnements"));
        ui->groupBoxAdd->setTitle("Ajouter un abonnement");
        ui->dateEditAdd->setDisplayFormat(QString("d").repeated(locale.dateFormat(QLocale::ShortFormat).count('d')));
        ui->label_4->setText(tr("Jour"));
        ui->dateEditAdd->setDate(QDate(2000,01,01));
    }
    else if(commande == "Modifier abo")
    {
        this->setWindowTitle(tr("OverAid © - Modifier l'abonnement"));
        ui->pushButtonAdd->setText("Modifier");
        ui->groupBoxAdd->setTitle("Modifier l'abonnement");
        ui->dateEditAdd->setDisplayFormat(QString("d").repeated(locale.dateFormat(QLocale::ShortFormat).count('d')));
        ui->label_4->setText(tr("Jour"));
        ui->dateEditAdd->setDate(QDate(2000,01,01));

        this->setMaximumWidth(16777215);
        ui->groupBoxAdd->setMaximumWidth(16777215);
    }

    ui->label_id->hide();
}

TransForm::~TransForm()
{
    delete ui;
}

void TransForm::actu_projet()
{
    //Autocompletion Projet
    QStringList projetList;
    QSqlQuery projet("SELECT DISTINCT projet FROM Transactions WHERE id_compte='"+QString::number(id_compte)+"' ORDER BY projet ASC");
    while(projet.next()) if(!projetList.contains(projet.value("projet").toString())) projetList.append(projet.value("projet").toString());

    QCompleter *com(new QCompleter(projetList, this));
    com->setCaseSensitivity(Qt::CaseInsensitive);
    com->setFilterMode(Qt::MatchContains);
    ui->lineEdit_projet->setCompleter(com);
}

void TransForm::actu_devise()
{
    QStringList allCurrencies;
    QSqlQuery devises("SELECT * FROM Devises");
    while(devises.next()) allCurrencies.append(devises.value("code").toString()+" : "+devises.value("nom").toString()+" ("+devises.value("symbole").toString()+")");
    ui->comboBox_devise->addItems(allCurrencies);

    QSqlQuery deviseCompte("SELECT devise FROM Comptes WHERE id_compte='"+QString::number(id_compte)+"'");
    if(deviseCompte.next()) ui->comboBox_devise->setCurrentIndex(ui->comboBox_devise->findText(deviseCompte.value("devise").toString(),Qt::MatchStartsWith));
}

void TransForm::on_comboBox_devise_currentTextChanged(const QString &arg1)
{
    QString devise = " "+arg1.split("(").at(1).split(")").at(0);
    foreach(QObject *object, ui->tabWidget_Line->findChild<QStackedWidget*>()->children())
        if(qobject_cast<lineForm*>(object))
        {
            qobject_cast<lineForm*>(object)->ui->doubleSpinBoxAdd_fixe->setSuffix(devise);
            if(!qobject_cast<lineForm*>(object)->ui->label_operationResult->text().isEmpty()) qobject_cast<lineForm*>(object)->ui->label_operationResult->setText(qobject_cast<lineForm*>(object)->ui->label_operationResult->text().split(" ").at(0)+devise);

            QSqlQuery deviseCompte("SELECT devise FROM Comptes WHERE id_compte='"+QString::number(id_compte)+"'");
            if(deviseCompte.next() && deviseCompte.value("devise").toString() == arg1.left(3))
            {
                qobject_cast<lineForm*>(object)->ui->label_deviseCompte->hide();
                qobject_cast<lineForm*>(object)->ui->doubleSpinBoxAdd_deviseCompte->hide();
            }
            else
            {
                qobject_cast<lineForm*>(object)->ui->label_deviseCompte->show();
                qobject_cast<lineForm*>(object)->ui->doubleSpinBoxAdd_deviseCompte->show();
                QSqlQuery symbole("SELECT symbole FROM Devises WHERE code='"+deviseCompte.value("devise").toString()+"'");
                if(symbole.next()) qobject_cast<lineForm*>(object)->ui->doubleSpinBoxAdd_deviseCompte->setSuffix(" "+symbole.value("symbole").toString());
            }
        }

    totalAmount();
}

//Debit ou Credit
void TransForm::on_pushButton_out_clicked()
{
    ui->checkBox_inOut->setCheckState(Qt::Unchecked);
    ui->pushButton_in->setChecked(false);
    ui->pushButton_out->setChecked(true);
}

void TransForm::on_pushButton_in_clicked()
{
    ui->checkBox_inOut->setCheckState(Qt::Checked);
    ui->pushButton_in->setChecked(true);
    ui->pushButton_out->setChecked(false);
}

void TransForm::on_checkBox_inOut_stateChanged(int arg1)
{
    if(arg1) on_pushButton_in_clicked();
    else on_pushButton_out_clicked();
}

//PDF
void TransForm::on_pushButtonAdd_PDF_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choisir un fichier"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), tr("PDF (*.pdf)"));
    if(path != "") ui->lineEditPDF->setText(path);
}

void TransForm::on_pushButton_deletePDF_clicked()
{
    ui->lineEditPDF->clear();
}

//Lignes comptables
void TransForm::on_tabWidget_Line_currentChanged(int index)
{
    if(index == ui->tabWidget_Line->count()-1)
    {
        lineForm *newTab = new lineForm;
        ui->tabWidget_Line->insertTab(ui->tabWidget_Line->count()-1, newTab, tr("Ligne ") + QString::number(ui->tabWidget_Line->count()));
        ui->tabWidget_Line->setCurrentIndex(index);
        connect(newTab, SIGNAL(OverAid_amountChanged()), this, SLOT(totalAmount()));
        newTab->id_categories = id_categories;
        newTab->id_compte = id_compte;
        newTab->actu_categorie();
        newTab->actu_desc();
        if(commande != "Abonnement" && commande != "Modifier abo") newTab->ui->toolButton->hide();

        for(int i = 0; i < ui->tabWidget_Line->count(); i++) ui->tabWidget_Line->setTabText(i, tr("Ligne ")+QString::number(i+1));
        ui->tabWidget_Line->setTabText(ui->tabWidget_Line->count()-1,"+");

        if(ui->comboBox_devise->currentIndex() != -1) on_comboBox_devise_currentTextChanged(ui->comboBox_devise->currentText());
        totalAmount();
    }
}

void TransForm::totalAmount()
{
    double montant = 0;
    for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
    {
        if(ui->tabWidget_Line->widget(i)->findChild<QRadioButton *>("radioButtonFixe")->isChecked()) montant += ui->tabWidget_Line->widget(i)->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_fixe")->value();
        else if(ui->tabWidget_Line->widget(i)->findChild<QRadioButton *>("radioButtonOperation")->isChecked()) montant += ui->tabWidget_Line->widget(i)->findChild<QLabel *>("label_operationResult")->text().split(" ").at(0).toDouble();
    }
    if(ui->tabWidget_Line->count() == 2) ui->label_totalAmount->clear();
    else ui->label_totalAmount->setText(tr("Montant total : ")+QString::number(montant,'f',2)+" "+ui->comboBox_devise->currentText().split("(").at(1).split(")").at(0));
}

void TransForm::on_tabWidget_Line_tabCloseRequested(int index)
{
    if(index != 0) ui->tabWidget_Line->setCurrentIndex(index-1);
    if(ui->tabWidget_Line->tabText(index) != "+") ui->tabWidget_Line->widget(index)->~QWidget();

    for(int i = 0; i < ui->tabWidget_Line->count(); i++) ui->tabWidget_Line->setTabText(i, tr("Ligne ")+QString::number(i+1));
    ui->tabWidget_Line->setTabText(ui->tabWidget_Line->count()-1,"+");

    foreach(QObject *object, ui->tabWidget_Line->findChild<QStackedWidget*>()->children())
        if(qobject_cast<lineForm*>(object)) qobject_cast<lineForm*>(object)->actu_desc();

    totalAmount();
}

void TransForm::on_pushButtonAdd_clicked()
{
    bool catExistOk = true;
    for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
        if(ui->tabWidget_Line->widget(i)->findChild<QComboBox *>("comboBoxAdd_Cat")->count() == 0) catExistOk = false;

    bool descriptionOk = true;
    for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
        if(ui->tabWidget_Line->widget(i)->findChild<QLineEdit *>("lineEditAdd_description")->text().isEmpty()) descriptionOk = false;

    if(!catExistOk) QMessageBox::warning(this, "Erreur", "Aucune catégorie n'existe.", "Fermer");
    else if(!descriptionOk) QMessageBox::warning(this, "Erreur", "Une des descriptions n'est pas renseignée.", "Fermer");
    else
    {
        QString type;
        if(ui->checkBox_inOut->checkState() == Qt::Unchecked) type = "Debit";
        else type = "Credit";

        QString moyen;
        if(ui->comboBox_moyen->currentText() == tr("Carte bancaire")) moyen = "Carte bancaire";
        else if(ui->comboBox_moyen->currentText() == tr("Espèces")) moyen = "Espèces";
        else if(ui->comboBox_moyen->currentText() == tr("Chèque")) moyen = "Chèque";
        else if(ui->comboBox_moyen->currentText() == tr("Virement")) moyen = "Virement";
        else if(ui->comboBox_moyen->currentText() == tr("Prélèvement")) moyen = "Prélèvement";

        QString montant, cat2, cat, desc, ope, montantDeviseCompte;
        for(int i = 0; i < ui->tabWidget_Line->count()-1; i++)
        {
            QWidget *tab = ui->tabWidget_Line->widget(i);

            if(i != 0)
            {
                montant += ";";
                cat2 += ";";
                cat += ";";
                desc += ";";
                ope += ";";
                if(!tab->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_deviseCompte")->isHidden()) montantDeviseCompte += ";";
            }

            if(tab->findChild<QRadioButton *>("radioButtonFixe")->isChecked()) montant += QString::number(tab->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_fixe")->value(), 'f', 2);
            else if(tab->findChild<QRadioButton *>("radioButtonOperation")->isChecked()) montant += tab->findChild<QLabel *>("label_operationResult")->text().split(" ").at(0);

            if(!tab->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_deviseCompte")->isHidden()) montantDeviseCompte += QString::number(tab->findChild<QDoubleSpinBox *>("doubleSpinBoxAdd_deviseCompte")->value(), 'f', 2);

            if(tab->findChild<QComboBox *>("comboBoxAdd_Cat2")->currentIndex() == 0) cat2 += "\"\"";
            else cat2 += "\""+tab->findChild<QLabel *>("label_idCat2")->text()+"\"";

            cat += "\""+QString::number(id_categories.at(tab->findChild<QComboBox *>("comboBoxAdd_Cat")->currentIndex()))+"\"";
            desc += tab->findChild<QLineEdit *>("lineEditAdd_description")->text().replace("'","''");
            ope += tab->findChild<QLineEdit *>("lineEditAdd_Operation")->text().replace("'","''");
        }

        QSqlQuery query;
        if(commande == "Ajouter")
        {
            query.prepare("INSERT INTO Transactions (id_compte, date, type, moyen, categorie, sous_categorie, description, montant, detail_montant, fichier, devise, montantDeviseCompte, projet) "
                          "VALUES ('"+QString::number(id_compte)+"', '"+ui->dateEditAdd->date().toString("yyyyMMdd")+"', '"+type.replace("'","''")+"', '"+moyen+"', '"+cat+"', "
                          "'"+cat2+"', '"+desc+"', '"+montant+"', '"+ope+"',:pdf, '"+ui->comboBox_devise->currentText().left(3)+"', '"+montantDeviseCompte+"', '"+ui->lineEdit_projet->text().replace("'","''")+"');");

            QMessageBox::information(this, tr("Transaction ajoutée"), tr("La transaction a bien été ajoutée."), tr("Fermer"));
        }

        if(commande == "Modifier")
        {
            query.prepare("UPDATE Transactions SET date='"+ui->dateEditAdd->date().toString("yyyyMMdd")+"', type='"+type.replace("'","''")+"', moyen='"+moyen+"', categorie='"+cat+"', sous_categorie='"+cat2+"', "
                          "description='"+desc+"', montant='"+montant+"', detail_montant='"+ope+"', fichier=:pdf, devise='"+ui->comboBox_devise->currentText().left(3)+"', montantDeviseCompte='"+montantDeviseCompte+"', projet='"+ui->lineEdit_projet->text().replace("'","''")+"' "
                          "WHERE id_trans='"+ui->label_id->text()+"'");

            QMessageBox::information(this, tr("Transaction modifiée"), tr("La transaction a bien été modifiée."), tr("Fermer"));
            this->close();
        }

        if(commande == "Abonnement")
        {
            query.prepare("INSERT INTO Abonnements (id_compte, type, moyen, categorie, sous_categorie, description, montant, detail_montant, fichier, renouvellement, dernier, devise, montantDeviseCompte, projet) "
                              "VALUES ('"+QString::number(id_compte)+"', '"+type.replace("'","''")+"', '"+moyen+"', '"+cat+"', "
                              "'"+cat2+"', '"+desc+"', '"+montant+"', '"+ope+"',:pdf, '"+ui->dateEditAdd->date().toString("dd")+"', '"+QDate::currentDate().toString("yyyyMM")+"', '"+ui->comboBox_devise->currentText().left(3)+"', "
                              "'"+montantDeviseCompte+"', '"+ui->lineEdit_projet->text().replace("'","''")+"');");

            QMessageBox::information(this, tr("Abonnement ajouté"), tr("L'abonnement a bien été ajouté."), tr("Fermer"));
        }

        if(commande == "Modifier abo")
        {
            query.prepare("UPDATE Abonnements SET type='"+type.replace("'","''")+"', moyen='"+moyen+"', categorie='"+cat+"', sous_categorie='"+cat2+"', description='"+desc+"', montant='"+montant+"', detail_montant='"+ope+"'"
                          ", fichier=:pdf, renouvellement='"+ui->dateEditAdd->date().toString("dd")+"', devise='"+ui->comboBox_devise->currentText().left(3)+"', montantDeviseCompte='"+montantDeviseCompte+"', projet='"+ui->lineEdit_projet->text().replace("'","''")+"' "
                          "WHERE id_sub='"+ui->label_id->text()+"'");
            QMessageBox::information(this, tr("Abonnement modifié"), tr("L'abonnement a bien été modifié."), tr("Fermer"));
            this->close();
        }

        if(ui->lineEditPDF->text().isEmpty()) query.bindValue(":pdf","");
        else
        {
            QFile file(ui->lineEditPDF->text());
            if (!file.open(QIODevice::ReadOnly)) return;
            QByteArray PDFInByteArray = file.readAll();

            query.bindValue(":pdf",PDFInByteArray);
        }
        query.exec();

        emit actu();
    }
}
