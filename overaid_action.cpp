#include "overaid.h"
#include "ui_overaid.h"
#include "managecategories.h"
#include "ui_managecategories.h"
#include "manageaccount.h"
#include "ui_manageaccount.h"
#include "managesubscriptions.h"
#include "ui_managesubscriptions.h"


//Sauvegarder la BDD
void OverAid::on_actionSauvegarder_la_base_de_donn_es_triggered()
{
    QString dbFile = QFileDialog::getSaveFileName(this, tr("Sauvegarder la base de données"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/"+"database "+QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm")+".db"), tr("Base de données (*.db)");
    if(!dbFile.isEmpty()) QFile::copy(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db", dbFile);
}

//Exporter au format CSV
void OverAid::on_actionExporter_au_format_CSV_triggered()
{
    QString csvFile = QFileDialog::getSaveFileName(this, tr("Exporter au format CSV"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/"+tr("transaction.csv"), tr("Fichier CSV (*.csv)"));
    QFile data(csvFile);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        QTextStream output(&data);
        output << "id_trans" << ";" << tr("Date") << ";" << tr("Dépense / Entrée") << ";" << tr("Moyen de paiement") << ";" << tr("Catégorie") << ";" << tr("Sous-catégorie") << ";" << tr("Description") << ";" << tr("Montant") << ";" << tr("Devise") << ";" << tr("Montant Devise Compte") << ";" << tr("Détail montant") << ";" << tr("Fichier PDF") << Qt::endl;
        output << "" << ";" << "" << ";" << ui->comboBoxFiltre_inOut->currentText() << ";" << ui->comboBoxFiltre_moyen->currentText() << ";";

        QString cat, cat2;
        bool allCatActionsChecked = true;
        foreach(QAction *catAction, ui->pushButtonFiltre_cat->menu()->actions())
        {
            if(catAction->isCheckable())
            {
                if(catAction->isChecked()) cat += catAction->text()+",";
                else allCatActionsChecked = false;
            }
        }
        if(cat.endsWith(",")) cat.resize(cat.size()-1);
        if(allCatActionsChecked) cat = tr("Tout");

        bool allCat2ActionsChecked = true;
        foreach(QAction *cat2Action, ui->pushButtonFiltre_cat2->menu()->actions())
        {
            if(cat2Action->isCheckable())
            {
                if(cat2Action->isChecked()) cat2 += cat2Action->text()+",";
                else allCatActionsChecked = false;
            }
        }
        if(cat2.endsWith(",")) cat2.resize(cat2.size()-1);
        if(allCat2ActionsChecked) cat2 = tr("Tout");

        output << cat << ";" << cat2 << ";" << ui->lineEditFiltre_desc->text()+"%" << ";" << "" << ";" << "" << ";" << ";" << ";" << "" << Qt::endl;

        QString columnNameExceptFile = "*";
        QSqlQuery nomDesColonnes("SELECT group_concat(name, ',') AS colonnes FROM pragma_table_info('Transactions') WHERE name <> 'fichier'");
        if(nomDesColonnes.next()) columnNameExceptFile = nomDesColonnes.value("colonnes").toString()+", CASE WHEN fichier IS NULL OR fichier = '' THEN 'Non' ELSE 'Oui' END AS fichier";

        QSqlQuery transaction("SELECT "+columnNameExceptFile+" FROM Transactions WHERE "+where+" ORDER BY (substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2)) ASC, id_trans ASC");
        while(transaction.next())
        {
            for(int i = 0; i <= transaction.value("categorie").toString().count(";"); i++)
            {
                output << transaction.value("id_trans").toString();
                output << ";";
                QDate date = QDate::fromString(transaction.value("date").toString(), "dd/MM/yyyy");
                if(language == "anglais") output << date.toString("MM/dd/yyyy");
                if(language == "francais") output << date.toString("dd/MM/yyyy");
                output << ";";

                QString inOutH = transaction.value("type").toString();
                if(inOutH == "Dépense") output << tr("Dépense");
                if(inOutH == "Entrée d'argent") output << tr("Entrée d'argent");
                output << ";";

                QString moyenH = transaction.value("moyen").toString();
                if(moyenH == "Carte bancaire") output << tr("Carte bancaire");
                if(moyenH == "Éspèces") output << tr("Éspèces");
                if(moyenH == "Chèque") output << tr("Chèque");
                if(moyenH == "Virement") output << tr("Virement");
                if(moyenH == "Prélèvement") output << tr("Prélèvement");
                output << ";";


                QSqlQuery cat("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("categorie").toString().remove("\"").split(";").at(i)+"'");
                if(cat.next()) output << cat.value("nom").toString();
                output << ";";

                QSqlQuery cat2("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("sous_categorie").toString().remove("\"").split(";").at(i)+"'");
                if(cat2.next()) output << cat2.value("nom").toString();
                output << ";";

                output << transaction.value("description").toString().split(";").at(i);
                output << ";";

                output << transaction.value("montant").toString().split(";").at(i);
                output << ";";

                output << transaction.value("devise").toString();
                output << ";";

                output << (transaction.value("montantDeviseCompte").toString().isEmpty() ? "" : transaction.value("montantDeviseCompte").toString().split(";").at(i));
                output << ";";

                output << transaction.value("detail_montant").toString().split(";").at(i);
                output << ";";

                output << transaction.value("fichier").toString();
                output << Qt::endl;
            }
        }
    }
}

