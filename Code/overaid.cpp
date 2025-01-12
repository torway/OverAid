#include "overaid.h"
#include "ui_overaid.h"
#include "lineform.h"
#include "ui_lineform.h"
#include "Classes/custommenu.h"


OverAid::OverAid(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::OverAid)
{
    ui->setupUi(this);
    this->setWindowTitle("OverAid ©");

    //Menu clic droit
    ui->lineEditFiltre_desc->setContextMenuPolicy(Qt::DefaultContextMenu);
    ui->treeWidgetSummary->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidgetSummary, &QTreeWidget::customContextMenuRequested, this, &OverAid::treeWidgetMenu);
}

OverAid::~OverAid()
{
    delete ui;
}

void OverAid::database()
{
    overaidDatabase = QSqlDatabase::addDatabase("QSQLITE");
    overaidDatabase.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db");

    //if(!QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/"+qApp->organizationName()).exists()) QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/"+qApp->organizationName());
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).exists()) QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db").exists())
    {
        if(overaidDatabase.open()) qDebug() << tr("Connexion à la base de donnée réussie.");
        else qDebug() << tr("Impossible de se connecter à la base de donnée.");

        QSqlQuery settings("CREATE TABLE 'Settings' ('version' TEXT NOT NULL DEFAULT 0, 'language' TEXT NOT NULL DEFAULT 'Francais', 'defaultAccount' TEXT NOT NULL DEFAULT '1', 'showGroupFilter' TEXT NOT NULL DEFAULT 'true', 'showDebugWindow' TEXT NOT NULL DEFAULT 'false', 'geometry' TEXT NOT NULL DEFAULT '0;0;0;0');");
        if(!settings.executedQuery().isEmpty())
        {
            //Créer la ligne de paramètres
            QSqlQuery settings("INSERT INTO Settings (version) VALUES ('"+version+"')");

            QString langue = "English";
            if(QLocale::languageToString(QLocale::system().language()) == "French") langue = "Français";
            else if(QLocale::languageToString(QLocale::system().language()) == "Spanish") langue = "Español";

            QSqlQuery langage("UPDATE Settings SET language='"+langue+"'");

            QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/translate.txt");
            if (file.open(QIODevice::ReadWrite))
            {
                file.resize(0);
                QTextStream stream(&file);
                stream << langue;
                file.close();
            }
        }
        QSqlQuery compte("CREATE TABLE 'Comptes' ('id_compte' INTEGER NOT NULL, 'nom' TEXT NOT NULL, 'montant_initial' DOUBLE NOT NULL, 'devise' TEXT NOT NULL, 'active' TEXT NOT NULL, PRIMARY KEY('id_compte' AUTOINCREMENT));");
        QSqlQuery transaction("CREATE TABLE 'Transactions' ('id_trans' INTEGER NOT NULL, 'id_compte' TEXT NOT NULL,'date' TEXT NOT NULL,'type' TEXT NOT NULL,'moyen' TEXT NOT NULL,'categorie' TEXT NOT NULL,'sous_categorie' TEXT NOT NULL,'description' TEXT NOT NULL,'montant' DOUBLE NOT NULL,'devise' TEXT NOT NULL,'montantDeviseCompte' DOUBLE NOT NULL,'detail_montant' TEXT NOT NULL,'fichier' BLOB,PRIMARY KEY('id_trans' AUTOINCREMENT));");
        QSqlQuery categorie("CREATE TABLE 'Catégories' ('id_cat' INTEGER NOT NULL,'type' TEXT,'nom' TEXT NOT NULL,'cat0' TEXT NOT NULL,'id_compte' INTEGER, PRIMARY KEY('id_cat' AUTOINCREMENT))");
        QSqlQuery abonnement("CREATE TABLE'Abonnements' ('id_sub' INTEGER NOT NULL,'id_compte' INTEGER NOT NULL,'renouvellement' TEXT NOT NULL,'type' TEXT NOT NULL,'moyen' TEXT NOT NULL,'categorie' TEXT NOT NULL,'sous_categorie' TEXT NOT NULL,'description' TEXT NOT NULL,'montant' DOUBLE NOT NULL,'devise' TEXT NOT NULL,'montantDeviseCompte' DOUBLE NOT NULL,'detail_montant' TEXT NOT NULL,'fichier' BLOB NOT NULL,'dernier' TEXT NOT NULL, PRIMARY KEY('id_sub' AUTOINCREMENT));");
        QSqlQuery filtre("CREATE TABLE 'Filtres' ('id_filtre' INTEGER NOT NULL, 'id_compte' INTEGER NOT NULL, 'nom' TEXT NOT NULL, 'type' TEXT NOT NULL, 'moyen' TEXT NOT NULL, 'categorie' TEXT NOT NULL, 'sous_categorie' TEXT NOT NULL, 'date_debut' TEXT NOT NULL, 'date_fin' TEXT NOT NULL, 'description' TEXT NOT NULL, 'devise' TEXT NOT NULL, PRIMARY KEY('id_filtre' AUTOINCREMENT));");
        QSqlQuery devise("CREATE TABLE 'Devises' ('id_currency' INTEGER NOT NULL, 'code' TEXT NOT NULL, 'nom' TEXT NOT NULL, 'symbole' TEXT NOT NULL, PRIMARY KEY('id_currency' AUTOINCREMENT));");
        if(!devise.executedQuery().isEmpty())
        {
            foreach (QString currency, devisesList) QSqlQuery currencyAdd("INSERT INTO Devises (code,nom,symbole) VALUES ('"+currency.split("-").at(0)+"','"+currency.split("-").at(1)+"','"+currency.split("-").at(2)+"')");

            //Créer le premier compte
            bool done = false, done2 = false, done3 = false;
            while(done == false)
            {
                bool ok;
                QString selectedOptionName = QInputDialog::getText(nullptr, tr("Nouveau compte"), tr("Veuillez choisir un nom pour votre compte."), QLineEdit::Normal, tr("Compte courant"), &ok);
                if(ok && !selectedOptionName.isEmpty())
                {
                    while(done2 == false)
                    {
                        bool ok;

                        QStringList allCurrencies;
                        QSqlQuery devises("SELECT * FROM Devises");
                        while(devises.next()) allCurrencies.append(devises.value("code").toString()+" : "+devises.value("nom").toString()+" ("+devises.value("symbole").toString()+")");

                        QString selectedOptionCurrency = QInputDialog::getItem(nullptr, tr("Nouveau compte"), tr("Veuillez choisir une devise pour votre compte."), allCurrencies, 0, false, &ok);
                        if(ok && !selectedOptionCurrency.isEmpty())
                        {
                            while(done3 == false)
                            {
                                bool ok;
                                double selectedOptionAmount = QInputDialog::getDouble(nullptr, tr("Nouveau compte"), tr("Veuillez choisir le montant initial pour votre compte."), 0, -100000, 100000, 2, &ok);
                                if(ok)
                                {
                                    done3 = true;
                                    QSqlQuery courant("INSERT INTO Comptes (nom,montant_initial,devise,active) VALUES ('"+selectedOptionName+"', '"+QString::number(selectedOptionAmount,'f',2)+"', '"+selectedOptionCurrency.left(3)+"', 'true')");
                                }
                            }
                        }
                        done2 = true;
                    }
                    done = true;
                }
            }
        }
    }
    else
    {
        if(overaidDatabase.open()) qDebug() << tr("Connexion à la base de donnée réussie.");
        else qDebug() << tr("Impossible de se connecter à la base de donnée.");
    }
}

void OverAid::closeEvent(QCloseEvent *event)
{
    QSqlQuery geometry("UPDATE Settings SET geometry='"+QString::number(this->geometry().x())+";"+QString::number(this->geometry().y())+";"+QString::number(this->geometry().width())+";"+QString::number(this->geometry().height())+"'");
    event->accept();
}

void OverAid::treeWidgetMenu(const QPoint &pos)
{
    bool transId = false, firstColumn = false;
    if(ui->treeWidgetSummary->currentItem() && !ui->treeWidgetSummary->currentItem()->text(11).isEmpty()) transId = true;
    if(ui->treeWidgetSummary->currentColumn() == 0) firstColumn = true;

    QAction *newAct_etendre = new QAction(tr("Tout étendre"), this);
    connect(newAct_etendre, SIGNAL(triggered()), this, SLOT(tout_etendre()));
    QAction *newAct_retract = new QAction(tr("Tout rétrécir"), this);
    connect(newAct_retract, SIGNAL(triggered()), this, SLOT(tout_retrecir()));

    QAction *newAct_copyCase = new QAction(tr("Copier la case"), this);
    connect(newAct_copyCase, SIGNAL(triggered()), this, SLOT(copy_case()));
    QAction *newAct_copyLine = new QAction(tr("Copier la ligne"), this);
    connect(newAct_copyLine, SIGNAL(triggered()), this, SLOT(copy_line()));

    QAction *newAct_abo = new QAction(tr("Convertir en abonnement"), this);
    connect(newAct_abo, &QAction::triggered, this, [this]{transToSub(true);});
    active && transId? newAct_abo->setEnabled(true) : newAct_abo->setEnabled(false);
    QAction *newAct_aboSansPDF = new QAction(tr("Convertir en abonnement (sans PDF)"), this);
    connect(newAct_aboSansPDF, &QAction::triggered, this, [this]{transToSub(false);});
    active && transId? newAct_aboSansPDF->setEnabled(true) : newAct_aboSansPDF->setEnabled(false);

    QAction *newAct = new QAction(tr("Modifier cette transaction"), this);
    connect(newAct, SIGNAL(triggered()), this, SLOT(modify_trans()));
    active && transId? newAct->setEnabled(true) : newAct->setEnabled(false);
    QAction *newAct_dup = new QAction(tr("Dupliquer cette transaction"), this);
    connect(newAct_dup, SIGNAL(triggered()), this, SLOT(duplicate()));
    active && transId? newAct_dup->setEnabled(true) : newAct_dup->setEnabled(false);
    QAction *newAct_delete = new QAction(tr("Supprimer cette transaction"), this);
    connect(newAct_delete, SIGNAL(triggered()), this, SLOT(delete_trans()));
    active && transId? newAct_delete->setEnabled(true) : newAct_delete->setEnabled(false);

    QAction *newAct_showAllColumns = new QAction(tr("Afficher toutes les colonnes"), this);
    connect(newAct_showAllColumns, SIGNAL(triggered()), this, SLOT(showAllColumns()));
    QAction *newAct_hideThisColumn = new QAction(tr("Masquer cette colonne"), this);
    connect(newAct_hideThisColumn, SIGNAL(triggered()), this, SLOT(hideThisColumn()));
    firstColumn? newAct_hideThisColumn->setEnabled(false) : newAct_hideThisColumn->setEnabled(true);


    QMenu menu(this);
    menu.addAction(newAct_etendre);
    menu.addAction(newAct_retract);
    menu.addSeparator();
    menu.addAction(newAct_copyCase);
    menu.addAction(newAct_copyLine);
    menu.addSeparator();
    menu.addAction(newAct_abo);
    menu.addAction(newAct_aboSansPDF);
    menu.addSeparator();
    menu.addAction(newAct);
    menu.addAction(newAct_dup);
    menu.addAction(newAct_delete);
    menu.addSeparator();
    menu.addAction(newAct_showAllColumns);
    menu.addAction(newAct_hideThisColumn);
    menu.exec(ui->treeWidgetSummary->mapToGlobal(pos));
}

