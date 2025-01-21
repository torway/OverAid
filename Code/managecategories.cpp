#include "managecategories.h"
#include "ui_managecategories.h"

ManageCategories::ManageCategories(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManageCategories)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("OverAid © - Gérer les catégories"));
}

ManageCategories::~ManageCategories()
{
    delete ui;
    emit actualiser();
}

void ManageCategories::on_pushButton_close_clicked()
{
    this->close();
    emit actualiser();
}

void ManageCategories::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit actualiser();
}

void ManageCategories::actu_categorie()
{
    ui->listWidget_cat->clear();
    id_cats.clear();

    QSqlQuery categorie("SELECT * FROM Catégories WHERE type=0 AND id_compte='"+QString::number(id_compte)+"' ORDER BY nom ASC");
    while (categorie.next())
    {
        id_cats.append(categorie.value("id_cat").toInt());
        ui->listWidget_cat->addItem(categorie.value("nom").toString());
    }

    if(ui->listWidget_cat->count() > 0)
        ui->listWidget_cat->setCurrentRow(0);
}

void ManageCategories::actu_sousCategorie()
{
    ui->listWidget_subcat->clear();
    id_subcats.clear();

    if(!id_cats.isEmpty() && ui->listWidget_cat->currentRow() < id_cats.count())
    {
        QSqlQuery subcategorie("SELECT * FROM Catégories WHERE type=1 AND cat0='"+QString::number(id_cats.at(ui->listWidget_cat->currentRow()))+"' ORDER BY nom ASC");
        while (subcategorie.next())
        {
            id_subcats.append(subcategorie.value("id_cat").toInt());
            ui->listWidget_subcat->addItem(subcategorie.value("nom").toString());
        }
    }
}

void ManageCategories::on_listWidget_cat_currentRowChanged()
{
    if(ui->listWidget_cat->currentRow() != -1) actu_sousCategorie();
}

void ManageCategories::on_pushButton_addCat_clicked()
{
    ui->listWidget_cat->addItem("");
    QListWidgetItem *item = ui->listWidget_cat->item(ui->listWidget_cat->count()-1);

    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->listWidget_cat->setCurrentItem(item);
    ui->listWidget_cat->editItem(item);

    on_listWidget_cat_itemDoubleClicked(item);
}

void ManageCategories::on_pushButton_addCat2_clicked()
{
    ui->listWidget_subcat->addItem("");
    QListWidgetItem *item = ui->listWidget_subcat->item(ui->listWidget_subcat->count()-1);

    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->listWidget_subcat->setCurrentItem(item);
    ui->listWidget_subcat->editItem(item);

    on_listWidget_subcat_itemDoubleClicked(item);
}

void ManageCategories::on_pushButton_deleteCat_clicked()
{
    //Supprimer la catégorie
    if(ui->listWidget_cat->currentRow() != -1)
    {
        QString countS,countT,countA;

        QSqlQuery countSubCat("SELECT COUNT(*) FROM Catégories WHERE cat0='"+QString::number(id_cats.at(ui->listWidget_cat->currentRow()))+"'");
        if(countSubCat.next()) countS = countSubCat.value(0).toString();
        QSqlQuery countTrans("SELECT COUNT(*) FROM Transactions WHERE categorie LIKE '%\""+QString::number(id_cats.at(ui->listWidget_cat->currentRow()))+"\"%'");
        if(countTrans.next()) countT = countTrans.value(0).toString();
        QSqlQuery countAbo("SELECT COUNT(*) FROM Abonnements WHERE categorie LIKE '%\""+QString::number(id_cats.at(ui->listWidget_cat->currentRow()))+"\"%'");
        if(countAbo.next()) countA = countAbo.value(0).toString();

        QMessageBox msgBox;
        msgBox.setText(tr("Supprimer la catégorie"));
        msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer la catégorie '")+ui->listWidget_cat->currentItem()->text()+"' ?" +
                                  tr("\n\nSupprimer cette catégorie entraînera la suppression de %1 sous-catégorie(s), de %2 transaction(s) et de %3 abonnement(s).").arg(countS,countT,countA));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setStyleSheet("QLabel{min-width: 350px;}");
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok)
        {
            QString cat = QString::number(id_cats.at(ui->listWidget_cat->currentRow()));
            QSqlQuery removeCat("DELETE FROM Catégories WHERE id_cat='"+cat+"'");
            QSqlQuery removeSubCat("DELETE FROM Catégories WHERE cat0='"+cat+"'");

            QSqlQuery removeTrans("DELETE FROM Transactions WHERE categorie LIKE '%\""+cat+"\"%'");
            QSqlQuery removeAbo("DELETE FROM Abonnements WHERE categorie LIKE '%\""+cat+"\"%'");

            actu_categorie();
            actu_sousCategorie();
        }
    }
    else QMessageBox::warning(this, tr("Impossible de supprimer la catégorie."), tr("Aucune catégorie n'a été séléctionnée."), tr("Corriger"));
}

