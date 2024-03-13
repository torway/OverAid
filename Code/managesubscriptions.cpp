#include "managesubscriptions.h"
#include "ui_managesubscriptions.h"
#include "lineform.h"
#include "ui_lineform.h"
#include "overaid.h"

ManageSubscriptions::ManageSubscriptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManageSubscriptions)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("OverAid © - Gérer les abonnements"));

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &ManageSubscriptions::treeWidgetMenu);
}

ManageSubscriptions::~ManageSubscriptions()
{
    delete ui;
}

void ManageSubscriptions::on_pushButton_clicked()
{
    this->close();
}

void ManageSubscriptions::treeWidgetMenu(const QPoint &pos)
{
    QTreeWidget *tree = ui->treeWidget;

    QAction *newAct = new QAction(tr("Modifier cet abonnement"), this);
    connect(newAct, SIGNAL(triggered()), this, SLOT(modify_sub()));
    QAction *newAct_dup = new QAction(tr("Dupliquer cet abonnement"), this);
    connect(newAct_dup, SIGNAL(triggered()), this, SLOT(duplicate()));
    QAction *newAct_delete = new QAction(tr("Supprimer cet abonnement"), this);
    connect(newAct_delete, SIGNAL(triggered()), this, SLOT(delete_sub()));


    QMenu menu(this);
    menu.addAction(newAct);
    menu.addAction(newAct_dup);
    menu.addAction(newAct_delete);
    menu.exec(tree->mapToGlobal(pos));
}

void ManageSubscriptions::actu()
{
    ui->widgetAdd->parentWidget()->layout()->replaceWidget(ui->widgetAdd,trans,Qt::FindDirectChildrenOnly);
    disconnect(trans, 0, this, 0);
    connect(trans, SIGNAL(actu()), this, SLOT(actu()));
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

    QSqlQuery categorie("SELECT * FROM Catégories WHERE type='0' AND id_compte='"+QString::number(id_compte)+"' ORDER BY nom");

    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(7);
    QStringList headerLabels;
    headerLabels.append(tr("Jour"));
    headerLabels.append("");
    headerLabels.append(tr("Débit / Crédit"));
    headerLabels.append(tr("Catégorie"));
    headerLabels.append(tr("Sous-catégorie"));
    headerLabels.append(tr("Description"));
    headerLabels.append(tr("Montant"));
    headerLabels.append(tr("Moyen de paiement"));
    headerLabels.append("id");
    ui->treeWidget->setHeaderLabels(headerLabels);
    ui->treeWidget->hideColumn(8);

    //Affichage
    QSqlQuery transaction("SELECT *,Devises.symbole FROM Abonnements JOIN Devises ON Devises.code=Abonnements.devise WHERE id_compte='"+QString::number(id_compte)+"' ORDER BY renouvellement DESC");
    while(transaction.next())
    {
        QTreeWidgetItem *trans = new QTreeWidgetItem();
        ui->treeWidget->addTopLevelItem(trans);
        trans->setText(0, transaction.value("renouvellement").toString());

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

        double montantTotal = 0;
        //Si multi catégorie
        if(transaction.value("categorie").toString().contains(";"))
        {
            trans->setText(3, "---");
            trans->setText(4, "---");
            trans->setText(5, "---");

            for(int i = 0; i < transaction.value("categorie").toString().count(";")+1; i++)
            {
                QTreeWidgetItem *category = new QTreeWidgetItem();

                QSqlQuery cat("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("categorie").toString().replace("\"","").split(";").at(i)+"'");
                if(cat.next()) category->setText(3, cat.value("nom").toString());

                QSqlQuery cat2("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("sous_categorie").toString().replace("\"","").split(";").at(i)+"'");
                if(cat2.next()) category->setText(4, cat2.value("nom").toString());

                montantTotal += transaction.value("montant").toString().split(";").at(i).toDouble();

                double montantLine = transaction.value("montant").toString().split(";").at(i).toDouble();
                category->setText(6, QString::number(montantLine,'f',2).replace('.', ',')+" "+transaction.value("symbole").toString());

                if(montantLine == 0) category->setForeground(6, QColor("orange"));
                if((inOutH == "Debit" && montantLine > 0) || (inOutH == "Credit" && montantLine < 0)) category->setForeground(6, QColor("red"));
                if((inOutH == "Debit" && montantLine < 0) || (inOutH == "Credit" && montantLine > 0)) category->setForeground(6, QColor("green"));

                category->setText(5, transaction.value("description").toString().split(";").at(i));
                category->setText(8, transaction.value("id_sub").toString());

                trans->addChild(category);
            }

            QFont font;
            font.setUnderline(true);
            trans->setFont(6, font);
        }
        else
        {
            QSqlQuery cat("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("categorie").toString().replace("\"","")+"'");
            if(cat.next()) trans->setText(3, cat.value("nom").toString());

            QSqlQuery cat2("SELECT nom FROM Catégories WHERE id_cat='"+transaction.value("sous_categorie").toString().replace("\"","")+"'");
            if(cat2.next()) trans->setText(4, cat2.value("nom").toString());

            montantTotal = transaction.value("montant").toDouble();

            trans->setText(5, transaction.value("description").toString());
        }

        montantTotal = QString::number(montantTotal,'f',2).toDouble();

        trans->setText(6, QString::number(montantTotal,'f',2).replace('.', ',')+" "+transaction.value("symbole").toString());
        if(montantTotal == 0) trans->setForeground(6, QColor("orange"));
        if((inOutH == "Debit" && montantTotal > 0) || (inOutH == "Credit" && montantTotal < 0)) trans->setForeground(6, QColor("red"));
        if((inOutH == "Debit" && montantTotal < 0) || (inOutH == "Credit" && montantTotal > 0)) trans->setForeground(6, QColor("green"));

        trans->setText(8, transaction.value("id_sub").toString());
        if(!transaction.value("fichier").toByteArray().isEmpty()) trans->setIcon(1, QIcon(":/qrc/ressources/image/pdf.png"));

        ui->treeWidget->expandItem(trans);
    }

    for(int i = 0; i < ui->treeWidget->columnCount(); i++)
    {
        ui->treeWidget->resizeColumnToContents(i);
        ui->treeWidget->setColumnWidth(i, ui->treeWidget->columnWidth(i)*1.2);
    }
}