void OverAid::actu(bool remember, bool actuCat)
{
    QTime time = QTime::currentTime();

    //MAJ Abonnement
    QSqlQuery abo("SELECT * FROM Abonnements WHERE CAST(dernier AS int)<'"+QDate::currentDate().toString("yyyyMM")+"'");
    while(abo.next())
    {
        QString dernier = abo.value("dernier").toString();
        if(dernier.isEmpty())
        {
            QSqlQuery update_abo("UPDATE Abonnements SET dernier='"+QDate::currentDate().addMonths(-1).toString("yyyyMM")+"' WHERE id_sub='"+abo.value("id_sub").toString()+"'");
            dernier = QDate::currentDate().addMonths(-1).toString("yyyyMM");
        }

        int renouvellement = abo.value("renouvellement").toInt();
        QDate lastDate = locale.toDate(dernier + abo.value("renouvellement").toString(),"yyyyMMdd");
        while(!lastDate.isValid())
        {
            renouvellement--;
            lastDate = locale.toDate(dernier + QString::number(renouvellement),"yyyyMMdd");
        }

        while(lastDate < QDate::currentDate().addDays(1-QDate::currentDate().day()))
        {
            renouvellement = abo.value("renouvellement").toInt();
            lastDate = lastDate.addMonths(1);
            QDate saveDate = lastDate;
            lastDate.setDate(saveDate.year(),saveDate.month(),renouvellement);
            while(!lastDate.isValid())
            {
                renouvellement--;
                lastDate.setDate(saveDate.year(),saveDate.month(),renouvellement);
            }

            QString expFinal = abo.value("description").toString();
            while(expFinal.contains("%"))
            {
                QString exp = expFinal.split("%").at(1).split("%").at(0);
                exp.replace("MM",QString::number(lastDate.month()));
                exp.replace('M',QString::number(lastDate.month()));
                exp.replace('A',QString::number(lastDate.year()));

                lineForm calcul;
                QByteArray expBA = exp.toUtf8();
                calcul.expressionToParse = expBA.constData();
                double resultat = calcul.expression();

                while(resultat > 12 && expFinal.split("%").at(1).split("%").at(0).contains("M"))
                {
                    resultat -= 12;
                    expFinal.replace("%A","%A+1");
                }
                while(resultat < 1 && expFinal.split("%").at(1).split("%").at(0).contains("M"))
                {
                    resultat += 12;
                    expFinal.replace("%A","%A-1");
                }

                QString result = QString::number(resultat);

                if(expFinal.split("%").at(1).split("%").at(0).contains("MM"))
                {
                    if(result.length() == 1) result = "0"+result;
                    result = locale.toString(locale.toDate(result,"MM"),"MMMM");
                    result[0] = result[0].toUpper();
                }

                expFinal = expFinal.replace("%"+expFinal.split("%").at(1).split("%").at(0)+"%",result);
            }
            if(!abo.value("description").toString().contains("%")) expFinal = abo.value("description").toString();

            QSqlQuery ajouter;
            ajouter.prepare("INSERT INTO Transactions (id_compte, date, type, moyen, categorie, sous_categorie, description, montant, detail_montant, fichier, devise, montantDeviseCompte, modeSaisie, projet) "
                            "VALUES ('"+abo.value("id_compte").toString()+"', '"+lastDate.toString("yyyyMMdd")+"', '"+abo.value("type").toString().replace("'","''")+"', '"+abo.value("moyen").toString().replace("'","''")+"', '"+abo.value("categorie").toString()+"', "
                            "'"+abo.value("sous_categorie").toString()+"', '"+expFinal.replace("'","''")+"', '"+abo.value("montant").toString()+"', '"+abo.value("detail_montant").toString()+"',:pdf, '"+abo.value("devise").toString()+"', "
                            "'"+abo.value("montantDeviseCompte").toString()+"', 'Abonnement', '"+abo.value("projet").toString()+"');");

            if(abo.value("fichier").toByteArray().isEmpty()) ajouter.bindValue(":pdf","");
            else ajouter.bindValue(":pdf",abo.value("fichier").toByteArray());

            ajouter.exec();

            QSqlQuery update("UPDATE Abonnements SET dernier='"+lastDate.toString("yyyyMM")+"' WHERE id_sub='"+abo.value("id_sub").toString()+"'");
        }
    }
    abo.last();
    if(abo.at() >= 0) QMessageBox::information(this, tr("Abonnements ajoutés"), tr("Les abonnements ont bien été ajoutés."), tr("Fermer"));

    //--------
    if(remember)
    {
        for(int i = 0; i < ui->treeWidgetSummary->topLevelItemCount(); i++)
        {
            rememberOpenedYearsID.append(ui->treeWidgetSummary->topLevelItem(i)->text(0));
            rememberOpenedYears.append(ui->treeWidgetSummary->topLevelItem(i)->isExpanded() ? "true" : "false");
            for(int j = 0; j < ui->treeWidgetSummary->topLevelItem(i)->childCount(); j++)
            {
                rememberOpenedMonthID.append(ui->treeWidgetSummary->topLevelItem(i)->text(0)+ui->treeWidgetSummary->topLevelItem(i)->child(j)->text(0));
                rememberOpenedMonth.append(ui->treeWidgetSummary->topLevelItem(i)->child(j)->isExpanded() ? "true" : "false");
                for(int k = 0; k < ui->treeWidgetSummary->topLevelItem(i)->child(j)->childCount(); k++)
                {
                    rememberOpenedItemsID.append(ui->treeWidgetSummary->topLevelItem(i)->child(j)->child(k)->text(11));
                    rememberOpenedItems.append(ui->treeWidgetSummary->topLevelItem(i)->child(j)->child(k)->isExpanded() ? "true" : "false");
                }
            }
        }
        rememberPosition = ui->treeWidgetSummary->verticalScrollBar()->sliderPosition();
    }

    if(actuCat)
    {
        actu_categorie();
        actu_sousCategorie();
        actu_savedFilters();
    }
    actu_historique();
    actu_desc();
    actu_projet();

    if(ui->tabWidget->currentIndex() == 1) on_tabWidget_currentChanged(1);

    //Reset Adding trans window
    ui->widgetTrans->parentWidget()->layout()->replaceWidget(ui->widgetTrans,trans,Qt::FindDirectChildrenOnly);
    disconnect(trans, 0, this, 0);
    connect(trans, &TransForm::actu, this, [=](){actu(true, false);});
    trans->id_categories = id_categories;
    trans->id_compte = id_compte;
    if(trans->ui->comboBox_devise->count() == 0) trans->actu_devise();
    trans->actu_projet();

    int count = trans->ui->tabWidget_Line->tabBar()->count();
    for(int i = 0; i <= count; i++)
    {
        trans->on_tabWidget_Line_currentChanged(0);
        trans->on_tabWidget_Line_tabCloseRequested(0);
    }

    trans->on_pushButton_deletePDF_clicked();

    if(active)
    {
        trans->setEnabled(true);
        ui->actionAbonnements->setEnabled(true);
        ui->actionCat_gories->setEnabled(true);
    }
    else
    {
        trans->setEnabled(false);
        ui->actionAbonnements->setEnabled(false);
        ui->actionCat_gories->setEnabled(false);
    }

    qDebug() << "actu time : " << time.msecsTo(QTime::currentTime()) << " msec.";
}

void OverAid::actu_desc()
{
    //Autocompletion Description
    QStringList descriptionList;
    QSqlQuery desc("SELECT DISTINCT description FROM Transactions WHERE id_compte='"+QString::number(id_compte)+"' ORDER BY description ASC");
    while (desc.next())
    {
        if(desc.value("description").toString().contains(";"))
        {
            for(int i = 0; i <= desc.value("description").toString().count(";"); i++)
                if(!descriptionList.contains(desc.value("description").toString().split(";").at(i).trimmed())) descriptionList.append(desc.value("description").toString().split(";").at(i).trimmed());
        }
        else if(!descriptionList.contains(desc.value("description").toString().trimmed())) descriptionList.append(desc.value("description").toString().trimmed());
    }

    QCompleter *com(new QCompleter(descriptionList, this));
    com->setCaseSensitivity(Qt::CaseInsensitive);
    com->setFilterMode(Qt::MatchContains);
    ui->lineEditFiltre_desc->setCompleter(com);
}

void OverAid::actu_projet()
{
    //Autocompletion Projet
    QStringList projetList;
    QSqlQuery projet("SELECT DISTINCT projet FROM Transactions WHERE id_compte='"+QString::number(id_compte)+"' ORDER BY projet ASC");
    while (projet.next())
        if(!projetList.contains(projet.value("projet").toString())) projetList.append(projet.value("projet").toString());

    QCompleter *com(new QCompleter(projetList, this));
    com->setCaseSensitivity(Qt::CaseInsensitive);
    com->setFilterMode(Qt::MatchContains);
    ui->lineEditFiltre_projet->setCompleter(com);
}

