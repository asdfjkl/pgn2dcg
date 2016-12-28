#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <QStringList>
#include <QDebug>
#include "chess/pgn_reader.h"
#include "chess/dcgwriter.h"
#include "chess/database.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("pgn2dcg");
    QCoreApplication::setApplicationVersion("v1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("pgn2dcg");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "PGN input file."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "*dc* output files."));

    QCommandLineOption pgnFileOption(QStringList() << "p" << "pgn-file",
              QCoreApplication::translate("main", "PGN input file <games.pgn>."),
              QCoreApplication::translate("main", "filename."));
    parser.addOption(pgnFileOption);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    // source pgn is args.at(0), destination filename is args.at(1)

    if (!parser.parse(QCoreApplication::arguments())) {
        QString errorMessage = parser.errorText();
        std::cout << errorMessage.toStdString() << std::endl;
        exit(0);
    }

    QString pgnFileName = parser.value(pgnFileOption);
    QFile pgnFile;
    pgnFile.setFileName(pgnFileName);
    if(!pgnFile.exists()) {
        std::cout << "Error: can't open PGN file." << std::endl;
        exit(0);
    }

    // inser db call here
    QString baseName = pgnFileName.left(pgnFileName.size()-4);
    chess::Database *database = new chess::Database(baseName);
    QMap<QString, quint32> *names = new QMap<QString, quint32>();
    QMap<QString, quint32> *sites = new QMap<QString, quint32>();
    database->importPgnNamesSites(pgnFileName, names, sites);

    database->importPgnAppendSites(sites);
    database->importPgnAppendNames(names);
    database->importPgnAppendGamesIndices(pgnFileName, names, sites);

    delete names;
    delete sites;
    delete database;
    return 0;
}
