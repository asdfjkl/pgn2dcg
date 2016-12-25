#ifndef DATABASE_H
#define DATABASE_H

#include <QString>

namespace chess {

class Database
{
public:
    Database(QString &filename);

    void importPgn(QString &pgnfile);
    void saveToFile();
    // scans all headers in pgn file and reads names and sites into passed maps
    void importPgnNamesSites(QString &pgnfile, QMap<QString, quint32> *names, QMap<QString, quint32> *sites);



private:
    // filename is only the base, always append *.dcs, *.dcn, *.dcg, *.dci
    QString filename;
    void writeSites();
    void writeNames();
    void writeIndex();
    void writeGames();


};

}

#endif // DATABASE_H