void OverAid::actu_categorie()
{
    id_categories.clear();

    if(ui->pushButtonFiltre_cat->findChild<CustomMenu *>()) ui->pushButtonFiltre_cat->findChild<CustomMenu *>()->deleteLater();
    CustomMenu *menuCat = new CustomMenu(ui->pushButtonFiltre_cat, ui->pushButtonFiltre_cat);
    QSqlQuery categorie("SELECT id_cat,nom FROM Catégories WHERE type='0' AND id_compte='"+QString::number(id_compte)+"' ORDER BY nom");
    while(categorie.next())
    {
        id_categories.append(categorie.value("id_cat").toInt());

        QAction *cat = new QAction(categorie.value("nom").toString());
        cat->setCheckable(true);
        cat->setChecked(true);
        menuCat->addAction(cat);
    }
    menuCat->addSeparator();
    QAction *tout = new QAction(tr("Sélectionner tout"));
    QAction *rien = new QAction(tr("Désélectionner tout"));
    menuCat->addAction(tout);
    menuCat->addAction(rien);
    connect(tout, SIGNAL(triggered()), menuCat, SLOT(activateAll()));
    connect(rien, SIGNAL(triggered()), menuCat, SLOT(activateNone()));
    ui->pushButtonFiltre_cat->setMenu(menuCat);

    disconnect(ui->pushButtonFiltre_cat->menu(), &CustomMenu::aboutToHide, 0, 0);
    connect(ui->pushButtonFiltre_cat->menu(), &CustomMenu::aboutToHide, this, [this]
    {
        QList<bool> newActionsCatState;
        foreach(QAction *action, ui->pushButtonFiltre_cat->menu()->actions())
            if(action->isCheckable()) newActionsCatState.append(action->isChecked());

        if(newActionsCatState != previousActionsCatState) actu_sousCategorie();
    });
}

void OverAid::on_pushButtonFiltre_cat_pressed()
{
    previousActionsCatState.clear();
    foreach(QAction *action, ui->pushButtonFiltre_cat->menu()->actions())
        if(action->isCheckable()) previousActionsCatState.append(action->isChecked());
}

void OverAid::actu_sousCategorie()
{
    //Filtres sous-catégorie
    id_sousCategories.clear();

    QString cat0_filtre = "cat0='-1'"; // Par défaut, si aucune catégorie n'est cochée

    if(ui->pushButtonFiltre_cat->menu()->actions().count() >= 4) {
        QStringList cat0FilterList;

        for(QAction *catAction : ui->pushButtonFiltre_cat->menu()->actions().mid(0, ui->pushButtonFiltre_cat->menu()->actions().count() - 3))
        {
            QString idCat = QString::number(id_categories.at(ui->pushButtonFiltre_cat->menu()->actions().indexOf(catAction)));
            if(catAction->isChecked()) cat0FilterList << "cat0='" + idCat + "'";
        }

        if(!cat0FilterList.isEmpty()) cat0_filtre = "(" + cat0FilterList.join(" OR ") + ")";
    }

    if(ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()) ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()->deleteLater();
    CustomMenu *menuCat2 = new CustomMenu(ui->pushButtonFiltre_cat2, ui->pushButtonFiltre_cat2);

    QSqlQuery sousCategorie_filtre("SELECT * FROM Catégories WHERE type='1' AND "+cat0_filtre+" AND id_compte='"+QString::number(id_compte)+"' ORDER BY nom");
    while(sousCategorie_filtre.next())
    {
        bool alreadyExists = false;
        int alreadyExistsID = -1;
        QString sous_cat = sousCategorie_filtre.value("id_cat").toString();

        foreach(QAction *cat2Action, menuCat2->actions())
        {
            if(cat2Action->text() == sousCategorie_filtre.value("nom").toString())
            {
                alreadyExists = true;
                alreadyExistsID = menuCat2->actions().indexOf(cat2Action);
                break;
            }
        }

        if(!alreadyExists)
        {
            id_sousCategories.append("sous_categorie LIKE '%\""+sous_cat+"\"%' ");

            QAction *cat2Action = new QAction(sousCategorie_filtre.value("nom").toString());
            cat2Action->setCheckable(true);
            cat2Action->setChecked(true);
            menuCat2->addAction(cat2Action);
        }
        //Deux id pour le même nom de sous catégories
        else
        {
            QString old = id_sousCategories.at(alreadyExistsID);
            id_sousCategories.append(old + "OR sous_categorie LIKE '%\""+sous_cat+"\"%' ");
            id_sousCategories.removeAt(alreadyExistsID);
        }
    }
    menuCat2->addSeparator();
    QAction *vide = new QAction(tr("Vide"));
    vide->setCheckable(true);
    vide->setChecked(true);
    menuCat2->addAction(vide);
    id_sousCategories.append("sous_categorie LIKE '%\"\"%' ");
    menuCat2->addSeparator();
    QAction *tout = new QAction(tr("Sélectionner tout"));
    QAction *rien = new QAction(tr("Désélectionner tout"));
    menuCat2->addAction(tout);
    menuCat2->addAction(rien);
    connect(tout, SIGNAL(triggered()), menuCat2, SLOT(activateAll()));
    connect(rien, SIGNAL(triggered()), menuCat2, SLOT(activateNone()));
    ui->pushButtonFiltre_cat2->setMenu(menuCat2);
}

void OverAid::actu_devise()
{
    ui->comboBoxFiltre_devise->clear();
    ui->comboBoxFiltre_devise->addItem(tr("Tout"));
    QStringList allCurrencies;
    QSqlQuery devises("SELECT * FROM Devises");
    while(devises.next()) allCurrencies.append(devises.value("code").toString()+" : "+devises.value("nom").toString()+" ("+devises.value("symbole").toString()+")");
    ui->comboBoxFiltre_devise->addItems(allCurrencies);
}

void OverAid::on_pushButtonFiltre_valide_clicked()
{
    actu_historique();
    if(ui->tabWidget->currentIndex() == 1) on_tabWidget_currentChanged(1);
}

void OverAid::on_pushButtonFiltre_delete_clicked()
{
    actu_categorie();
    actu_sousCategorie();
    ui->comboBoxFiltre_inOut->setCurrentIndex(0);
    ui->comboBoxFiltre_moyen->setCurrentIndex(0);
    ui->lineEditFiltre_desc->clear();
    ui->lineEditFiltre_projet->clear();
    ui->dateEditFilter_start->setDate(QDate(1900,1,1));
    ui->dateEditFilter_end->setDate(QDate(2100,12,31));
    ui->comboBoxFiltre_devise->setCurrentIndex(0);
    ui->comboBoxFiltre_modeSaisie->setCurrentIndex(0);
    if(ui->comboBox_savedFilters->currentIndex() != 0) ui->comboBox_savedFilters->setCurrentIndex(0); //Afin d'éviter les boucles infinies
    on_pushButtonFiltre_valide_clicked();
}

void OverAid::actu_savedFilters()
{
    //Filtres sauvegardés
    disconnect(ui->comboBox_savedFilters, 0, this, 0);
    ui->label_idSavedFilter->hide();
    ui->comboBox_savedFilters->clear();
    id_savedFilters.clear();
    ui->comboBox_savedFilters->addItem("");
    id_savedFilters.append(-1);

    QSqlQuery filtres("SELECT id_filtre,nom FROM Filtres WHERE id_compte='"+QString::number(id_compte)+"'");
    while (filtres.next())
    {
        ui->comboBox_savedFilters->addItem(filtres.value("nom").toString());
        id_savedFilters.append(filtres.value("id_filtre").toInt());
    }
    ui->comboBox_savedFilters->insertSeparator(ui->comboBox_savedFilters->count());
    ui->comboBox_savedFilters->addItem(tr("Sauvegarder et remplacer ce filtre"));
    ui->comboBox_savedFilters->addItem(tr("Sauvegarder comme nouveau filtre"));
    ui->comboBox_savedFilters->addItem(tr("Supprimer ce filtre"));
    ui->comboBox_savedFilters->setItemData(ui->comboBox_savedFilters->findText(tr("Sauvegarder et remplacer ce filtre")), QVariant(0), Qt::ItemDataRole::UserRole - 1);
    ui->comboBox_savedFilters->setItemData(ui->comboBox_savedFilters->findText(tr("Supprimer ce filtre")), QVariant(0), Qt::ItemDataRole::UserRole - 1);
    ui->comboBox_savedFilters->setCurrentIndex(0);
    connect(ui->comboBox_savedFilters, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBox_savedFilters_currentTextChanged()));
}