void ManageSubscriptions::modify_sub()
{
    if(!ui->treeWidget->currentItem()->text(8).isEmpty())
    {
        TransForm *modif = new TransForm(nullptr, "Modifier abo");
        modif->setWindowModality(Qt::ApplicationModal);
        modif->show();
        connect(modif, SIGNAL(actu()), this, SLOT(actu()));
        modif->id_categories = id_categories;
        modif->id_compte = id_compte;
        modif->actu_devise();
        modif->actu_projet();

        QSqlQuery transaction("SELECT * FROM Abonnements WHERE id_sub='"+ui->treeWidget->currentItem()->text(8)+"'");
        if(transaction.next())
        {
            modif->ui->dateEditAdd->setDate(QDate::fromString(transaction.value("renouvellement").toString(),"dd"));

            if(transaction.value("type").toString() == "Debit") modif->on_pushButton_out_clicked();
            else modif->on_pushButton_in_clicked();

            QString moyenH = transaction.value("moyen").toString();
            if(moyenH == "Carte bancaire") modif->ui->comboBox_moyen->setCurrentText(tr("Carte bancaire"));
            else if(moyenH == "Espèces") modif->ui->comboBox_moyen->setCurrentText(tr("Espèces"));
            else if(moyenH == "Chèque") modif->ui->comboBox_moyen->setCurrentText(tr("Chèque"));
            else if(moyenH == "Virement") modif->ui->comboBox_moyen->setCurrentText(tr("Virement"));
            else if(moyenH == "Prélèvement") modif->ui->comboBox_moyen->setCurrentText(tr("Prélèvement"));

            modif->ui->label_id->setText(transaction.value("id_sub").toString());

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

            if(!transaction.value("fichier").toByteArray().isEmpty())
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

void ManageSubscriptions::duplicate()
{
    if(!ui->treeWidget->currentItem()->text(8).isEmpty())
    {
        QSqlQuery transaction("SELECT * FROM Abonnements WHERE id_sub='"+ui->treeWidget->currentItem()->text(8)+"'");
        if(transaction.next())
        {
            trans->ui->dateEditAdd->setDate(QDate::fromString(transaction.value("renouvellement").toString(),"dd"));

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

void ManageSubscriptions::delete_sub()
{
    if(!ui->treeWidget->currentItem()->text(8).isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Supprimer la transaction"));
        if(ui->treeWidget->currentItem()->parent())
            msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer l'abonnement multi-transaction tous les %1 du mois ?").arg(QDate::fromString(ui->treeWidget->currentItem()->parent()->text(0),"dd").toString("dd")));
        else msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer l'abonnement '%1' tous les %2 du mois ?").arg(ui->treeWidget->currentItem()->text(5),QDate::fromString(ui->treeWidget->currentItem()->text(0),"dd").toString("dd")));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setStyleSheet("QLabel{min-width: 350px;}");
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok)
        {
            QSqlQuery remove("DELETE FROM Abonnements WHERE id_sub='"+ui->treeWidget->currentItem()->text(8)+"'");
            actu();
        }
    }
    else QMessageBox::warning(this, tr("Erreur"), tr("Il ne s'agit pas d'un abonnement."), tr("Fermer"));
}

void ManageSubscriptions::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    //Montant - Montant devise compte - Opération
    if(column == 6)
    {
        QString symboleCompte;
        QSqlQuery devise("SELECT Devises.symbole FROM Comptes JOIN Devises ON Devises.code=Comptes.devise WHERE id_compte='"+QString::number(id_compte)+"'");
        if(devise.next()) symboleCompte = devise.value("symbole").toString();

        QSqlQuery transaction("SELECT montant,devise,detail_montant,montantDeviseCompte,Devises.symbole FROM Abonnements JOIN Devises ON Devises.code=Abonnements.devise WHERE id_sub='"+item->text(8)+"'");
        if(transaction.next())
        {
            QString montant, detailMontant, montantDeviseCompte;
            //S'il s'agit d'une multi-ligne
            if(item->text(2) == "" && item->parent())
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

            ui->treeWidget->resizeColumnToContents(6);
            ui->treeWidget->setColumnWidth(6, ui->treeWidget->columnWidth(6)*1.2);
        }
    }

    //Ouvrir le PDF
    if(column == 1 && !item->icon(1).isNull())
    {
        QSqlQuery transaction("SELECT fichier FROM Abonnements WHERE id_sub='"+item->text(8)+"'");
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

