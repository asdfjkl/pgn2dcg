#ifndef DATABASE_H
#define DATABASE_H

#include <QString>

namespace chess {

class Database
{
public:
    Database(QString &filename);

    void importPgnAndSave(QString &pgnfile);
    void saveToFile();

private:
    // filename is only the base, always append *.dcs, *.dcn, *.dcg, *.dci
    QString filenameBase;
    QString filenameNames;
    QString filenameSites;
    QString filenameIndex;
    QString filenameGames;
    QByteArray magicNameString;
    QByteArray magicIndexString;
    QByteArray magicGamesString;
    QByteArray magicSitesString;
    QMap<quint32, QString> *offsetNames;
    QMap<quint32, QString> *offsetSites;
    void writeSites();
    void writeNames();
    void writeIndex();
    void writeGames();
    // scans all headers in pgn file and reads names and sites into passed maps
    void importPgnNamesSites(QString &pgnfile, QMap<QString, quint32> *names, QMap<QString, quint32> *sites);
    void importPgnAppendNames(QMap<QString, quint32> *names);
    void importPgnAppendSites(QMap<QString, quint32> *sites);
    void importPgnAppendGamesIndices(QString &pgnfile, QMap<QString, quint32> *names, QMap<QString, quint32> *sites);


};

}

#endif // DATABASE_H