void OverAid::on_comboBox_savedFilters_currentTextChanged()
{
    QString inOut, moyen, cat, cat2, modeSaisie;

    switch (ui->comboBoxFiltre_inOut->currentIndex()) {
        case 1: inOut = "Debit"; break;
        case 2: inOut = "Credit"; break;
        default: inOut = ""; break;
    }

     switch (ui->comboBoxFiltre_moyen->currentIndex()) {
        case 1: moyen = "Carte bancaire"; break;
        case 2: moyen = "Espèces"; break;
        case 3: moyen = "Chèque"; break;
        case 4: moyen = "Virement"; break;
        case 5: moyen = "Prélèvement"; break;
        default: moyen = ""; break;
      }

    switch (ui->comboBoxFiltre_modeSaisie->currentIndex()) {
        case 1: modeSaisie = "Manuel"; break;
        case 2: modeSaisie = "Abonnement"; break;
        case 3: modeSaisie = "Import en masse"; break;
        default: modeSaisie = ""; break;
     }

    if(ui->pushButtonFiltre_cat->menu()->actions().count() >= 4) {
        bool allCatActionsChecked = true;
        foreach(QAction *catAction, ui->pushButtonFiltre_cat->menu()->actions())
        {
            QString idCat =  QString::number(id_categories.at(ui->pushButtonFiltre_cat->menu()->actions().indexOf(catAction)));
            if(catAction->isChecked()) cat += "\""+idCat+"\";";
            else allCatActionsChecked = false;
            if(ui->pushButtonFiltre_cat->menu()->actions().indexOf(catAction) == ui->pushButtonFiltre_cat->menu()->actions().count()-4) break;
        }
        cat.chop(1);
        if(allCatActionsChecked) cat = "";
    }

    if(ui->pushButtonFiltre_cat2->menu()->actions().count() >= 5) {
        bool allCat2ActionsChecked = true;
        if(ui->pushButtonFiltre_cat2->menu()->actions().count() > 5) {
            foreach(QAction *cat2Action, ui->pushButtonFiltre_cat2->menu()->actions()) {
                if(cat2Action->isChecked()) cat2 += id_sousCategories.at(ui->pushButtonFiltre_cat2->menu()->actions().indexOf(cat2Action))+";";
                else allCat2ActionsChecked = false;
                if(ui->pushButtonFiltre_cat2->menu()->actions().indexOf(cat2Action) == ui->pushButtonFiltre_cat2->menu()->actions().count()-6) break;
            }
        }
        if(ui->pushButtonFiltre_cat2->menu()->actions().at(ui->pushButtonFiltre_cat2->menu()->actions().count()-4)->isChecked()) cat2 += id_sousCategories.last()+";";
        else allCat2ActionsChecked = false;
        if(allCat2ActionsChecked) cat2 = "";
        cat2.chop(1);
        cat2.remove(QRegularExpression("[^\\d\";]")); //Garder que les " ; et les chiffres
    }

    if(ui->comboBox_savedFilters->currentText() == "") {
        ui->comboBox_savedFilters->setItemData(ui->comboBox_savedFilters->findText(tr("Sauvegarder et remplacer ce filtre")), QVariant(0), Qt::ItemDataRole::UserRole - 1);
        ui->comboBox_savedFilters->setItemData(ui->comboBox_savedFilters->findText(tr("Supprimer ce filtre")), QVariant(0), Qt::ItemDataRole::UserRole - 1);
        on_pushButtonFiltre_delete_clicked();
        ui->label_idSavedFilter->clear();
    }
    else if(ui->comboBox_savedFilters->currentIndex() == ui->comboBox_savedFilters->count()-1) {
        QSqlQuery nom("SELECT nom FROM Filtres WHERE id_filtre='"+ui->label_idSavedFilter->text()+"'");
        if(nom.next()) {
            QMessageBox::information(this,tr("Filtre personnalisé supprimé"),tr("Le filtre personnalisé '%1' a bien été supprimé.").arg(nom.value("nom").toString()), tr("Fermer"));
            QSqlQuery remove("DELETE FROM Filtres WHERE id_filtre='"+ui->label_idSavedFilter->text()+"'");
            actu_savedFilters();
            on_pushButtonFiltre_delete_clicked();
        }
    }
    else if(ui->comboBox_savedFilters->currentIndex() == ui->comboBox_savedFilters->count()-2) {
        QStringList currentNames;
        QSqlQuery filtres("SELECT nom FROM Filtres WHERE id_compte='"+QString::number(id_compte)+"'");
        while (filtres.next()) currentNames.append(filtres.value("nom").toString());

        bool done = false;
        while(done == false)
        {
            bool ok;
            QString selectedOptionName = QInputDialog::getText(nullptr, tr("Nouveau filtre"), tr("Veuillez choisir un nom pour ce filtre personnalisé."), QLineEdit::Normal, tr(""), &ok);
            if(ok)
            {
                if(!selectedOptionName.isEmpty())
                {
                    if(currentNames.contains(selectedOptionName)) QMessageBox::warning(this,tr("Erreur"),tr("Un filtre personnalisé porte déjà ce nom."), tr("Fermer"));
                    else {
                        QSqlQuery add("INSERT INTO Filtres (id_compte, nom, type, moyen, categorie, sous_categorie, date_debut, date_fin, description, devise, projet, modeSaisie) "
                                      "VALUES ('"+QString::number(id_compte)+"','"+selectedOptionName+"','"+inOut+"','"+moyen+"','"+cat+"','"+cat2+"','"+ui->dateEditFilter_start->date().toString("yyyyMMdd")+"','"+ui->dateEditFilter_end->date().toString("yyyyMMdd")+"',"
                                      "'"+ui->lineEditFiltre_desc->text().replace("'","''")+"','"+(ui->comboBoxFiltre_devise->currentIndex() == 0 ? "" : ui->comboBoxFiltre_devise->currentText().left(3))+"',"
                                      "'"+ui->lineEditFiltre_projet->text().replace("'","''")+"','"+modeSaisie+"')");

                        actu_savedFilters();
                        ui->comboBox_savedFilters->setCurrentIndex(ui->comboBox_savedFilters->findText(selectedOptionName));
                        QMessageBox::information(this,tr("Filtre personnalisé ajouté"),tr("Le filtre personnalisé '%1' a bien été ajouté.").arg(selectedOptionName), tr("Fermer"));
                        done = true;
                    }
                }
                else QMessageBox::warning(this,tr("Erreur"),tr("Le filtre personnalisé doit avoir un nom valide.").arg(selectedOptionName), tr("Fermer"));
            }
            else
            {
                ui->comboBox_savedFilters->setCurrentIndex(0);
                done = true;
            }
        }
    }
    else if(ui->comboBox_savedFilters->currentIndex() == ui->comboBox_savedFilters->count()-3) {
        QSqlQuery update("UPDATE Filtres SET type='"+inOut+"', moyen='"+moyen+"', categorie='"+cat+"', sous_categorie='"+cat2+"', date_debut='"+ui->dateEditFilter_start->date().toString("yyyyMMdd")+"', date_fin='"+ui->dateEditFilter_end->date().toString("yyyyMMdd")+"', "
                         "description='"+ui->lineEditFiltre_desc->text().replace("'","''")+"', devise='"+(ui->comboBoxFiltre_devise->currentIndex() == 0 ? "" : ui->comboBoxFiltre_devise->currentText().left(3))+"', "
                         "projet='"+ui->lineEditFiltre_projet->text().replace("'","''")+"', modeSaisie='"+modeSaisie+"' "
                         "WHERE id_filtre='"+ui->label_idSavedFilter->text()+"'");
        ui->comboBox_savedFilters->setCurrentIndex(id_savedFilters.indexOf(ui->label_idSavedFilter->text().toInt()));
        QMessageBox::information(this,tr("Filtre personnalisé mis à jour"),tr("Le filtre personnalisé '%1' a bien été mis à jour.").arg(ui->comboBox_savedFilters->currentText()), tr("Fermer"));
    }
    else {
        QSqlQuery filtres("SELECT * FROM Filtres WHERE nom='"+ui->comboBox_savedFilters->currentText()+"' AND id_compte='"+QString::number(id_compte)+"'");
        if(filtres.next()) {
            ui->label_idSavedFilter->setText(filtres.value("id_filtre").toString());

            QString inOutH = filtres.value("type").toString();
            if(inOutH == "Debit") inOut = tr("Débit");
            else if(inOutH == "Credit") inOut = tr("Crédit");
            if(ui->comboBoxFiltre_inOut->findText(inOut) != -1) ui->comboBoxFiltre_inOut->setCurrentText(inOut);
            else ui->comboBoxFiltre_inOut->setCurrentIndex(0);

            QString modeSaisieH = filtres.value("modeSaisie").toString();
            if(modeSaisieH == "Manuel") modeSaisie = tr("Manuel");
            else if(modeSaisieH == "Abonnement") modeSaisie = tr("Abonnement");
            else if(modeSaisieH == "Import en masse") modeSaisie = tr("Import en masse");
            if(ui->comboBoxFiltre_modeSaisie->findText(modeSaisie) != -1) ui->comboBoxFiltre_modeSaisie->setCurrentText(modeSaisie);
            else ui->comboBoxFiltre_modeSaisie->setCurrentIndex(0);

            QString moyenH = filtres.value("moyen").toString();
            if(moyenH == "Carte bancaire") moyen = tr("Carte bancaire");
            else if(moyenH == "Espèces") moyen = tr("Espèces");
            else if(moyenH == "Chèque") moyen = tr("Chèque");
            else if(moyenH == "Virement") moyen = tr("Virement");
            else if(moyenH == "Prélèvement") moyen = tr("Prélèvement");
            if(ui->comboBoxFiltre_moyen->findText(moyen) != -1) ui->comboBoxFiltre_moyen->setCurrentText(moyen);
            else ui->comboBoxFiltre_moyen->setCurrentIndex(0);

            if(filtres.value("devise").toString() != "")
            {
                int id_devise = ui->comboBoxFiltre_devise->findText(filtres.value("devise").toString(),Qt::MatchStartsWith);
                if(id_devise >= 0) ui->comboBoxFiltre_devise->setCurrentIndex(id_devise);
            }
            else ui->comboBoxFiltre_devise->setCurrentIndex(0);

            ui->dateEditFilter_start->setDate(QDate::fromString(filtres.value("date_debut").toString(),"yyyyMMdd"));
            ui->dateEditFilter_end->setDate(QDate::fromString(filtres.value("date_fin").toString(),"yyyyMMdd"));
            ui->lineEditFiltre_desc->setText(filtres.value("description").toString());
            ui->lineEditFiltre_projet->setText(filtres.value("projet").toString());

            if(filtres.value("categorie").toString() != "") {
                ui->pushButtonFiltre_cat->findChild<CustomMenu *>()->activateNone();
                foreach(QString id_cat, filtres.value("categorie").toString().remove("\"").split(";"))
                    if(id_categories.contains(id_cat.toInt())) ui->pushButtonFiltre_cat->findChild<CustomMenu *>()->actions().at(id_categories.indexOf(id_cat.toInt()))->setChecked(true);
            }
            else ui->pushButtonFiltre_cat->findChild<CustomMenu *>()->activateAll();
            previousActionsCatState.clear();
            emit ui->pushButtonFiltre_cat->findChild<CustomMenu *>()->aboutToHide();

            if(filtres.value("sous_categorie").toString() != "") {
                QStringList id_sousCategoriesChoped = id_sousCategories;
                for (int i = 0; i < id_sousCategoriesChoped.size(); ++i)
                    id_sousCategoriesChoped[i] = id_sousCategoriesChoped[i].remove(QRegularExpression("[^\\d\";]")); //Garder que les " ; et les chiffres

                ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()->activateNone();
                foreach(QString id_cat2, filtres.value("sous_categorie").toString().split(";"))
                    if(id_cat2 == "\"\"") ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()->actions().at(ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()->actions().count()-4)->setChecked(true);
                    else if(id_sousCategoriesChoped.contains(id_cat2)) ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()->actions().at(id_sousCategoriesChoped.indexOf(id_cat2))->setChecked(true);
            }
            else ui->pushButtonFiltre_cat2->findChild<CustomMenu *>()->activateAll();
        }
        ui->comboBox_savedFilters->setItemData(ui->comboBox_savedFilters->findText(tr("Sauvegarder et remplacer ce filtre")), QVariant(), Qt::ItemDataRole::UserRole - 1);
        ui->comboBox_savedFilters->setItemData(ui->comboBox_savedFilters->findText(tr("Supprimer ce filtre")), QVariant(), Qt::ItemDataRole::UserRole - 1);
        ui->label_idSavedFilter->setText(filtres.value("id_filtre").toString());
        on_pushButtonFiltre_valide_clicked();
    }
}

