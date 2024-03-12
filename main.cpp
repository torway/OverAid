#include "overaid.h"
#include "ui_overaid.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator anglais;
    anglais.load(":/qrc/ressources/translations/english.qm");

    //Création du fichier settings.csv
    if(!QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).exists() || !QFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv").exists())
    {
        QDir().mkdir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv");

        if (file.open(QIODevice::ReadWrite))
        {
            file.resize(0);
            QTextStream stream(&file);
            stream << "settings;translate;tab;filters;filtersStat";
            stream << "\n";
            stream << "false;francais;0;0;true,0,0";
            file.close();
        }
    }

    //Lecture du fichier settings
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/settings.csv");

    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        QStringList headers = stream.readLine().split(";");
        QStringList parameters = stream.readLine().split(";");

        //Anglais
        if(parameters.at(headers.indexOf("translate")) == "anglais") a.installTranslator(&anglais);

        file.close();
    }

    OverAid w;

    //Langue sélectionnée dans le menu
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);
        QStringList headers = stream.readLine().split(";");
        QStringList parameters = stream.readLine().split(";");

        w.language = parameters.at(headers.indexOf("translate"));

        //Anglais
        if(parameters.at(headers.indexOf("translate")) == "anglais")
        {
            foreach(QAction *action, w.ui->menuLangage->actions())
                if(action->isCheckable()) action->setChecked(false);
            w.ui->actionAnglais->setChecked(true);
        }
    }

    w.actu_compte();
    w.show();
    return a.exec();
}
