#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <QStringList>
#include <QDebug>
#include "chess/pgn_reader.h"
#include "chess/dcgencoder.h"
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


    QCommandLineOption dbFileOption(QStringList() << "o" << "out-file",
              QCoreApplication::translate("main", "database file <database.dcg>."),
              QCoreApplication::translate("main", "filename."));
    parser.addOption(dbFileOption);

    QCommandLineOption appendOption("a", QCoreApplication::translate("main", "If database exists, append instead of overwriting"));
    parser.addOption(appendOption);

    parser.process(app);

    bool append = parser.isSet(appendOption);

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
        std::cout << "Error: can't open PGN file or no PGN filename given." << std::endl;
        exit(0);
    }

    pgnFile.setFileName(pgnFileName);
    if(!pgnFile.exists()) {
        std::cout << "Error: can't open PGN file." << std::endl;
        exit(0);
    }

    QString dbFileName = parser.value(dbFileOption);
    if(dbFileName.endsWith(".dcg") || dbFileName.endsWith(".dci") || dbFileName.endsWith(".dcs")
            || dbFileName.endsWith(".dcn")) {
        dbFileName = dbFileName.left(dbFileName.size()-4);
    }
    if(dbFileName.isEmpty()) {
        std::cout << "Error: no output Database filename given." << std::endl;
        exit(0);
    }

    if(!append) {
        QFile dbFile;
        dbFile.setFileName(dbFileName);
        if(dbFile.exists()) {
            bool succ = dbFile.remove();
            if(!succ) {
                std::cout << "Error: Output File exists, can't be deleted and append option is not selected." << std::endl;
                exit(0);
            }
        }
    }

    chess::Database *database = new chess::Database(dbFileName);
    database->importPgnAndSave(pgnFileName);
    delete database;

    return 0;
}