void OverAid::actu_filtre()
{
    //Filtre
    QString inOut, moyen, cat, cat2, desc, date, devise, modeSaisie, projet;

    switch (ui->comboBoxFiltre_inOut->currentIndex()) {
        case 1: inOut = "type='Debit'"; break;
        case 2: inOut = "type='Credit'"; break;
        default: inOut = "1+1"; break;
    }

    switch (ui->comboBoxFiltre_modeSaisie->currentIndex()) {
        case 1: modeSaisie = " modeSaisie='Manuel'"; break;
        case 2: modeSaisie = " modeSaisie='Abonnement'"; break;
        case 3: modeSaisie = " modeSaisie='Import en masse'"; break;
        default: modeSaisie = " 1+1"; break;
    }

    switch (ui->comboBoxFiltre_moyen->currentIndex()) {
        case 1: moyen = " moyen='Carte bancaire'"; break;
        case 2: moyen = " moyen='Virement'"; break;
        case 3: moyen = " moyen='Espèces'"; break;
        case 4: moyen = " moyen='Chèque'"; break;
        case 5: moyen = " moyen='Prélèvement'"; break;
        default: moyen = " 1+1"; break;
    }

    if(ui->pushButtonFiltre_cat->menu()->actions().count() >= 4) {
        bool allCatActionsChecked = true;
        foreach(QAction *catAction, ui->pushButtonFiltre_cat->menu()->actions())
        {
            QString idCat =  QString::number(id_categories.at(ui->pushButtonFiltre_cat->menu()->actions().indexOf(catAction)));
            if(catAction->isChecked()) cat += " OR categorie LIKE '%\""+idCat+"\"%'";
            else allCatActionsChecked = false;
            if(ui->pushButtonFiltre_cat->menu()->actions().indexOf(catAction) == ui->pushButtonFiltre_cat->menu()->actions().count()-4) break;
        }
        if(allCatActionsChecked) cat = " 1+1";
        if(cat.startsWith(" OR")) cat = " ("+cat.right(cat.length()-4)+")";
    }

    if(ui->pushButtonFiltre_cat2->menu()->actions().count() >= 5) {
        bool allCat2ActionsChecked = true;
        if(ui->pushButtonFiltre_cat2->menu()->actions().count() > 5) {
            foreach(QAction *cat2Action, ui->pushButtonFiltre_cat2->menu()->actions()) {
                if(cat2Action->isChecked()) cat2 += "OR "+id_sousCategories.at(ui->pushButtonFiltre_cat2->menu()->actions().indexOf(cat2Action));
                else allCat2ActionsChecked = false;
                if(ui->pushButtonFiltre_cat2->menu()->actions().indexOf(cat2Action) == ui->pushButtonFiltre_cat2->menu()->actions().count()-6) break;
            }
        }
        if(ui->pushButtonFiltre_cat2->menu()->actions().at(ui->pushButtonFiltre_cat2->menu()->actions().count()-4)->isChecked()) cat2 += "OR "+id_sousCategories.last();
        else allCat2ActionsChecked = false;
        if(allCat2ActionsChecked) cat2 = " 1+1";
        if(cat2.startsWith("OR")) cat2 = " ("+cat2.right(cat2.length()-3).left(cat2.size()-1)+")";
    }

    if(ui->lineEditFiltre_desc->text().isEmpty()) desc = " 1+1";
    else desc = " (description LIKE '"+ui->lineEditFiltre_desc->text().replace("'","''")+"%' OR description LIKE '%;"+ui->lineEditFiltre_desc->text().replace("'","''")+"%')";

    if(ui->lineEditFiltre_projet->text().isEmpty()) projet = " 1+1";
    else projet = " projet='"+ui->lineEditFiltre_projet->text().replace("'","''")+"'";

    date = " date >= '"+ui->dateEditFilter_start->date().toString("yyyyMMdd")+"' AND date <= '"+ui->dateEditFilter_end->date().toString("yyyyMMdd")+"'";

    if(ui->comboBoxFiltre_devise->currentIndex() == 0) devise = " 1+1";
    else devise = " devise='"+ui->comboBoxFiltre_devise->currentText().left(3)+"'";

    where = inOut+" AND"+moyen+" AND"+cat+" AND"+cat2+" AND"+desc+" AND"+devise+" AND"+modeSaisie+" AND"+projet+" AND id_compte='"+QString::number(id_compte)+"' AND"+date+" ";

    while(where.contains("1+1 AND 1+1")) where.replace("1+1 AND 1+1", "1+1");
}

