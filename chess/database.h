#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include "chess/pgn_reader.h"
#include "chess/dcgencoder.h"

namespace chess {

class Database
{
public:
    Database(QString &filename);
    ~Database();

    void importPgnAndSave(QString &pgnfile);
    void saveToFile();

private:
    // filename is only the base, always append *.dcs, *.dcn, *.dcg, *.dci
    QString filenameBase;
    QString filenameNames;
    QString filenameSites;
    QString filenameEvents;
    QString filenameIndex;
    QString filenameGames;
    QByteArray magicNameString;
    QByteArray magicIndexString;
    QByteArray magicGamesString;
    QByteArray magicSitesString;
    QByteArray magicEventString;
    QMap<quint32, QString> *offsetNames;
    QMap<quint32, QString> *offsetSites;
    QMap<quint32, QString> *offsetEvents;
    void writeSites();
    void writeNames();
    void writeIndex();
    void writeGames();
    // scans all headers in pgn file and reads names and sites into passed maps
    void importPgnNamesSitesEvents(QString &pgnfile,
                                   QMap<QString, quint32> *names,
                                   QMap<QString, quint32> *sites,
                                   QMap<QString, quint32> *events);
    void importPgnAppendNames(QMap<QString, quint32> *names);
    void importPgnAppendSites(QMap<QString, quint32> *sites);
    void importPgnAppendEvents(QMap<QString, quint32> *events);
    void importPgnAppendGamesIndices(QString &pgnfile,
                                     QMap<QString, quint32> *names,
                                     QMap<QString, quint32> *sites,
                                     QMap<QString, quint32> *events);

    chess::DcgEncoder *dcgencoder;
    chess::PgnReader *pgnreader;

};

}

#endif // DATABASE_H
