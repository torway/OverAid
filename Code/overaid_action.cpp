#include "overaid.h"
#include "ui_overaid.h"
#include "managecategories.h"
#include "manageaccount.h"
#include "managesubscriptions.h"
#include "bulkimport.h"


//Fermer
void OverAid::on_actionFermer_triggered()
{
    qApp->quit();
}

//A propos
void OverAid::on_action_about_triggered()
{
    qint64 bddSize = 0;
    QFileInfo bddInfo(overaidDatabase.databaseName());
    if (bddInfo.exists() && bddInfo.isFile()) bddSize = bddInfo.size();

    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("À propos"));
    aboutBox.setText(tr("À propos") +"\n\n"+ tr("Version : %1").arg(QApplication::applicationVersion()) +"\n"+ tr("Développé par : Tom BEBIN") +"\n\n"+ tr("Taille de la base de données : %1 Mo").arg(round(static_cast<double>(bddSize) / (1024 * 1024)*10)/10) +"\n\n"+ tr("Gérez vos comptes et vos dépenses en toute simplicité.") +"\n");
    aboutBox.exec();
}

//Sauvegarder la BDD
void OverAid::on_actionSauvegarder_la_base_de_donn_es_triggered()
{
    QString dbFile = QFileDialog::getSaveFileName(this, tr("Sauvegarder la base de données"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/"+"database "+QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm")+".db"), tr("Base de données (*.db)");
    if(!dbFile.isEmpty()) QFile::copy(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db", dbFile);
}

//Importer une BDD
void OverAid::on_actionImporter_une_base_de_donn_es_triggered()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Importer une base de données"));
    msgBox.setInformativeText(tr("Importer une base de données remplacera celle actuellement utilisée.\nCette action est irréversible.\nVeuillez sauvegarder vos données avant d'effectuer cette action.\n\nÊtes-vous sûr(e)(s) de vouloir continuer ?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setStyleSheet("QLabel{min-width: 350px;}");
    int ret = msgBox.exec();

    if(ret == QMessageBox::Ok) if(importDatabase())
        {
            QMessageBox::information(this, tr("Base de données importée"), tr("La base de données a bien été importée.\nElle remplace désormais l'ancienne."), tr("Fermer"));

            database();
            actu_settings();
        }
}

bool OverAid::importDatabase()
{
    QSqlDatabase importDatabase = QSqlDatabase::addDatabase("QSQLITE", "importedConnection");
    importDatabase.setDatabaseName(QFileDialog::getOpenFileName(this, tr("Choisir une base de données"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("Base de données (*.db)")));

    if(!importDatabase.open()) {
        qDebug() << tr("Impossible de se connecter à la base de donnée.");
        return false;
    }

    QStringList overaidTables = overaidDatabase.tables();
    QStringList importTables = importDatabase.tables();
    overaidTables.sort();
    importTables.sort();

    if(overaidTables != importTables) {
        QMessageBox::warning(this, tr("Erreur"), tr("La base de données n'a pas la structure de tables nécessaire."), tr("Fermer"));
        return false;
    }

    foreach (const QString& tableName, overaidDatabase.tables()) {
        QSqlRecord overaidRecord = overaidDatabase.record(tableName);
        QSqlRecord importRecord = importDatabase.record(tableName);

        if (overaidRecord != importRecord) {
            QMessageBox::warning(this, tr("Erreur"), tr("La table %1 n'a pas la structure nécessaire.").arg(tableName), tr("Fermer"));
            return false;
        }
    }

    QFile::remove(overaidDatabase.databaseName());
    QFile::copy(importDatabase.databaseName(), overaidDatabase.databaseName());

    return true;
}

//Speedtest
void OverAid::on_actionTest_de_vitesse_triggered()
{
    this->setEnabled(false);

    bool ok;
    int selectedOptionAmount = QInputDialog::getInt(nullptr, tr("Nombre de rechargement"), tr("Veuillez choisir le nombre de rechargement pour le test.\n\nPlus le nombre de rechargement est élevé, \nplus le test est représentatif mais plus de puissance sera utilisée."), 20, 0, 100000, 1, &ok);
    if(ok)
    {
        QTime startTime = QTime::currentTime();
        int minTime = 0, maxTime = 0, sommeTime = 0, totalTime = 0;
        QList<int> listTime;

        QWidget window;
        window.setWindowTitle(tr("Test de vitesse"));
        window.setFixedWidth(this->width()/2);
        window.setFixedHeight(40);

        QProgressBar progressBar;
        progressBar.setMinimum(0);
        progressBar.setMaximum(100);

        QVBoxLayout layout(&window);
        layout.addWidget(&progressBar);
        window.setLayout(&layout);

        window.show();

        for(int i = 0; i < selectedOptionAmount; i++)
        {
            QApplication::processEvents();
            QTime timer = QTime::currentTime();
            actu(true, false);
            int time = timer.msecsTo(QTime::currentTime());

            if(i == 0) minTime = maxTime = time;
            else
            {
                if(time < minTime) minTime = time;
                if(time > maxTime) maxTime = time;
            }
            listTime.append(time);
            sommeTime += time;
            progressBar.setValue(((i+1)*100) / selectedOptionAmount);
        }
        totalTime = startTime.msecsTo(QTime::currentTime());

        window.destroyed();

        QSqlQuery transaction("SELECT COUNT(*) FROM Transactions WHERE "+where);
        if(transaction.next())
        {
            QMessageBox result;
            result.setWindowTitle(tr("Test de vitesse"));
            result.setText(tr("Résultats pour %1 transactions et %2 rechargements : ").arg(transaction.value(0).toString(), QString::number(selectedOptionAmount)) +"\n\n"+
                           tr("Vitesse minimum : %1 msec").arg(minTime) +"\n"+
                           tr("Vitesse moyene : %1 msec").arg(sommeTime/listTime.size()) +"\n"+
                           tr("Vitesse maximum : %1 msec").arg(maxTime) +"\n\n"+
                           tr("Vitesse moyenne par transaction : %1 msec").arg(QString::number((double(sommeTime)/double(listTime.size()))/double(transaction.value(0).toInt()),'f',4)) +"\n"+
                           tr("Vitesse totale : %1 msec").arg(totalTime));
            result.exec();
        }
    }

    this->setEnabled(true);
}

//Exporter au format CSV
void OverAid::on_actionExporter_au_format_CSV_triggered()
{
    QString csvFile = QFileDialog::getSaveFileName(this, tr("Exporter au format CSV"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+"/"+tr("transaction.csv"), tr("Fichier CSV (*.csv)"));
    QFile data(csvFile);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        QTextStream output(&data);
        output << "id_trans" << ";" << tr("Date") << ";" << tr("Débit / Crédit") << ";" << tr("Moyen de paiement") << ";" << tr("Catégorie") << ";" << tr("Sous-catégorie") << ";" << tr("Description") << ";" << tr("Montant") << ";" << tr("Devise") << ";" << tr("Montant Devise Compte") << ";" << tr("Détail montant") << ";" << tr("Mode de saisie") << ";" << tr("Projet") << ";" << tr("Fichier PDF") << Qt::endl;
        output << "" << ";" << "> "+ui->dateEditFilter_start->date().toString(locale.dateFormat(QLocale::ShortFormat))+", < "+ui->dateEditFilter_end->date().toString(locale.dateFormat(QLocale::ShortFormat)) << ";" << ui->comboBoxFiltre_inOut->currentText() << ";" << ui->comboBoxFiltre_moyen->currentText() << ";";

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

        output << cat << ";" << cat2 << ";" << ui->lineEditFiltre_desc->text()+"%" << ";" << ";" << (ui->comboBoxFiltre_devise->currentIndex() != 0 ? ui->comboBoxFiltre_devise->currentText().first(3) : ui->comboBoxFiltre_devise->currentText()) << ";" << ";" << ";" << ui->comboBoxFiltre_modeSaisie->currentText() << ";" << ui->lineEditFiltre_projet->text() << ";" << Qt::endl;

        QString columnNameExceptFile = "*";
        QSqlQuery nomDesColonnes("SELECT group_concat(name, ',') AS colonnes FROM pragma_table_info('Transactions') WHERE name <> 'fichier'");
        if(nomDesColonnes.next()) columnNameExceptFile = nomDesColonnes.value("colonnes").toString()+", CASE WHEN fichier IS NULL OR fichier = '' THEN 'false' ELSE 'true' END AS fichier";

        QSqlQuery transaction("SELECT "+columnNameExceptFile+" FROM Transactions WHERE "+where+" ORDER BY date ASC, id_trans ASC");
        while(transaction.next())
        {
            for(int i = 0; i <= transaction.value("categorie").toString().count(";"); i++)
            {
                output << transaction.value("id_trans").toString() << ";";
                QDate date = QDate::fromString(transaction.value("date").toString(), "yyyyMMdd");
                output << locale.toString(date, locale.dateFormat(QLocale::ShortFormat).contains("yyyy") ? locale.dateFormat(QLocale::ShortFormat) : locale.dateFormat(QLocale::ShortFormat).replace("yy","yyyy")) << ";";

                QString inOutH = transaction.value("type").toString();
                if(inOutH == "Debit") output << tr("Débit");
                else if(inOutH == "Credit") output << tr("Crédit");
                output << ";";

                QString moyenH = transaction.value("moyen").toString();
                if(moyenH == "Carte bancaire") output << tr("Carte bancaire");
                else if(moyenH == "Espèces") output << tr("Espèces");
                else if(moyenH == "Chèque") output << tr("Chèque");
                else if(moyenH == "Virement") output << tr("Virement");
                else if(moyenH == "Prélèvement") output << tr("Prélèvement");
                output << ";";

                QSqlQuery cat("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("categorie").toString().remove("\"").split(";").at(i)+"'");
                if(cat.next()) output << cat.value("nom").toString();
                output << ";";

                QSqlQuery cat2("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("sous_categorie").toString().remove("\"").split(";").at(i)+"'");
                if(cat2.next()) output << cat2.value("nom").toString();
                output << ";";

                output << transaction.value("description").toString().split(";").at(i) << ";";
                output << transaction.value("montant").toString().split(";").at(i) << ";";
                output << transaction.value("devise").toString() << ";";
                output << (transaction.value("montantDeviseCompte").toString().isEmpty() ? "" : transaction.value("montantDeviseCompte").toString().split(";").at(i)) << ";";
                output << transaction.value("detail_montant").toString().split(";").at(i) << ";";

                QString modeSaisieH = transaction.value("modeSaisie").toString();
                if(modeSaisieH == "Manuel") output << tr("Manuel");
                else if(modeSaisieH == "Abonnement") output << tr("Abonnement");
                else if(modeSaisieH == "Import en masse") output << tr("Import en masse");
                output << ";";

                output << transaction.value("projet").toString() << ";";
                output << (transaction.value("fichier").toString() == "true" ? tr("Oui") : tr("Non")) << Qt::endl;
            }
        }
    }
}

//Exporter les PDF
void OverAid::on_actionExporter_les_PDF_en_ZIP_triggered()
{
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/"+tr("Documents PDF")).exists()) QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/"+tr("Documents PDF"));
    else
    {
        QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/"+tr("Documents PDF")).removeRecursively();
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/"+tr("Documents PDF"));
    }

    QSqlQuery transaction("SELECT * FROM Transactions WHERE "+where+" AND fichier !='' ORDER BY date DESC, id_trans DESC");
    while(transaction.next())
    {
        QFile file(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/"+tr("Documents PDF")+"/"+transaction.value("date").toString()
                   +" | "+transaction.value("type").toString()+" | "+transaction.value("description").toString()+".pdf");
        if (file.open(QFile::WriteOnly))
        {
            file.write(transaction.value("fichier").toByteArray());
            file.close();
        }
    }

    QDesktopServices::openUrl(QUrl("file:///"+QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)+"/"+tr("Documents PDF")+"/"));
}