void OverAid::actu_historique()
{
    qDebug() << "----------";
    QTime timerHisto = QTime::currentTime();
    actu_filtre();
    ui->label_reste->setText(tr("Solde : ")+"-----");
    ui->label_prevision->setText(tr("Solde futur : ")+"-----");

    ui->treeWidgetSummary->clear();
    ui->treeWidgetSummary->setColumnCount(7);
    QStringList headerLabels;
    headerLabels.append(tr("Date"));
    headerLabels.append("");
    headerLabels.append(tr("Débit / Crédit"));
    headerLabels.append(tr("Catégorie"));
    headerLabels.append(tr("Sous-catégorie"));
    headerLabels.append(tr("Description"));
    headerLabels.append(tr("Montant"));
    headerLabels.append(tr("Moyen de paiement"));
    headerLabels.append(tr("Mode de saisie"));
    headerLabels.append(tr("Projet"));
    if(where.startsWith("1+1 AND id_compte")) headerLabels.append(tr("Solde"));
    else headerLabels.append(tr("Total"));
    headerLabels.append("id");
    ui->treeWidgetSummary->setHeaderLabels(headerLabels);

    //Cacher les colonnes masquées
    QSqlQuery hiddenColumns("SELECT hiddenColumns FROM Settings");
    if(hiddenColumns.next())
    {
        foreach(QString i, hiddenColumns.value(0).toString().split(";"))
            ui->treeWidgetSummary->hideColumn(i.toInt());
    }

    //Prévision et reste compte
    double solde = 0, deltaM = 0, deltaY = 0;

    QString deviseCompte, symboleCompte;
    QSqlQuery compteInitial("SELECT montant_initial,devise,Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
    if(compteInitial.next())
    {
        if(where.startsWith("1+1 AND id_compte")) solde = compteInitial.value("montant_initial").toDouble();
        deviseCompte = compteInitial.value("devise").toString();
        symboleCompte = compteInitial.value("symbole").toString();
    }

    QStringList anneeMoisList, anneeList;

    QIcon pdfIcon = QIcon(":/qrc/ressources/image/pdf.png");

    QString columnNameExceptFile = "*";
    QSqlQuery nomDesColonnes("SELECT group_concat(name, ',') AS colonnes FROM pragma_table_info('Transactions') WHERE name <> 'fichier'");
    if(nomDesColonnes.next()) columnNameExceptFile = nomDesColonnes.value("colonnes").toString()+", CASE WHEN fichier IS NULL OR fichier = '' THEN 'Non' ELSE 'Oui' END AS fichier, Devises.symbole";

    QSqlQuery transaction("SELECT "+columnNameExceptFile+" FROM Transactions JOIN Devises ON Transactions.devise = Devises.code WHERE "+where+" ORDER BY date ASC, id_trans ASC");
    qDebug() << transaction.lastQuery();
    while(transaction.next())
    {
        QTreeWidgetItem *trans = new QTreeWidgetItem();

        QDate date = QDate::fromString(transaction.value("date").toString(), "yyyyMMdd");
        trans->setText(0, locale.toString(date, locale.dateFormat(QLocale::ShortFormat).contains("yyyy") ? locale.dateFormat(QLocale::ShortFormat) : locale.dateFormat(QLocale::ShortFormat).replace("yy","yyyy")));

        if(!anneeList.contains(QString::number(date.year())))
        {
            QTreeWidgetItem *annee = new QTreeWidgetItem();
            annee->setText(0, QString::number(date.year()));

            anneeList.append(QString::number(date.year()));
            deltaY = 0;

            ui->treeWidgetSummary->insertTopLevelItem(0, annee);
        }

        QString moisString = locale.toString(date, "MMMM");
        moisString[0] = moisString[0].toUpper();
        if(!anneeMoisList.contains(QString::number(date.year())+moisString))
        {
            QTreeWidgetItem *mois = new QTreeWidgetItem();
            mois->setText(0, moisString);

            anneeMoisList.append(QString::number(date.year())+moisString);
            deltaM = 0;

            ui->treeWidgetSummary->topLevelItem(0)->insertChild(0, mois);
        }

        QString inOut, inOutH = transaction.value("type").toString();
        if(inOutH == "Debit") inOut = tr("Débit");
        else if(inOutH == "Credit") inOut = tr("Crédit");
        trans->setText(2, inOut);

        QString moyen, moyenH = transaction.value("moyen").toString();
        if(moyenH == "Carte bancaire") moyen = tr("Carte bancaire");
        else if(moyenH == "Espèces") moyen = tr("Espèces");
        else if(moyenH == "Chèque") moyen = tr("Chèque");
        else if(moyenH == "Virement") moyen = tr("Virement");
        else if(moyenH == "Prélèvement") moyen = tr("Prélèvement");
        trans->setText(7, moyen);

        //Si multi catégorie
        double montantTotal = 0, montantTotalDeviseCompte = 0;
        if(transaction.value("categorie").toString().contains(";"))
        {
            trans->setText(3, "---");
            trans->setText(4, "---");
            trans->setText(5, "---");

            int countDescStr = 1, countUncategorized = 0, countLine = 0;
            for(int i = 0; i <= transaction.value("categorie").toString().count(";"); i++)
            {
                countLine++;
                QTreeWidgetItem *category = new QTreeWidgetItem();

                QSqlQuery cat("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("categorie").toString().remove("\"").split(";").at(i)+"'");
                if(cat.next()) category->setText(3, cat.value("nom").toString());

                QSqlQuery cat2("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("sous_categorie").toString().remove("\"").split(";").at(i)+"'");
                if(cat2.next()) category->setText(4, cat2.value("nom").toString());

                bool tout = false, categ = true, categ2 = true, desc = true;
                if(where.startsWith("1+1 AND id_compte")) tout = true;
                else
                {
                    if(where.contains("(categorie LIKE") && !where.contains(transaction.value("categorie").toString().split(";").at(i))) categ = false;

                    if(where.contains("(sous_categorie LIKE"))
                    {
                        QString cat2;
                        if(transaction.value("sous_categorie").toString().split(";").at(i) == "") cat2 = "\"\"";
                        else cat2 = transaction.value("sous_categorie").toString().split(";").at(i);

                        if(!where.contains(cat2)) categ2 = false;
                    }

                    if(where.contains("(description LIKE"))
                    {
                        QString oldLen = QString::number(countDescStr);
                        QString len = QString::number(transaction.value("description").toString().split(";").at(i).length());
                        QString whereDesc = where;
                        whereDesc.replace("description","substr(description,"+oldLen+","+len+")");

                        QSqlQuery description("SELECT description FROM Transactions WHERE "+whereDesc+" AND substr(description,"+oldLen+","+len+") = '"+QString(transaction.value("description").toString().split(";").at(i)).replace("'","''")+"'");
                        if(!description.next()) desc = false;
                        countDescStr += transaction.value("description").toString().split(";").at(i).length()+1;
                    }
                }

                if(tout || (categ && categ2 && desc))
                {
                    montantTotal += transaction.value("montant").toString().split(";").at(i).toDouble();
                    montantTotalDeviseCompte += deviseCompte == transaction.value("devise").toString() ? transaction.value("montant").toString().split(";").at(i).toDouble() : transaction.value("montantDeviseCompte").toString().split(";").at(i).toDouble();
                }
                else countUncategorized++;

                double montantLine = transaction.value("montant").toString().split(";").at(i).toDouble();
                category->setText(6, QString::number(montantLine,'f',2).replace('.', ',')+" "+transaction.value("symbole").toString());

                //Couleur multi cat
                if(montantLine == 0) category->setForeground(6, QColor("orange"));
                if((inOutH == "Debit" && montantLine > 0) || (inOutH == "Credit" && montantLine < 0)) category->setForeground(6, Qt::red);
                if((inOutH == "Debit" && montantLine < 0) || (inOutH == "Credit" && montantLine > 0)) category->setForeground(6, Qt::darkGreen);

                category->setText(5, transaction.value("description").toString().split(";").at(i));
                category->setText(11, transaction.value("id_trans").toString());

                trans->addChild(category);
            }

            //Supprimer l'item si aucune sous-transaction ne correspond à l'entiereté des filtres
            int originalChildCount = trans->childCount();
            if (countLine == countUncategorized) for(int i = 0; i < originalChildCount; i++) trans->removeChild(trans->child(0));

            montantTotal = QString::number(montantTotal,'f',2).toDouble();
            montantTotalDeviseCompte = QString::number(montantTotalDeviseCompte,'f',2).toDouble();

            QFont font;
            font.setUnderline(true);
            trans->setFont(6, font);
        }
        else
        {
            QSqlQuery cat("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("categorie").toString().remove("\"")+"'");
            if(cat.next()) trans->setText(3, cat.value("nom").toString());

            QSqlQuery cat2("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("sous_categorie").toString().remove("\"")+"'");
            if(cat2.next()) trans->setText(4, cat2.value("nom").toString());

            montantTotal = transaction.value("montant").toDouble();
            montantTotalDeviseCompte = deviseCompte == transaction.value("devise").toString() ? transaction.value("montant").toDouble() : transaction.value("montantDeviseCompte").toDouble();

            trans->setText(5, transaction.value("description").toString());
        }

        trans->setText(6, QString::number(montantTotal,'f',2).replace('.', ',')+" "+transaction.value("symbole").toString());

        //Couleur cat
        if(montantTotal == 0) trans->setForeground(6, QColor("orange"));
        if((inOutH == "Debit" && montantTotal > 0) || (inOutH == "Credit" && montantTotal < 0)) trans->setForeground(6, Qt::red);
        if((inOutH == "Debit" && montantTotal < 0) || (inOutH == "Credit" && montantTotal > 0)) trans->setForeground(6, Qt::darkGreen);

        if(!(inOutH == "Debit" && moyenH == "Espèces") || !where.startsWith("1+1 AND id_compte"))
        {
            if(inOutH == "Debit") solde -= montantTotalDeviseCompte;
            else solde += montantTotalDeviseCompte;
        }

        if(inOutH == "Debit")
        {
            deltaY -= montantTotalDeviseCompte;
            deltaM -= montantTotalDeviseCompte;
        }
        else
        {
            deltaY += montantTotalDeviseCompte;
            deltaM += montantTotalDeviseCompte;
        }

        if(date <= QDate::currentDate()) ui->label_reste->setText(tr("Solde : ")+QString::number(solde,'f',2).replace(".",",")+" "+symboleCompte);
        ui->label_prevision->setText(tr("Solde futur : ")+QString::number(solde,'f',2).replace(".",",")+" "+symboleCompte);

        QString modeSaisie, modeSaisieH = transaction.value("modeSaisie").toString();
        if(modeSaisieH == "Manuel") modeSaisie = tr("Manuel");
        else if(modeSaisieH == "Abonnement") modeSaisie = tr("Abonnement");
        else if(modeSaisieH == "Import en masse") modeSaisie = tr("Import en masse");
        trans->setText(8, modeSaisie);

        trans->setText(9, transaction.value("projet").toString());

        trans->setText(10, QString::number(solde,'f',2).replace(".",",")+" "+symboleCompte);
        trans->setForeground(10, Qt::gray);

        trans->setText(11, transaction.value("id_trans").toString());

        if((!transaction.value("fichier").toByteArray().isEmpty() && transaction.value("fichier").toString() != "Non") || transaction.value("fichier").toString() == "Oui") trans->setIcon(1, pdfIcon);

        //--------
        QFont bold;
        bold.setBold(true);

        //Couleur année
        ui->treeWidgetSummary->topLevelItem(0)->setText(1, QString::number(deltaY,'f',2).replace('.', ',')+" "+symboleCompte);
        if(deltaY == 0) ui->treeWidgetSummary->topLevelItem(0)->setForeground(1, QColor("orange"));
        else if(deltaY < 0) ui->treeWidgetSummary->topLevelItem(0)->setForeground(1, Qt::red);
        else ui->treeWidgetSummary->topLevelItem(0)->setForeground(1, Qt::darkGreen);
        ui->treeWidgetSummary->topLevelItem(0)->setFont(1, bold);

        //Couleur mois
        ui->treeWidgetSummary->topLevelItem(0)->child(0)->setText(1, QString::number(deltaM,'f',2).replace('.', ',')+" "+symboleCompte);
        if(deltaM == 0) ui->treeWidgetSummary->topLevelItem(0)->child(0)->setForeground(1, QColor("orange"));
        else if(deltaM < 0) ui->treeWidgetSummary->topLevelItem(0)->child(0)->setForeground(1, Qt::red);
        else ui->treeWidgetSummary->topLevelItem(0)->child(0)->setForeground(1, Qt::darkGreen);

        ui->treeWidgetSummary->topLevelItem(0)->child(0)->insertChild(0, trans);
        trans->setExpanded(true);
    }

    //Nettoyer les transaction multi-catégorie nulle
    QTime timerCleaning = QTime::currentTime();
    for (int yearIndex = 0; yearIndex < ui->treeWidgetSummary->topLevelItemCount(); yearIndex++) {
        QTreeWidgetItem *yearItem = ui->treeWidgetSummary->topLevelItem(yearIndex);
        for (int monthIndex = 0; monthIndex < yearItem->childCount(); monthIndex++) {
            QTreeWidgetItem *monthItem = yearItem->child(monthIndex);
            for (int i = 0; i < monthItem->childCount(); ++i) {
                if(monthItem->child(i)->text(3) == "---" && monthItem->child(i)->childCount() == 0)
                {
                    delete monthItem->child(i);
                    --i;
                }
            }

            if (monthItem->childCount() == 0) {
                delete monthItem;
                --monthIndex;
            }
        }
        if (yearItem->childCount() == 0) {
            delete yearItem;
            --yearIndex;
        }
    }
    qDebug() << "cleaning time : " << timerCleaning.msecsTo(QTime::currentTime()) << "msec.";

    QTreeWidgetItem *emptyTrans = new QTreeWidgetItem();
    ui->treeWidgetSummary->addTopLevelItem(emptyTrans);

    //Rouvrir les arbres ouverts comme avant le refresh
    QTime timerRemember = QTime::currentTime();
    if(!rememberOpenedYears.isEmpty() && !rememberOpenedMonth.isEmpty() && !rememberOpenedItems.isEmpty())
    {
        for(int i = 0; i < ui->treeWidgetSummary->topLevelItemCount(); i++)
        {
            int year = rememberOpenedYearsID.indexOf(ui->treeWidgetSummary->topLevelItem(i)->text(0));
            if(year != -1) ui->treeWidgetSummary->topLevelItem(i)->setExpanded(rememberOpenedYears.at(year) == "true" ? true : false);
            for(int j = 0; j < ui->treeWidgetSummary->topLevelItem(i)->childCount(); j++)
            {
                int month = rememberOpenedMonthID.indexOf(ui->treeWidgetSummary->topLevelItem(i)->text(0)+ui->treeWidgetSummary->topLevelItem(i)->child(j)->text(0));
                if(month != -1) ui->treeWidgetSummary->topLevelItem(i)->child(j)->setExpanded(rememberOpenedMonth.at(month) == "true" ? true : false);
                for(int k = 0; k < ui->treeWidgetSummary->topLevelItem(i)->child(j)->childCount(); k++)
                {
                    int item = rememberOpenedItemsID.indexOf(ui->treeWidgetSummary->topLevelItem(i)->child(j)->child(k)->text(11));
                    if(item != -1) ui->treeWidgetSummary->topLevelItem(i)->child(j)->child(k)->setExpanded(rememberOpenedItems.at(item) == "true" ? true : false);
                }
            }
        }
        rememberOpenedYearsID.clear();
        rememberOpenedMonthID.clear();
        rememberOpenedItemsID.clear();
        rememberOpenedYears.clear();
        rememberOpenedMonth.clear();
        rememberOpenedItems.clear();
    }

    //Ouvrir l'année en cours ou le dernier arbre
    else
    {
        if(ui->treeWidgetSummary->topLevelItemCount() > 0) //S'il existe des transactions
        {
            QTreeWidgetItem *year;
            QList<QTreeWidgetItem *> currentYearItems = ui->treeWidgetSummary->findItems(QString::number(QDate::currentDate().year()),Qt::MatchExactly);
            if(!currentYearItems.isEmpty()) year = currentYearItems.at(0);
            else year = ui->treeWidgetSummary->topLevelItem(0);

            ui->treeWidgetSummary->expandItem(year); //Etendre l'année
            ui->treeWidgetSummary->setCurrentItem(year); //Selectionner l'année par défaut

            for(int i = 0; i < year->childCount(); i++)
                ui->treeWidgetSummary->expandItem(year->child(i)); //Etendre les mois de l'année
        }
    }

    for(int i = 0; i < ui->treeWidgetSummary->columnCount(); i++)
    {
        ui->treeWidgetSummary->resizeColumnToContents(i);
        if(i != 5) ui->treeWidgetSummary->setColumnWidth(i, ui->treeWidgetSummary->columnWidth(i)*1.2); //Redimensionner *1.20 toutes les colonnes au texte (sauf colonne 'Description')
    }

    //Positionner la barre verticale comme avant le refresh
    if(rememberPosition != 0)
    {
        ui->treeWidgetSummary->verticalScrollBar()->setSliderPosition(rememberPosition);
        rememberPosition = 0;
    }
    qDebug() << "remembering time : " << timerRemember.msecsTo(QTime::currentTime()) << "msec.";

    //Changer les labels si filtres
    if(!where.startsWith("1+1 AND id_compte"))
    {
        ui->label_reste->setText(tr("Solde : ")+"-----");
        ui->label_prevision->setText(tr("Solde futur : ")+"-----");
        ui->label_reste->setEnabled(false);
        ui->label_prevision->setEnabled(false);
    }
    else
    {
        ui->label_reste->setEnabled(true);
        ui->label_prevision->setEnabled(true);
    }

    qDebug() << "refreshing time : " << timerHisto.msecsTo(QTime::currentTime()) << "msec.";
}