//Exporter les PDF
void OverAid::on_actionExporter_les_PDF_en_ZIP_triggered()
{
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/Documents PDF").exists()) QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/Documents PDF");
    else
    {
        QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/Documents PDF").removeRecursively();
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/Documents PDF");
    }

    QSqlQuery transaction("SELECT * FROM Transactions WHERE "+where+" AND fichier !='' ORDER BY (substr(date, 7, 4) || '-' || substr(date, 4, 2) || '-' || substr(date, 1, 2)) DESC, id_trans DESC");
    while(transaction.next())
    {
        QFile file(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/Documents PDF/"+QDate::fromString(transaction.value("date").toString(),"dd/MM/yyyy").toString("yyyyMMdd")
                   +" | "+transaction.value("type").toString()+" | "+transaction.value("description").toString()+".pdf");
        if (file.open(QFile::WriteOnly))
        {
            file.write(transaction.value("fichier").toByteArray());
            file.close();
        }
    }

    QDesktopServices::openUrl(QUrl("file:///"+QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/Documents PDF/"));
}

//Fermer
void OverAid::on_actionFermer_triggered()
{
    qApp->quit();
}

//Gestion des comptes
void OverAid::on_actionAjouter_un_compte_triggered()
{
    ManageAccount *acc = new ManageAccount(nullptr);
    acc->setWindowModality(Qt::ApplicationModal);
    acc->show();
    connect(acc, SIGNAL(actualiser()), this, SLOT(actu_compte()));
}

void OverAid::actu_compte()
{
    //Actualiser actionBar Comptes
    ui->menuComptes->clear();
    ui->menuComptes->addAction(ui->actionAjouter_un_compte);

    QSqlQuery nbCompte("SELECT id_compte FROM Comptes");
    if(!nbCompte.next()) QSqlQuery courant("INSERT INTO Comptes (nom,montant_initial,active) VALUES ('Courant',0, 'true', 'EUR')");

    ui->menuComptes->insertAction(ui->actionAjouter_un_compte, createTextSeparator(tr("Comptes actifs")));

    QSqlQuery actif("SELECT id_compte,nom FROM Comptes WHERE active='true' ORDER BY id_compte ASC");
    while (actif.next())
    {
        QAction *action = new QAction(actif.value("nom").toString());
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=]() {this->changeAccount(action);});
        ui->menuComptes->insertAction(ui->actionAjouter_un_compte,action);
    }

    ui->menuComptes->insertAction(ui->actionAjouter_un_compte, createTextSeparator(tr("Comptes archivés")));

    QSqlQuery archive("SELECT id_compte,nom FROM Comptes WHERE active='false' ORDER BY id_compte ASC");
    while (archive.next())
    {
        QAction *action = new QAction(archive.value("nom").toString());
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=]() {this->changeAccount(action);});
        ui->menuComptes->insertAction(ui->actionAjouter_un_compte,action);
    }

    ui->menuComptes->insertSeparator(ui->actionAjouter_un_compte);
    changeAccount(ui->menuComptes->actions().at(1));
}

QWidgetAction* OverAid::createTextSeparator(const QString& text)
{
    QLabel* pLabel = new QLabel(text);
    pLabel->setAlignment(Qt::AlignCenter);
    pLabel->setGeometry(pLabel->x(), pLabel->y(), 100, 22);
    pLabel->setStyleSheet("QLabel{color: #828282}");
    QWidgetAction* separator = new QWidgetAction(this);
    separator->setDefaultWidget(pLabel);
    return separator;
}

void OverAid::changeAccount(QAction *actionClicked)
{
    foreach(QAction *action, ui->menuComptes->actions())
        if(action->isCheckable()) action->setChecked(false);

    actionClicked->setChecked(true);

    QSqlQuery compte("SELECT id_compte,active FROM Comptes WHERE nom='"+actionClicked->text()+"'");
    if(compte.next())
    {
        id_compte = compte.value("id_compte").toInt();
        active = compte.value("active").toString() == "true"? true : false;
    }

    actu(false, true);
}

//Gestion des catégories
void OverAid::on_actionCat_gories_triggered()
{
    ManageCategories *cat = new ManageCategories;
    cat->setWindowModality(Qt::ApplicationModal);
    cat->show();
    cat->id_compte = id_compte;
    cat->actu_categorie();
    cat->actu_sousCategorie();
    connect(cat, &ManageCategories::actualiser, this, [=](){actu(true, false);});
}

//Gestion des langues
void OverAid::on_actionAnglais_triggered()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv");

    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        QStringList headers = stream.readLine().split(";");
        QStringList parameters = stream.readLine().split(";");
        parameters[headers.indexOf("translate")] = "anglais";

        file.resize(0);
        stream << headers.join(";") << "\n" << parameters.join(";");
    }

    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void OverAid::on_actionFran_ais_triggered()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv");

    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        QStringList headers = stream.readLine().split(";");
        QStringList parameters = stream.readLine().split(";");
        parameters[headers.indexOf("translate")] = "francais";

        file.resize(0);
        stream << headers.join(";") << "\n" << parameters.join(";");
    }

    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

//Gérer les abonnements
void OverAid::on_actionAbonnements_triggered()
{
    ManageSubscriptions *sub = new ManageSubscriptions;
    sub->setWindowModality(Qt::ApplicationModal);
    sub->show();
    sub->id_compte = id_compte;
    sub->id_categories = id_categories;
    sub->actu();
}