//Import en masse
void OverAid::on_actionImport_en_masse_triggered()
{
    BulkImport *bulk = new BulkImport(nullptr);
    bulk->setWindowModality(Qt::ApplicationModal);
    bulk->show();
    connect(bulk, &BulkImport::actualiser, this, [=](){actu(true, false);});
}

//Gestion des comptes
void OverAid::on_actionGerer_les_comptes_triggered()
{
    ManageAccount *acc = new ManageAccount(nullptr);
    acc->setWindowModality(Qt::ApplicationModal);
    acc->show();
    connect(acc, &ManageAccount::actualiser, this, [=](){
        actu_compte();
        QSqlQuery settings("SELECT * FROM Settings");
        if(settings.next()) {
        QSqlQuery compte("SELECT nom FROM Comptes WHERE id_compte='"+settings.value("defaultAccount").toString()+"'");
        if(compte.next()) {
            foreach(QAction *action, ui->menuComptes->actions()) {
                if(action->text() == compte.value(0).toString()) {
                    changeAccount(action);
                    break;
                }
            }
        }
        }});
    connect(acc, &ManageAccount::actuSolde, this, [=](){actu(true,false);});
}

void OverAid::actu_compte()
{
    //Actualiser actionBar Comptes
    ui->menuComptes->clear();
    ui->menuComptes->addAction(ui->actionGerer_les_comptes);

    QSqlQuery nbCompte("SELECT id_compte FROM Comptes");
    if(!nbCompte.next()) QSqlQuery courant("INSERT INTO Comptes (nom,montant_initial,active) VALUES ('Courant',0, 'true', 'EUR')");

    ui->menuComptes->insertAction(ui->actionGerer_les_comptes, createTextSeparator(tr("Comptes actifs")));

    int nb_account = 1;
    QSqlQuery actif("SELECT id_compte,nom FROM Comptes WHERE active='true' ORDER BY id_compte ASC");
    while (actif.next())
    {
        QAction *action = new QAction(actif.value("nom").toString());
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=]() {this->changeAccount(action);});
        if(nb_account < 10) action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0 + nb_account));
        ui->menuComptes->insertAction(ui->actionGerer_les_comptes,action);
        nb_account++;
    }

    ui->menuComptes->insertAction(ui->actionGerer_les_comptes, createTextSeparator(tr("Comptes archivés")));

    int nb_accountArchived = 1;
    QSqlQuery archive("SELECT id_compte,nom FROM Comptes WHERE active='false' ORDER BY id_compte ASC");
    while (archive.next())
    {
        QAction *action = new QAction(archive.value("nom").toString());
        action->setCheckable(true);
        connect(action, &QAction::triggered, this, [=]() {this->changeAccount(action);});
        if(nb_accountArchived < 10) action->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_0 + nb_accountArchived));
        ui->menuComptes->insertAction(ui->actionGerer_les_comptes,action);
        nb_accountArchived++;
    }

    ui->menuComptes->insertSeparator(ui->actionGerer_les_comptes);
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

    QSqlQuery nb_cat("SELECT COUNT(*) FROM Catégories WHERE id_compte='"+QString::number(id_compte)+"'");
    if(nb_cat.next() && nb_cat.value(0).toInt() < 1) on_actionCat_gories_triggered();

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
    connect(cat, &ManageCategories::actualiser, this, [=](){actu(true, true);});
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