void OverAid::tout_etendre()
{
    ui->treeWidgetSummary->expandAll();
    for(int i = 0; i < ui->treeWidgetSummary->columnCount(); i++)
    {
        ui->treeWidgetSummary->resizeColumnToContents(i);
        if(i != 5) ui->treeWidgetSummary->setColumnWidth(i, ui->treeWidgetSummary->columnWidth(i)*1.2);
    }
}

void OverAid::tout_retrecir()
{
    ui->treeWidgetSummary->expandAll();
    for(int i = 0; i < ui->treeWidgetSummary->columnCount(); i++)
    {
        ui->treeWidgetSummary->resizeColumnToContents(i);
        if(i != 5) ui->treeWidgetSummary->setColumnWidth(i, ui->treeWidgetSummary->columnWidth(i)*1.2);
    }
    ui->treeWidgetSummary->collapseAll();
}

void OverAid::copy_case()
{
    QApplication::clipboard()->setText(ui->treeWidgetSummary->currentItem()->text(ui->treeWidgetSummary->currentColumn()));
}

void OverAid::copy_line()
{
    QString line;
    for(int i = 0; i < ui->treeWidgetSummary->columnCount()-1; i++)
        line += ui->treeWidgetSummary->currentItem()->text(i) + ";";
    if(line.endsWith(";")) line.chop(1);

    QApplication::clipboard()->setText(line);
}

void OverAid::transToSub(bool avecPDF)
{
    if(!ui->treeWidgetSummary->currentItem()->text(11).isEmpty())
    {
        QSqlQuery abo("SELECT * FROM Transactions WHERE id_trans='"+ui->treeWidgetSummary->currentItem()->text(11)+"'");
        while (abo.next())
        {
            QSqlQuery ajouter;
            ajouter.prepare("INSERT INTO Abonnements (id_compte, type, moyen, categorie, sous_categorie, description, montant, detail_montant, fichier, renouvellement, dernier, devise, montantDeviseCompte) "
                              "VALUES ('"+abo.value("id_compte").toString()+"', '"+abo.value("type").toString().replace("'","''")+"', '"+abo.value("moyen").toString().replace("'","''")+"', '"+abo.value("categorie").toString()+"', "
                              "'"+abo.value("sous_categorie").toString()+"', '"+abo.value("description").toString().replace("'","''")+"', '"+abo.value("montant").toString()+"', '"+abo.value("detail_montant").toString()+"',:pdf, "
                              "'"+abo.value("date").toString().right(2)+"', '"+QDate::currentDate().toString("yyyyMM")+"', '"+abo.value("devise").toString()+"', '"+abo.value("montantDeviseCompte").toString()+"');");
            if(abo.value("fichier").toByteArray().isEmpty() || !avecPDF)
            {
                ajouter.bindValue(":pdf","");
                ajouter.exec();
            }
            else
            {
                ajouter.bindValue(":pdf",abo.value("fichier").toByteArray());
                ajouter.exec();
            }
        }
        QMessageBox::information(this, tr("Abonnement ajouté"), tr("La transaction a bien été convertie en abonnement."), tr("Fermer"));
        if(ui->treeWidgetSummary->indexOfTopLevelItem(ui->treeWidgetSummary->currentItem()->parent()->parent()) != 0 &&
           ui->treeWidgetSummary->currentItem()->parent()->parent()->indexOfChild(ui->treeWidgetSummary->currentItem()->parent()) != 0) actu(false, false);
    }
    else QMessageBox::warning(this, tr("Erreur"), tr("Il ne s'agit pas d'une transaction."), tr("Fermer"));
}

void OverAid::modify_trans()
{
    if(!ui->treeWidgetSummary->currentItem()->text(11).isEmpty())
    {
        TransForm *modif = new TransForm(nullptr, "Modifier");
        modif->setWindowModality(Qt::ApplicationModal);
        modif->show();
        connect(modif, &TransForm::actu, this, [=](){actu(true, false);});
        modif->id_categories = id_categories;
        modif->id_compte = id_compte;
        modif->actu_devise();
        modif->actu_projet();

        QSqlQuery transaction("SELECT * FROM Transactions WHERE id_trans='"+ui->treeWidgetSummary->currentItem()->text(11)+"'");
        if(transaction.next())
        {
            modif->ui->dateEditAdd->setDate(locale.toDate(transaction.value("date").toString(),"yyyyMMdd"));

            if(transaction.value("type").toString() == "Debit") modif->on_pushButton_out_clicked();
            else modif->on_pushButton_in_clicked();

            QString moyenH = transaction.value("moyen").toString();
            if(moyenH == "Carte bancaire") modif->ui->comboBox_moyen->setCurrentText(tr("Carte bancaire"));
            else if(moyenH == "Espèces") modif->ui->comboBox_moyen->setCurrentText(tr("Espèces"));
            else if(moyenH == "Chèque") modif->ui->comboBox_moyen->setCurrentText(tr("Chèque"));
            else if(moyenH == "Virement") modif->ui->comboBox_moyen->setCurrentText(tr("Virement"));
            else if(moyenH == "Prélèvement") modif->ui->comboBox_moyen->setCurrentText(tr("Prélèvement"));

            modif->ui->label_id->setText(transaction.value("id_trans").toString());

            for(int i = 0; i <= transaction.value("categorie").toString().count(";"); i++)
            {
                lineForm *newTab = new lineForm();
                modif->ui->tabWidget_Line->insertTab(modif->ui->tabWidget_Line->count()-1, newTab, tr("Ligne ") + QString::number(modif->ui->tabWidget_Line->count()));
                connect(newTab, SIGNAL(OverAid_amountChanged()), modif, SLOT(totalAmount()));
                newTab->id_categories = id_categories;
                newTab->id_compte = id_compte;
                newTab->actu_categorie();
                newTab->actu_sousCategorie();
                newTab->actu_desc();
                newTab->ui->toolButton->hide();

                if(!transaction.value("montantDeviseCompte").toString().isEmpty()) newTab->ui->doubleSpinBoxAdd_deviseCompte->setValue(transaction.value("montantDeviseCompte").toString().split(";").at(i).toDouble());

                if(transaction.value("detail_montant").toString().split(";").at(i).isEmpty())
                {
                    newTab->ui->radioButtonFixe->setChecked(true);
                    newTab->on_radioButtonFixe_clicked();

                    newTab->ui->doubleSpinBoxAdd_fixe->setValue(transaction.value("montant").toString().split(";").at(i).toDouble());
                }
                else
                {
                    newTab->ui->radioButtonOperation->setChecked(true);
                    newTab->on_radioButtonOperation_clicked();

                    newTab->ui->lineEditAdd_Operation->setText(transaction.value("detail_montant").toString().split(";").at(i));
                }

                newTab->ui->comboBoxAdd_Cat->setCurrentIndex(id_categories.indexOf(transaction.value("categorie").toString().remove("\"").split(";").at(i).toInt()));
                QString id = transaction.value("sous_categorie").toString().remove("\"").split(";").at(i);
                newTab->set_sousCategorie(&id);

                newTab->ui->lineEditAdd_description->setText(transaction.value("description").toString().split(";").at(i));
            }
            modif->ui->tabWidget_Line->setCurrentIndex(0);
            modif->on_tabWidget_Line_currentChanged(0);
            if(modif->ui->comboBox_devise->currentText().left(3) != transaction.value("devise").toString()) modif->ui->comboBox_devise->setCurrentIndex(modif->ui->comboBox_devise->findText(transaction.value("devise").toString(),Qt::MatchStartsWith));
            else modif->on_comboBox_devise_currentTextChanged(modif->ui->comboBox_devise->currentText());

            modif->ui->lineEdit_projet->setText(transaction.value("projet").toString());

            if(!transaction.value("fichier").toString().isEmpty())
            {
                //Enregistrement temporaire du pdf
                QByteArray pdf = transaction.value("fichier").toByteArray();

                if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp").exists()) QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp");
                QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp/Document.pdf");

                if (file.open(QFile::WriteOnly))
                {
                    file.write(pdf);
                    file.close();
                }

                modif->ui->lineEditPDF->setText(file.fileName());
            }
        }
    }
    else QMessageBox::warning(this, tr("Erreur"), tr("Il ne s'agit pas d'une transaction."), tr("Fermer"));
}

