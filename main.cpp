#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDataStream>
#include <iostream>
#include "chess/pgn_reader.h"

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

    // first scan pgn file, extract all names and sites, and but them
    // into Maps (Site, offset) and (Name, offset)
    std::cout << "opening " << pgnFileName.toStdString() << std::endl;
    chess::PgnReader *pgnReader = new chess::PgnReader();
    std::cout << "scanning headers... " << std::endl;
    QList<chess::HeaderOffset*> *headers = pgnReader->scan_headers(pgnFileName);
    QMap<QString, quint64> *names = new QMap<QString, quint64>();
    QMap<QString, quint64> *sites = new QMap<QString, quint64>();
    for(int i=0; i<headers->size();i++) {
        std::cout << "converting game " << (i+1) << " of " << headers->size() << std::endl;
        chess::Game *gi = pgnReader->readGameFromFile(pgnFileName, headers->value(i)->offset);
        QString whitePlayer = gi->headers->value("White");
        QString blackPlayer = gi->headers->value("Black");
        QString site = gi->headers->value("Site");
        // add white player name
        // some name normalization could take place here
        if(!names->contains(whitePlayer)) {
            names->insert(whitePlayer, 0);
        }
        // add black player name
        if(!names->contains(blackPlayer)) {
            names->insert(blackPlayer, 0);
        }
        // add site name
        if(!sites->contains(site)) {
            sites->insert(site, 0);
        }
        delete gi;
    }

    // write names into file
    QString fnNamesString = pgnFileName.left(pgnFileName.size()-3).append("dcn");
    QFile fnNames(fnNamesString);
    QByteArray magicNameString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x6e");
    bool success = false;
    if(fnNames.open(QFile::WriteOnly)) {
      QDataStream s(&fnNames);
      s.writeRawData(magicNameString, magicNameString.length());
      QList<QString> keys = names->keys();
      for (int i = 0; i < keys.length(); i++) {
          QByteArray name_i = keys.at(i).toUtf8();
          // truncate if too long
          if(name_i.size() > 36) {
              name_i = name_i.left(36);
          }
          int pad_n = 36 - name_i.length();
          if(pad_n > 0) {
              for(int j=0;j<pad_n;j++) {
                  name_i.append(0x20);
              }
          }
          s.writeRawData(name_i,36);
      }
      success = true;
    } else {
      std::cerr << "error opening output file\n";
    }
    fnNames.close();
    if(!success) {
        throw std::invalid_argument("Error writing file");
    }

    // write sites into file
    QString fnSitesString = pgnFileName.left(pgnFileName.size()-3).append("dcs");
    QFile fnSites(fnSitesString);
    QByteArray magicSiteString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x73");
    success = false;
    if(fnSites.open(QFile::WriteOnly)) {
      QDataStream s(&fnSites);
      s.writeRawData(magicSiteString, magicSiteString.length());
      QList<QString> keys = sites->keys();
      for (int i = 0; i < keys.length(); i++) {
          QByteArray site_i = keys.at(i).toUtf8();
          // truncate if too long
          if(site_i.size() > 36) {
              site_i = site_i.left(36);
          }
          int pad_n = 36 - site_i.length();
          if(pad_n > 0) {
              for(int j=0;j<pad_n;j++) {
                  site_i.append(0x20);
              }
          }
          s.writeRawData(site_i,36);
      }
      success = true;
    } else {
      std::cerr << "error opening output file\n";
    }
    fnNames.close();
    if(!success) {
        throw std::invalid_argument("Error writing file");
    }

    return 0;
}