//Gestion des langues
void OverAid::on_actionFran_ais_triggered()
{
    actu_langage("Français");
}

void OverAid::on_actionAnglais_triggered()
{
    actu_langage("English");
}

void OverAid::on_actionEspagnol_triggered()
{
    actu_langage("Español");
}

void OverAid::actu_langage(QString langage)
{
    QSqlQuery settings("UPDATE Settings SET language='"+langage+"'");
    
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/translate.txt");

    if(file.open(QIODevice::ReadWrite))
    {
        file.resize(0);
        QTextStream stream(&file);
        stream << langage;
        file.close();
    }

    QMessageBox::warning(this, tr("Changement de langue"), tr("L'application va redémarrer."), tr("Fermer"));

    qApp->quit();
    QProcess::startDetached(qApp->applicationFilePath(), qApp->arguments());
}

void OverAid::on_actionAfficher_le_d_bugger_triggered()
{
    QSqlQuery settings("UPDATE Settings SET showDebugWindow='"+QString(ui->actionAfficher_le_d_bugger->isChecked() ? "true":"false")+"'");
    ui->actionAfficher_le_d_bugger->isChecked()? ui->plainTextEdit_debug->show() : ui->plainTextEdit_debug->hide();
}

//Gérer les paramètres
void OverAid::actu_settings()
{
    update();
    actu_devise();

    QSqlQuery settings("SELECT * FROM Settings");
    if(settings.next()) {
        qDebug() << "version : " << settings.value("version").toString();

        //Compte par défault
        actu_compte();
        QSqlQuery compte("SELECT nom FROM Comptes WHERE id_compte='"+settings.value("defaultAccount").toString()+"'");
        if(compte.next()) {
            foreach(QAction *action, ui->menuComptes->actions()) {
                if(action->text() == compte.value(0).toString()) {
                    changeAccount(action);
                    break;
                }
            }
        }

        //Fenêtre debug
        settings.value("showDebugWindow").toString() == "true" ? ui->plainTextEdit_debug->show() : ui->plainTextEdit_debug->hide();
        settings.value("showDebugWindow").toString() == "true" ? ui->actionAfficher_le_d_bugger->setChecked(true) : ui->actionAfficher_le_d_bugger->setChecked(false);

        //GroupBox Filtre
        if(settings.value("showGroupFilter").toString() == "false") {
            ui->groupBox_filtre->setMaximumHeight(20);
            ui->groupBox_filtre->setChecked(false);
        }

        //Geometry
        if(settings.value("geometry").toString() != "0;0;0;0")
            this->setGeometry(QRect(settings.value("geometry").toString().split(";").at(0).toInt(),settings.value("geometry").toString().split(";").at(1).toInt(),settings.value("geometry").toString().split(";").at(2).toInt(),settings.value("geometry").toString().split(";").at(3).toInt()));
    }
}