void OverAid::duplicate()
{
    if(!ui->treeWidgetSummary->currentItem()->text(11).isEmpty())
    {
        QSqlQuery transaction("SELECT * FROM Transactions WHERE id_trans='"+ui->treeWidgetSummary->currentItem()->text(11)+"'");
        if(transaction.next())
        {
            trans->ui->dateEditAdd->setDate(locale.toDate(transaction.value("date").toString(),"yyyyMMdd"));

            if(transaction.value("type").toString() == "Debit") trans->on_pushButton_out_clicked();
            else trans->on_pushButton_in_clicked();

            QString moyenH = transaction.value("moyen").toString();
            if(moyenH == "Carte bancaire") trans->ui->comboBox_moyen->setCurrentText(tr("Carte bancaire"));
            else if(moyenH == "Espèces") trans->ui->comboBox_moyen->setCurrentText(tr("Espèces"));
            else if(moyenH == "Chèque") trans->ui->comboBox_moyen->setCurrentText(tr("Chèque"));
            else if(moyenH == "Virement") trans->ui->comboBox_moyen->setCurrentText(tr("Virement"));
            else if(moyenH == "Prélèvement") trans->ui->comboBox_moyen->setCurrentText(tr("Prélèvement"));

            for(int i = 0; i <= transaction.value("categorie").toString().count(";"); i++)
            {
                lineForm *newTab;
                if(i == 0)
                {
                    int count = trans->ui->tabWidget_Line->tabBar()->count();
                    for(int i = 0; i <= count; i++)
                    {
                        trans->on_tabWidget_Line_currentChanged(0);
                        trans->on_tabWidget_Line_tabCloseRequested(0);
                    }
                    newTab = trans->ui->tabWidget_Line->findChild<QStackedWidget*>()->findChildren<lineForm*>().at(0);
                }
                else
                {
                    newTab = new lineForm();
                    trans->ui->tabWidget_Line->insertTab(trans->ui->tabWidget_Line->count()-1, newTab, tr("Ligne ") + QString::number(trans->ui->tabWidget_Line->count()));
                    connect(newTab, SIGNAL(OverAid_amountChanged()), trans, SLOT(totalAmount()));
                    newTab->id_categories = id_categories;
                    newTab->id_compte = id_compte;
                    newTab->actu_categorie();
                    newTab->actu_sousCategorie();
                    newTab->actu_desc();
                }
                newTab->ui->toolButton->hide();

                if(!transaction.value("montantDeviseCompte").toString().isEmpty()) newTab->ui->doubleSpinBoxAdd_deviseCompte->setValue(transaction.value("montantDeviseCompte").toString().split(";").at(i).toDouble());

                if(transaction.value("detail_montant").toString().split(";").at(i).isEmpty())
                {
                    newTab->ui->radioButtonFixe->setChecked(true);
                    newTab->on_radioButtonFixe_clicked();

                    newTab->ui->doubleSpinBoxAdd_fixe->setValue(transaction.value("montant").toString().split(";").at(i).toDouble());
                }
                else
                {
                    newTab->ui->radioButtonOperation->setChecked(true);
                    newTab->on_radioButtonOperation_clicked();

                    newTab->ui->lineEditAdd_Operation->setText(transaction.value("detail_montant").toString().split(";").at(i));
                }

                newTab->ui->comboBoxAdd_Cat->setCurrentIndex(id_categories.indexOf(transaction.value("categorie").toString().remove("\"").split(";").at(i).toInt()));
                QString id = transaction.value("sous_categorie").toString().remove("\"").split(";").at(i);
                newTab->set_sousCategorie(&id);

                newTab->ui->lineEditAdd_description->setText(transaction.value("description").toString().split(";").at(i));
            }
            trans->ui->tabWidget_Line->setCurrentIndex(0);
            if(trans->ui->comboBox_devise->currentText().left(3) != transaction.value("devise").toString()) trans->ui->comboBox_devise->setCurrentIndex(trans->ui->comboBox_devise->findText(transaction.value("devise").toString(),Qt::MatchStartsWith));
            else trans->on_comboBox_devise_currentTextChanged(trans->ui->comboBox_devise->currentText());

            trans->ui->lineEdit_projet->setText(transaction.value("projet").toString());
        }
    }
    else QMessageBox::warning(this, tr("Erreur"), tr("Il ne s'agit pas d'une transaction."), tr("Fermer"));
}

void OverAid::delete_trans()
{
    if(!ui->treeWidgetSummary->currentItem()->text(11).isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Supprimer la transaction"));
        if(ui->treeWidgetSummary->currentItem()->parent()->parent()->parent())
            msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer la multi-transaction du %1 ?").arg(locale.toString(locale.toDate(ui->treeWidgetSummary->currentItem()->parent()->text(0),locale.ShortFormat),locale.LongFormat)));
        else msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer la transaction '%1' du %2 ?").arg(ui->treeWidgetSummary->currentItem()->text(5),locale.toString(locale.toDate(ui->treeWidgetSummary->currentItem()->text(0),locale.ShortFormat),locale.LongFormat)));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setStyleSheet("QLabel{min-width: 350px;}");
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok)
        {
            QSqlQuery remove("DELETE FROM Transactions WHERE id_trans='"+ui->treeWidgetSummary->currentItem()->text(11)+"'");
            actu(true, false);
        }
    }
    else QMessageBox::warning(this, tr("Erreur"), tr("Il ne s'agit pas d'une transaction."), tr("Fermer"));
}

void OverAid::on_treeWidgetSummary_itemClicked(QTreeWidgetItem *item, int column)
{
    //Montant - Montant devise compte - Opération
    if(column == 6 && item->parent())
    {
        QString symboleCompte;
        QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
        if(devise.next()) symboleCompte = devise.value("symbole").toString();

        QSqlQuery transaction("SELECT montant,devise,detail_montant,montantDeviseCompte,Devises.symbole FROM Transactions JOIN Devises ON Devises.code=Transactions.devise WHERE id_trans='"+item->text(11)+"'");
        if(transaction.next())
        {
            QString montant, detailMontant, montantDeviseCompte;
            //S'il s'agit d'une multi-ligne
            if(item->text(2) == "")
            {
                int i = item->parent()->indexOfChild(item);
                montant = transaction.value("montant").toString().split(";").at(i) + " " + transaction.value("symbole").toString();
                detailMontant = transaction.value("detail_montant").toString().split(";").at(i);
                if(transaction.value("montantDeviseCompte").toString().contains(";")) montantDeviseCompte = transaction.value("montantDeviseCompte").toString().split(";").at(i) + " " + symboleCompte;
            }
            //S'il ne s'agit pas d'une multi-ligne et que ce n'est pas non plus l'en-tête d'une multi-ligne
            else if(item->text(3) != "---")
            {
                montant = transaction.value("montant").toString() + " " + transaction.value("symbole").toString();
                detailMontant = transaction.value("detail_montant").toString();
                if(!transaction.value("montantDeviseCompte").toString().isEmpty()) montantDeviseCompte = transaction.value("montantDeviseCompte").toString() + " " + symboleCompte;
            }
            //S'il s'agit d'une en-tête d'une multi-ligne
            else
            {
                for (int i = 0; i < item->childCount(); i++) {
                    montant = QString::number(montant.toDouble() + transaction.value("montant").toString().split(";").at(i).toDouble(), 'f', 2);
                    if(transaction.value("montantDeviseCompte").toString().contains(";")) montantDeviseCompte = QString::number(montantDeviseCompte.toDouble() + transaction.value("montantDeviseCompte").toString().split(";").at(i).toDouble());
                }
                montant += " " + transaction.value("symbole").toString();
                if(!montantDeviseCompte.isEmpty()) montantDeviseCompte += " " + symboleCompte;
            }

            montant = QString::number(montant.split(" ").at(0).toDouble(),'f',2).replace(".",",")+ " " + montant.split(" ").at(1);
            if(!montantDeviseCompte.isEmpty()) montantDeviseCompte = QString::number(montantDeviseCompte.split(" ").at(0).toDouble(),'f',2).replace(".",",")+ " " + montantDeviseCompte.split(" ").at(1);

            if (item->text(6) == montant) item->setText(6, (!detailMontant.isEmpty()) ? detailMontant : ((!montantDeviseCompte.isEmpty()) ? montantDeviseCompte : montant));
            else if (item->text(6) == detailMontant) item->setText(6, (!montantDeviseCompte.isEmpty()) ? montantDeviseCompte : (montant));
            else if (item->text(6) == montantDeviseCompte) item->setText(6, montant);

            ui->treeWidgetSummary->resizeColumnToContents(6);
            ui->treeWidgetSummary->setColumnWidth(6, ui->treeWidgetSummary->columnWidth(6)*1.2);
        }
    }

    //Ouvrir le PDF
    if(column == 1 && !item->icon(1).isNull())
    {
        QSqlQuery transaction("SELECT fichier FROM Transactions WHERE id_trans='"+item->text(11)+"'");
        if(transaction.next())
        {
            QByteArray pdf = transaction.value("fichier").toByteArray();

            if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp").exists()) QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp");
            QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp/Document.pdf");

            if (file.open(QFile::WriteOnly))
            {
                file.write(pdf);
                file.close();
                QDesktopServices::openUrl(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/temp/Document.pdf"));
            }
        }
    }
}

void OverAid::showAllColumns()
{
    for(int i = 0; i < ui->treeWidgetSummary->columnCount()-1; i++)
    {
        ui->treeWidgetSummary->showColumn(i);
        ui->treeWidgetSummary->resizeColumnToContents(i);
        if(i != 5) ui->treeWidgetSummary->setColumnWidth(i, ui->treeWidgetSummary->columnWidth(i)*1.2); //Redimensionner *1.20 toutes les colonnes au texte (sauf colonne 'Description')
    }
    QSqlQuery showColumns("UPDATE Settings SET hiddenColumns='11'");
}

void OverAid::hideThisColumn()
{
    if(ui->treeWidgetSummary->currentColumn() > 0) ui->treeWidgetSummary->hideColumn(ui->treeWidgetSummary->currentColumn());

    QString hiddenColumns;
    for(int i = 0; i < ui->treeWidgetSummary->columnCount(); i++)
        if(ui->treeWidgetSummary->isColumnHidden(i)) hiddenColumns += QString::number(i)+";";
    if(hiddenColumns.size() > 1) hiddenColumns.chop(1);

    QSqlQuery hideColumns("UPDATE Settings SET hiddenColumns='"+hiddenColumns+"'");
}

void OverAid::on_groupBox_filtre_toggled(bool arg1)
{
    //Masquer le groupBox des filtres (avec animation)
    QPropertyAnimation *group = new QPropertyAnimation(ui->groupBox_filtre, "maximumHeight");
    group->setDuration(1000);
    group->setStartValue(ui->groupBox_filtre->maximumHeight());
    group->setEndValue(arg1 ? 110 : 20);

    group->start();

    QSqlQuery settings("UPDATE Settings SET showGroupFilter='"+QString(arg1 ? "true" : "false")+"'");
}
