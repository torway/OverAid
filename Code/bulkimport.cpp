#include "bulkimport.h"
#include "ui_bulkimport.h"
#include "overaid.h"

BulkImport::BulkImport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BulkImport)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("OverAid © - Import en masse"));

    ui->pushButton_import->setEnabled(false);

    connect(ui->plainTextEdit_import->verticalScrollBar(), &QScrollBar::valueChanged, ui->plainTextEdit_error->verticalScrollBar(), &QScrollBar::setValue);
    connect(ui->plainTextEdit_error->verticalScrollBar(), &QScrollBar::valueChanged, ui->plainTextEdit_import->verticalScrollBar(), &QScrollBar::setValue);
}

BulkImport::~BulkImport()
{
    delete ui;
    emit actualiser();
}

void BulkImport::on_pushButton_close_clicked()
{
    this->close();
    emit actualiser();
}

void BulkImport::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit actualiser();
}

void BulkImport::on_pushButton_download_clicked()
{
    QString bulkModel = QFileDialog::getSaveFileName(this, tr("Télécharger le modèle d'import en masse"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/"+tr("import_en_masse.xlsx"), tr("Excel (*.xlsx)"));
    if(!bulkModel.isEmpty()) QFile::copy(":/qrc/ressources/Bulk_Import.xlsx", bulkModel);
}

void BulkImport::on_plainTextEdit_import_textChanged()
{
    ui->pushButton_import->setEnabled(false);
}

void BulkImport::on_pushButton_check_clicked()
{
    ui->plainTextEdit_error->clear();

    QStringList lines = ui->plainTextEdit_import->toPlainText().split("\n");
    for(int i = 0; i < lines.size(); i++)
    {
        QString line = lines.at(i).trimmed();
        QString check = checkFields(line);
        if(check == "OK") ui->plainTextEdit_error->insertPlainText(i != lines.size()-1 ? "\n" : "");
        else ui->plainTextEdit_error->insertPlainText(tr("Erreur : ")+check+(i != lines.size()-1 ? "\n" : ""));
    }

    ui->pushButton_import->setEnabled(true);
}

QString BulkImport::checkFields(QString line)
{
    QStringList fields = line.split(";");

    if(fields.size() != 8) return tr("Le nombre de champs (séparés par des ';') est different de 8.");

    QSqlQuery accountSQL("SELECT id_compte FROM Comptes WHERE nom='"+fields.at(0)+"'");
    if(!accountSQL.next()) return tr("Le compte '%1' n'existe pas.").arg(fields.at(0));

    if(!QDate::fromString(fields.at(1),"yyyyMMdd").isValid()) return tr("La date n'est pas au bon format.");

    if(fields.at(2) != "+" && fields.at(2) != "-") return tr("Le crédit/débit n'est ni un +, ni un -.");

    QString cat;
    QSqlQuery catSQL("SELECT id_cat FROM Catégories WHERE nom='"+fields.at(3)+"'");
    if(catSQL.next()) cat = catSQL.value(0).toString();
    else return tr("La catégorie '%1' n'existe pas.").arg(fields.at(3));

    QSqlQuery cat2SQL("SELECT id_cat FROM Catégories WHERE nom='"+fields.at(4)+"' AND cat0='"+cat+"'");
    if(!cat2SQL.next()) return tr("La sous-catégorie '%1' n'existe pas dans la catégorie '%2'.").arg(fields.at(4), fields.at(3));

    static QRegularExpression regex("^\\d+(?:\\.\\d{0,2})?$");
    if(!regex.match(fields.at(6)).hasMatch()) return tr("Le montant n'est pas au bon format.");

    return "OK";
}

void BulkImport::on_pushButton_import_clicked()
{
    QString importReste, errorReste;

    QStringList lines = ui->plainTextEdit_import->toPlainText().split("\n");
    for(int i = 0; i < lines.size(); i++)
    {
        QString line = lines.at(i).trimmed();
        if(!addLine(line))
        {
            importReste += line+(i != lines.size()-1 ? "\n" : "");
            errorReste += ui->plainTextEdit_error->toPlainText().split("\n").at(i).trimmed()+(i != lines.size()-1 ? "\n" : "");
        }
    }

    ui->plainTextEdit_import->setPlainText(importReste);
    ui->plainTextEdit_error->setPlainText(errorReste);

    QMessageBox::information(this, tr("Transactions ajoutées"), tr("Les transactions sans erreur ont bien été ajoutées."), tr("Fermer"));

    if(ui->plainTextEdit_error->toPlainText().isEmpty())
    {
        this->close();
        emit actualiser();
    }
}

bool BulkImport::addLine(QString line)
{
    QStringList fields = line.split(";");

    QString account, devise;
    QSqlQuery accountSQL("SELECT id_compte,devise FROM Comptes WHERE nom='"+fields.at(0)+"'");
    if(accountSQL.next())
    {
        account = accountSQL.value("id_compte").toString();
        devise = accountSQL.value("devise").toString();
    }
    else return false;

    QString date;
    if(QDate::fromString(fields.at(1),"yyyyMMdd").isValid()) date = fields.at(1);
    else return false;

    QString debitCredit;
    if(fields.at(2) == "+") debitCredit = "Credit";
    else if (fields.at(2) == "-") debitCredit = "Debit";
    else return false;

    QString cat;
    QSqlQuery catSQL("SELECT id_cat FROM Catégories WHERE nom='"+fields.at(3)+"'");
    if(catSQL.next()) cat = catSQL.value(0).toString();
    else return false;

    QString cat2;
    QSqlQuery cat2SQL("SELECT id_cat FROM Catégories WHERE nom='"+fields.at(4)+"' AND cat0='"+cat+"'");
    if(cat2SQL.next()) cat2 = cat2SQL.value(0).toString();
    else return false;

    QString desc = QString(fields.at(5)).replace("'","''");

    QString montant;
    static QRegularExpression regex("^\\d+(?:\\.\\d{0,2})?$");
    if(regex.match(fields.at(6)).hasMatch()) montant = fields.at(6);
    else return false;

    QString projet = QString(fields.at(7)).replace("'","''");

    QSqlQuery ajout("INSERT INTO Transactions (id_compte, date, type, moyen, categorie, sous_categorie, description, montant, detail_montant, fichier, devise, montantDeviseCompte, projet, modeSaisie) "
                    "VALUES ('"+account+"', '"+date+"', '"+debitCredit+"', 'Carte bancaire', '"+cat+"', '"+cat2+"', '"+desc+"', '"+montant+"', '', '', '"+devise+"', '', '"+projet+"', 'Import en masse');");

    return true;
}

