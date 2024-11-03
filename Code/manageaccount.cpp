#include "manageaccount.h"
#include "ui_manageaccount.h"

ManageAccount::ManageAccount(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManageAccount)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("OverAid © - Gérer les comptes"));

    QStringList allCurrencies;
    QSqlQuery devises("SELECT * FROM Devises");
    while(devises.next()) allCurrencies.append(devises.value("code").toString()+" : "+devises.value("nom").toString()+" ("+devises.value("symbole").toString()+")");

    ui->comboBox_devise->addItems(allCurrencies);
    ui->comboBox_devise_2->addItems(allCurrencies);
    ui->comboBox_devise_2->setEnabled(false);

    actu();
}

ManageAccount::~ManageAccount()
{
    delete ui;
    emit actualiser();
}

void ManageAccount::on_pushButton_close_clicked()
{
    this->close();
    emit actualiser();
}

void ManageAccount::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit actualiser();
}

void ManageAccount::actu()
{
    ui->listWidget->clear();
    id_account.clear();

    QSqlQuery compte("SELECT * FROM Comptes");
    while(compte.next())
    {
        id_account.append(compte.value("id_compte").toInt());
        ui->listWidget->addItem(compte.value("nom").toString());
    }

    QSqlQuery defaut("SELECT defaultAccount FROM Settings");
    if(defaut.next())
    {
        QFont bold;
        bold.setBold(true);
        bold.setUnderline(true);
        ui->listWidget->item(id_account.indexOf(defaut.value(0).toInt()))->setFont(bold);
    }

    if(ui->listWidget->item(0)) ui->listWidget->setCurrentItem(ui->listWidget->item(0));
}

void ManageAccount::on_pushButton_add_clicked()
{
    QSqlQuery nom("SELECT * FROM Comptes WHERE nom='"+ui->lineEdit_nom->text()+"'");
    nom.last();
    if(nom.at() >= 0) QMessageBox::warning(this, tr("Erreur"), tr("Un compte possède déjà ce nom."), tr("Fermer"));
    else if(ui->lineEdit_nom->text().isEmpty()) QMessageBox::warning(this, "Erreur", "Le nom n'est pas renseigné.", "Fermer");
    else
    {
        QSqlQuery compte("INSERT INTO Comptes (nom,montant_initial,active,devise) VALUES ('"+ui->lineEdit_nom->text().replace("'","''")+"', '"+QString::number(ui->doubleSpinBox->value())+"', "
                         "'true', '"+ui->comboBox_devise->currentText().left(3)+"')");

        QMessageBox::information(this, "Compte ajouté", "Le compte a bien été ajouté.", "Fermer");
        actu();
    }
}

void ManageAccount::on_pushButton_delete_clicked()
{
    //Supprimer le compte
    if(ui->listWidget->currentRow() != -1)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Supprimer le compte"));
        msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer le compte '%1' ?").arg(ui->listWidget->currentItem()->text())+
                                  tr("\n\nSupprimer ce compte entraînera la suppression de toutes ses transactions."));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setStyleSheet("QLabel{min-width: 350px;}");
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok)
        {
            QSqlQuery remove("DELETE FROM Comptes WHERE id_compte='"+QString::number(id_account.at(ui->listWidget->currentRow()))+"'");
            QSqlQuery remove2("DELETE FROM Transactions WHERE id_compte='"+QString::number(id_account.at(ui->listWidget->currentRow()))+"'");
            QSqlQuery remove3("DELETE FROM Abonnements WHERE id_compte='"+QString::number(id_account.at(ui->listWidget->currentRow()))+"'");

            QSqlQuery defaut("SELECT defaultAccount FROM Settings");
            if(defaut.next() && defaut.value("defaultAccount").toInt() == id_account.at(ui->listWidget->currentRow()))
            {
                QSqlQuery comptes("SELECT id_compte FROM Comptes");
                while(comptes.next())
                {
                    if(comptes.value("id_comptes").toInt() != id_account.at(ui->listWidget->currentRow()))
                    {
                        QSqlQuery defaut("UPDATE Settings SET defaultAccount='"+comptes.value("id_compte").toString()+"'");
                        break;
                    }
                }
                actu();
            }
            else actu();
        }
    }
    else QMessageBox::warning(this, tr("Impossible de supprimer le compte."), tr("Aucun compte n'a été séléctionné."), tr("Corriger"));
}

void ManageAccount::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow != -1)
    {
        QSqlQuery modify("SELECT * FROM Comptes WHERE id_compte='"+QString::number(id_account.at(currentRow))+"'");
        if(modify.next())
        {
            ui->doubleSpinBox_2->setValue(modify.value("montant_initial").toDouble());
            ui->lineEdit_nom_2->setText(modify.value("nom").toString());
            ui->checkBox_archive->setChecked(modify.value("active").toString() == "true"? false : true);
            ui->comboBox_devise_2->setCurrentIndex(ui->comboBox_devise_2->findText(modify.value("devise").toString(),Qt::MatchStartsWith));
        }
    }
}

void ManageAccount::on_pushButton_modify_clicked()
{
    if(!ui->lineEdit_nom_2->text().isEmpty())
    {
        QSqlQuery compte("UPDATE Comptes SET nom='"+ui->lineEdit_nom_2->text().replace("'","''")+"', montant_initial='"+QString::number(ui->doubleSpinBox_2->value())+
                         "', active='"+(ui->checkBox_archive->isChecked() == true? "false" : "true")+"' WHERE id_compte='"+QString::number(id_account.at(ui->listWidget->currentRow()))+"'");

        if(ui->checkBox_archive->isChecked()) QSqlQuery remove("DELETE FROM Abonnements WHERE id_compte='"+QString::number(id_account.at(ui->listWidget->currentRow()))+"'");

        QMessageBox::information(this, tr("Compte modifié"), tr("Le compte a bien été modifié."), tr("Fermer"));
        actu();
        emit actuSolde();
    }
    else QMessageBox::warning(this, tr("Erreur"), tr("Le nom n'est pas renseigné."), tr("Fermer"));
}

void ManageAccount::on_comboBox_devise_currentTextChanged(const QString &arg1)
{
    ui->doubleSpinBox->setSuffix(" "+arg1.split("(").at(1).split(")").at(0));
}

void ManageAccount::on_comboBox_devise_2_currentTextChanged(const QString &arg1)
{
    ui->doubleSpinBox_2->setSuffix(" "+arg1.split("(").at(1).split(")").at(0));
}

void ManageAccount::on_pushButton_defaultAccount_clicked()
{
    if(ui->listWidget->currentRow() != -1)
    {
        QSqlQuery remove("UPDATE Settings SET defaultAccount='"+QString::number(id_account.at(ui->listWidget->currentRow()))+"'");
    }
    actu();
}

