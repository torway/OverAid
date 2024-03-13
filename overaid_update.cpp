#include "overaid.h"
#include "ui_overaid.h"


void OverAid::update()
{
    //Mise à jour
    QTime time = QTime::currentTime();

    QSqlQuery settings("CREATE TABLE \"Settings\" (\"version\" TEXT NOT NULL DEFAULT '2.0', \"language\" TEXT NOT NULL DEFAULT 'Français', \"defaultAccount\" TEXT NOT NULL DEFAULT '1', \"showGroupFilter\" TEXT NOT NULL DEFAULT 'true', \"showDebugWindow\" TEXT NOT NULL DEFAULT 'false', \"geometry\" TEXT NOT NULL DEFAULT '0;0;0;0');");
    if(!settings.executedQuery().isEmpty())
    {
        QSqlQuery settings("INSERT INTO Settings (version) VALUES (2.0)");

        if(QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv").exists()) QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv").remove();

        //Comptes actifs
        QSqlQuery update("ALTER TABLE Comptes ADD active TEXT NOT NULL DEFAULT 'true'");
        if(!update.executedQuery().isEmpty()) actu_compte();

        //Mettre les id_cat entre guillemets
        QSqlQuery guillemets("SELECT * FROM Transactions WHERE categorie NOT LIKE '%\"%' OR sous_categorie NOT LIKE '%\"%'");
        while (guillemets.next()) {
            QString concatenatedCat, concatenatedCat2;

            for (const QString &id : guillemets.value("categorie").toString().split(';')) {
                if(!id.contains("\"")) concatenatedCat += "\"" + id + "\";";
                else concatenatedCat += id + ";";
            }
            concatenatedCat.chop(1);

            for (const QString &id : guillemets.value("sous_categorie").toString().split(';')) {
                if(!id.contains("\"")) concatenatedCat2 += "\"" + id + "\";";
                else concatenatedCat2 += id + ";";
            }
            concatenatedCat2.chop(1);

            QSqlQuery updateGuillemets("UPDATE Transactions SET categorie='"+concatenatedCat+"', sous_categorie='"+concatenatedCat2+"' WHERE id_trans='"+guillemets.value("id_trans").toString()+"'");
        }

        QSqlQuery filtre("CREATE TABLE \"Filtres\" (\"id_filtre\" INTEGER NOT NULL, \"id_compte\" INTEGER NOT NULL, \"nom\" TEXT NOT NULL, \"type\" TEXT NOT NULL, \"moyen\" TEXT NOT NULL, \"categorie\" TEXT NOT NULL, \"sous_categorie\" TEXT NOT NULL, \"date_debut\" TEXT NOT NULL, \"date_fin\" TEXT NOT NULL, \"description\" TEXT NOT NULL, \"devise\" TEXT NOT NULL, PRIMARY KEY(\"id_filtre\" AUTOINCREMENT));");

        //Devises
        QSqlQuery devise("CREATE TABLE \"Devises\" (\"id_currency\" INTEGER NOT NULL, \"code\" TEXT NOT NULL, \"nom\" TEXT NOT NULL, \"symbole\" TEXT NOT NULL, PRIMARY KEY(\"id_currency\" AUTOINCREMENT));");
        if(!devise.executedQuery().isEmpty())
        {
            foreach (QString currency, devisesList) QSqlQuery currencyAdd("INSERT INTO Devises (code,nom,symbole) VALUES ('"+currency.split("-").at(0)+"','"+currency.split("-").at(1)+"','"+currency.split("-").at(2)+"')");
        }

        QSqlQuery deviseCompte("ALTER TABLE Comptes ADD devise TEXT NOT NULL DEFAULT 'NaN'");
        QSqlQuery deviseAbo("ALTER TABLE Abonnements ADD devise TEXT NOT NULL DEFAULT 'NaN'");
        QSqlQuery deviseTrans("ALTER TABLE Transactions ADD devise TEXT NOT NULL DEFAULT 'NaN'");
        if(!deviseCompte.executedQuery().isEmpty() || !deviseTrans.executedQuery().isEmpty() || !deviseAbo.executedQuery().isEmpty())
        {
            QStringList allCurrencies;
            QSqlQuery devises("SELECT * FROM Devises");
            while(devises.next()) allCurrencies.append(devises.value("code").toString()+" : "+devises.value("nom").toString()+" ("+devises.value("symbole").toString()+")");

            QSqlQuery comptes("SELECT * FROM Comptes WHERE devise='NaN'");
            while (comptes.next())
            {
                bool done = false;
                while(done == false)
                {
                    bool ok;
                    QString selectedOption = QInputDialog::getItem(nullptr, "Devises", "Veuillez choisir une devise pour le compte '"+comptes.value("nom").toString()+"' :", allCurrencies, 0, false, &ok);
                    if(ok && !selectedOption.isEmpty())
                    {
                        done = true;
                        QSqlQuery updateDeviseCompte("UPDATE Comptes SET devise='"+selectedOption.left(3)+"' WHERE id_compte='"+comptes.value("id_compte").toString()+"'");
                        QSqlQuery updateDeviseTrans("UPDATE Transactions SET devise='"+selectedOption.left(3)+"' WHERE id_compte='"+comptes.value("id_compte").toString()+"'");
                        QSqlQuery updateDeviseAbo("UPDATE Abonnements SET devise='"+selectedOption.left(3)+"' WHERE id_compte='"+comptes.value("id_compte").toString()+"'");
                    }
                }
            }
        }
        QSqlQuery montantDeviseCompteTrans("ALTER TABLE Transactions ADD montantDeviseCompte REAL NOT NULL DEFAULT ''");
        QSqlQuery montantDeviseCompteAbo("ALTER TABLE Abonnements ADD montantDeviseCompte REAL NOT NULL DEFAULT ''");

        // Vérifier l'ordre actuel des colonnes
        QSqlQuery newComptes("PRAGMA table_info('Comptes')");
        QString newComptes_colonnesActuelles;
        while (newComptes.next()) {
            QString nomColonne = newComptes.value("name").toString();
            newComptes_colonnesActuelles += nomColonne + ", ";
        }
        newComptes_colonnesActuelles.chop(2);
        QString newComptes_ordreSouhaite = "id_compte, nom, montant_initial, devise, active";
        if (newComptes_colonnesActuelles != newComptes_ordreSouhaite) {
            qDebug() << "newComptes";
            QSqlQuery nouvelleTable("CREATE TABLE \"new_Comptes\" (\"id_compte\" INTEGER NOT NULL, \"nom\" TEXT NOT NULL, \"montant_initial\" DOUBLE NOT NULL, \"devise\" TEXT NOT NULL, \"active\" TEXT NOT NULL, PRIMARY KEY(\"id_compte\" AUTOINCREMENT));");
            QSqlQuery transfertData("INSERT INTO new_Comptes ("+newComptes_ordreSouhaite+") SELECT "+newComptes_ordreSouhaite+" FROM Comptes");
            overaidDatabase.close();
            QSqlDatabase::removeDatabase("QSQLITE");
            overaidDatabase = QSqlDatabase();
            overaidDatabase = QSqlDatabase::addDatabase("QSQLITE");
            overaidDatabase.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db");
            overaidDatabase.open();
            QSqlQuery supprimerAncienneTable("DROP TABLE Comptes");
            QSqlQuery renommerTable("ALTER TABLE new_Comptes RENAME TO Comptes");
        }

        QSqlQuery newTransactions("PRAGMA table_info('Transactions')");
        QString newTransactions_colonnesActuelles;
        while (newTransactions.next()) {
            QString nomColonne = newTransactions.value("name").toString();
            newTransactions_colonnesActuelles += nomColonne + ", ";
        }
        newTransactions_colonnesActuelles.chop(2);
        QString newTransactions_ordreSouhaite = "id_trans, id_compte, date, type, moyen, categorie, sous_categorie, description, montant, devise, montantDeviseCompte, detail_montant, fichier";
        if (newTransactions_colonnesActuelles != newTransactions_ordreSouhaite) {
            qDebug() << "newTransactions";
            QSqlQuery nouvelleTable("CREATE TABLE \"new_Transactions\" (\"id_trans\" INTEGER NOT NULL, \"id_compte\" TEXT NOT NULL,\"date\" TEXT NOT NULL,\"type\" TEXT NOT NULL,\"moyen\" TEXT NOT NULL,\"categorie\" TEXT NOT NULL,\"sous_categorie\" TEXT NOT NULL,\"description\" TEXT NOT NULL,\"montant\" DOUBLE NOT NULL,\"devise\" TEXT NOT NULL,\"montantDeviseCompte\" DOUBLE NOT NULL,\"detail_montant\" TEXT NOT NULL,\"fichier\" BLOB,PRIMARY KEY(\"id_trans\" AUTOINCREMENT));");
            QSqlQuery transfertData("INSERT INTO new_Transactions ("+newTransactions_ordreSouhaite+") SELECT "+newTransactions_ordreSouhaite+" FROM Transactions");
            overaidDatabase.close();
            QSqlDatabase::removeDatabase("QSQLITE");
            overaidDatabase = QSqlDatabase();
            overaidDatabase = QSqlDatabase::addDatabase("QSQLITE");
            overaidDatabase.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db");
            overaidDatabase.open();
            QSqlQuery supprimerAncienneTable("DROP TABLE Transactions");
            QSqlQuery renommerTable("ALTER TABLE new_Transactions RENAME TO Transactions");
        }

        QSqlQuery newAbonnements("PRAGMA table_info('Abonnements')");
        QString newAbonnements_colonnesActuelles;
        while (newAbonnements.next()) {
            QString nomColonne = newAbonnements.value("name").toString();
            newAbonnements_colonnesActuelles += nomColonne + ", ";
        }
        newAbonnements_colonnesActuelles.chop(2);
        QString newAbonnements_ordreSouhaite = "id_sub, id_compte, renouvellement, type, moyen, categorie, sous_categorie, description, montant, devise, montantDeviseCompte, detail_montant, fichier, dernier";
        if (newAbonnements_colonnesActuelles != newAbonnements_ordreSouhaite) {
            qDebug() << "newAbonnements";
            QSqlQuery nouvelleTable("CREATE TABLE\"new_Abonnements\" (\"id_sub\" INTEGER NOT NULL,\"id_compte\" INTEGER NOT NULL,\"renouvellement\" TEXT NOT NULL,\"type\" TEXT NOT NULL,\"moyen\" TEXT NOT NULL,\"categorie\" TEXT NOT NULL,\"sous_categorie\" TEXT NOT NULL,\"description\" TEXT NOT NULL,\"montant\" DOUBLE NOT NULL,\"devise\" TEXT NOT NULL,\"montantDeviseCompte\" DOUBLE NOT NULL,\"detail_montant\" TEXT NOT NULL,\"fichier\" BLOB NOT NULL,\"dernier\" TEXT NOT NULL, PRIMARY KEY(\"id_sub\" AUTOINCREMENT));");
            QSqlQuery transfertData("INSERT INTO new_Abonnements ("+newAbonnements_ordreSouhaite+") SELECT "+newAbonnements_ordreSouhaite+" FROM Abonnements");
            overaidDatabase.close();
            QSqlDatabase::removeDatabase("QSQLITE");
            overaidDatabase = QSqlDatabase();
            overaidDatabase = QSqlDatabase::addDatabase("QSQLITE");
            overaidDatabase.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/database.db");
            overaidDatabase.open();
            QSqlQuery supprimerAncienneTable("DROP TABLE Abonnements");
            QSqlQuery renommerTable("ALTER TABLE new_Abonnements RENAME TO Abonnements");
        }

        if(newAbonnements_colonnesActuelles != newAbonnements_ordreSouhaite || newTransactions_colonnesActuelles != newTransactions_ordreSouhaite || newComptes_colonnesActuelles != newComptes_ordreSouhaite) QSqlQuery vacuum("VACUUM");
    }

    //----Version 2.0-----

    QVersionNumber versionDB, versionApp;
    versionApp = QVersionNumber::fromString(version);
    QSqlQuery versionSQL("SELECT version FROM Settings");
    if(versionSQL.next()) versionDB = QVersionNumber::fromString(versionSQL.value("version").toString());

    if(versionDB < versionApp)
    {
        if(versionDB < QVersionNumber::fromString("2.1"))
        {
            QSqlQuery updateDebitCredit("UPDATE Transactions SET type='Debit' WHERE type='Dépense'");
            QSqlQuery updateDebitCredit2("UPDATE Transactions SET type='Credit' WHERE type='Entrée d''argent'");
            QSqlQuery updateDebitCredit3("UPDATE Abonnements SET type='Debit' WHERE type='Dépense'");
            QSqlQuery updateDebitCredit4("UPDATE Abonnements SET type='Credit' WHERE type='Entrée d''argent'");

            QSqlQuery updateDateTrans("UPDATE Transactions SET date=(substr(date, 7, 4) || substr(date, 4, 2) || substr(date, 1, 2)) WHERE date LIKE '%/%/%'");
            QSqlQuery updateDateFiltre("UPDATE Filtres SET date_debut=(substr(date_debut, 7, 4) || substr(date_debut, 4, 2) || substr(date_debut, 1, 2)), "
                                       "date_fin=(substr(date_fin, 7, 4) || substr(date_fin, 4, 2) || substr(date_fin, 1, 2)) WHERE date_debut LIKE '%/%/%'");
            QSqlQuery updateEspecesTrans("UPDATE Transactions SET moyen='Espèces' WHERE moyen='Éspèces'");
            QSqlQuery updateEspecesAbo("UPDATE Abonnements SET moyen='Espèces' WHERE moyen='Éspèces'");
        }

        QSqlQuery updateVersion("UPDATE Settings SET version='"+version+"'");
    }
    qDebug() << "updating time : " << time.msecsTo(QTime::currentTime()) << " msec.";
}