void ManageCategories::on_pushButton_deleteCat2_clicked()
{
    //Supprimer la sous-catégorie
    if(ui->listWidget_subcat->currentRow() != -1)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Supprimer la sous-catégorie"));
        msgBox.setInformativeText(tr("Etes-vous sûr(e)(s) de vouloir supprimer la catégorie '")+ui->listWidget_subcat->currentItem()->text()+"' ?" +
                                  tr("\n\nSupprimer cette sous-catégorie videra la sous-catégorie de ses transactions et de ses abonnements."));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setStyleSheet("QLabel{min-width: 350px;}");
        int ret = msgBox.exec();

        if (ret == QMessageBox::Ok)
        {
            QString cat2 = QString::number(id_subcats.at(ui->listWidget_subcat->currentRow()));
            QSqlQuery remove("DELETE FROM Catégories WHERE id_cat='"+cat2+"'");

            QSqlQuery update("UPDATE Transactions SET sous_categorie = REPLACE(sous_categorie,\""+cat2+"\",\"\") WHERE sous_categorie LIKE '%\""+cat2+"\"%'");
            QSqlQuery update2("UPDATE Abonnements SET sous_categorie = REPLACE(sous_categorie,\""+cat2+"\",\"\") WHERE sous_categorie LIKE '%\""+cat2+"\"%'");

            actu_sousCategorie();
        }
    }
    else QMessageBox::warning(this, tr("Impossible de supprimer la sous-catégorie."), tr("Aucune sous-catégorie n'a été séléctionnée."), tr("Corriger"));
}

void ManageCategories::on_listWidget_cat_itemDoubleClicked(QListWidgetItem *item)
{
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    connect(ui->listWidget_cat->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, [=]() {
        listWidget_itemDoubleClicked_finished("0", ui->listWidget_cat, ui->listWidget_cat->currentItem());
    });
}

void ManageCategories::on_listWidget_subcat_itemDoubleClicked(QListWidgetItem *item)
{
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    connect(ui->listWidget_subcat->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, [=]() {
        listWidget_itemDoubleClicked_finished("1", ui->listWidget_subcat, ui->listWidget_subcat->currentItem());
    });
}

void ManageCategories::listWidget_itemDoubleClicked_finished(QString catType, QListWidget *listWidget, QListWidgetItem *item)
{
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    QString id = "-1";
    if(catType == "0" && listWidget->currentRow() < id_cats.count()) id = QString::number(id_cats.at(listWidget->currentRow()));
    else if(catType == "1" && listWidget->currentRow() < id_subcats.count()) id = QString::number(id_subcats.at(listWidget->currentRow()));

    QString cat0 = "1+1";
    if(catType == "1") cat0 = "cat0='"+QString::number(id_cats.at(ui->listWidget_cat->currentRow()))+"'";

    QSqlQuery exist("SELECT * FROM Catégories WHERE nom='"+item->text().replace("'","''")+"' AND id_compte='"+QString::number(id_compte)+"' AND id_cat!='"+id+"' AND "+cat0+"");
    if(!exist.next())
    {
        if(id != "-1") QSqlQuery update("UPDATE Catégories SET nom = '"+item->text().replace("'","''")+"' WHERE id_cat='"+id+"'");
        else if(!item->text().isEmpty())
        {
            QSqlQuery ajouter("INSERT INTO Catégories (type,nom,cat0,id_compte) VALUES ('"+catType+"','"+item->text().replace("'","''")+"',"
                              "'"+(cat0 == "1+1" ? "" : QString::number(id_cats.at(ui->listWidget_cat->currentRow())))+"','"+QString::number(id_compte)+"')");
            if(catType == "0") QMessageBox::information(this, tr("Catégorie ajoutée"), tr("La catégorie a bien été ajoutée."), tr("Fermer"));
            else if(catType == "1") QMessageBox::information(this, tr("Sous catégorie ajoutée"), tr("La sous-catégorie a bien été ajoutée à la catégorie %1.").arg(ui->listWidget_cat->currentItem()->text()), tr("Fermer"));
        }
    }
    else
    {
        if(catType == "0") QMessageBox::warning(this, tr("Erreur"), tr("Une catégorie porte déjà ce nom.\nVeuillez en choisir un autre."), tr("Fermer"));
        else if(catType == "1") QMessageBox::warning(this, tr("Erreur"), tr("Une sous-catégorie porte déjà ce nom au sein de cette catégorie.\nVeuillez en choisir un autre."), tr("Fermer"));
    }

    QString oldCat = ui->listWidget_cat->currentItem() != nullptr ? ui->listWidget_cat->currentItem()->text() : "";
    QString oldCat2 = ui->listWidget_subcat->currentItem() != nullptr ? ui->listWidget_subcat->currentItem()->text() : "";
    actu_categorie();
    actu_sousCategorie();
    if(oldCat != "") ui->listWidget_cat->setCurrentItem(ui->listWidget_cat->findItems(oldCat,Qt::MatchExactly).at(0));
    if(oldCat2 != "") ui->listWidget_subcat->setCurrentItem(ui->listWidget_subcat->findItems(oldCat2,Qt::MatchExactly).at(0));

    disconnect(ui->listWidget_cat->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, nullptr);
    disconnect(ui->listWidget_subcat->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, nullptr);
}
