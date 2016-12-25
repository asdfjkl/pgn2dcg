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
    QString test = QString("foo");
    chess::Database *database = new chess::Database(test);
    QMap<QString, quint32> *names = new QMap<QString, quint32>();
    QMap<QString, quint32> *sites = new QMap<QString, quint32>();
    database->importPgnNamesSites(pgnFileName, names, sites);


    //qDebug() << "scanned headers FINISHED";

    // now write down sites and names
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
          quint32 offset = fnNames.pos();
          s.writeRawData(name_i,36);
          names->insert(name_i, offset);
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
          quint32 offset = fnSites.pos();
          s.writeRawData(site_i,36);
          sites->insert(site_i, offset);
      }
      success = true;
    } else {
      std::cerr << "error opening output file\n";
    }
    fnNames.close();
    if(!success) {
        throw std::invalid_argument("Error writing file");
    }
    pgnFile.close();


    // now save everything
    chess::HeaderOffset *header = new chess::HeaderOffset();
    quint64 offset = 0;
    quint64 size = pgnFile.size();
    bool stop = false;

    chess::DcgWriter *dcgWriter = new chess::DcgWriter();

    QString fnIndexString = pgnFileName.left(pgnFileName.size()-3).append("dci");
    QFile fnIndex(fnIndexString);

    QString fnGamesString = pgnFileName.left(pgnFileName.size()-3).append("dcg");
    QFile fnGames(fnGamesString);

    chess::PgnReader *pgnReader = new chess::PgnReader();
    const char* encoding = pgnReader->detect_encoding(pgnFileName);
    if(fnIndex.open(QFile::WriteOnly)) {
      QDataStream si(&fnIndex);
      QByteArray magicIndexString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x69");
      si.writeRawData(magicIndexString, magicIndexString.length());

      if(fnGames.open(QFile::WriteOnly)) {
        QDataStream sg(&fnGames);
        QByteArray magicGamesString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x67");
        sg.writeRawData(magicGamesString, magicGamesString.length());

        std::cout << "\nsaving games: 0/"<< size;
        int i = 0;
        while(!stop) {
            if(i%100==0) {
                std::cout << "\rsaving games: "<<offset<< "/"<<size << std::flush;
            }
            i++;
            int res = pgnReader->readNextHeader(pgnFileName, encoding, &offset, header);
            if(res < 0) {
                stop = true;
                continue;
            }
            // first write index entry
            // status
            si << quint8(0x00);
            // game offset
            si << quint64(fnGames.pos());
            // white offset
            QString white = header->headers->value("White");
            quint32 whiteOffset = names->value(white);
            si << whiteOffset;
            // black offset
            QString black = header->headers->value("Black");
            quint32 blackOffset = names->value(black);
            si << blackOffset;
            // round
            quint32 round = header->headers->value("Round").toUInt();
            si << round;
            // site offset
            quint32 site_offset = sites->value(header->headers->value("Site"));
            si << site_offset;
            // elo white
            quint16 elo_white = header->headers->value("Elo White").toUInt();
            si << elo_white;
            quint16 elo_black = header->headers->value("Elo White").toUInt();
            si << elo_black;
            // result
            if(header->headers->contains("Result")) {
                QString res = header->headers->value("Result");
                if(res == "1-0") {
                    si << 0x01;
                } else if(res == "0-1") {
                    si << 0x02;
                } else if(res == "1/2-1/2") {
                    si << 0x03;
                } else {
                    si << 0x00;
                }
            } else {
                si << 0x00;
            }
            // ECO
            if(header->headers->contains("ECO")) {
                QByteArray eco = header->headers->value("ECO").toUtf8();
                si.writeRawData(eco, eco.length());
            } else {
                si << 0x00 << 0x00 << 0x00;
            }
            // parse date
            if(header->headers->contains("Date")) {
                QString date = header->headers->value("Date");
                // try to parse the date
                quint16 year = 0;
                quint8 month = 0;
                quint8 day = 0;
                QStringList dd_mm_yy = date.split(".");
                if(dd_mm_yy.size() > 0 && dd_mm_yy.at(0).length() == 4) {
                    quint16 prob_year = dd_mm_yy.at(0).toInt();
                    if(prob_year > 0 && prob_year < 2100) {
                        year = prob_year;
                    }
                    if(dd_mm_yy.size() > 1 && dd_mm_yy.at(1).length() == 2) {
                        quint16 prob_month = dd_mm_yy.at(1).toInt();
                        if(prob_year > 0 && prob_year <= 12) {
                            month = prob_month;
                        }
                        if(dd_mm_yy.size() > 2 && dd_mm_yy.at(2).length() == 2) {
                        quint16 prob_day = dd_mm_yy.at(2).toInt();
                        if(prob_year > 0 && prob_year < 32) {
                            day = prob_day;
                            }
                        }
                    }
                }
                si << year;
                si << month;
                si << day;
            } else {
                si << 0x00 << 0x00 << 0x00 << 0x00;
            }
            //qDebug() << "just before reading back file";
            chess::Game *g = pgnReader->readGameFromFile(pgnFileName, encoding, header->offset);
            //qDebug() << "READ file ok";
            QByteArray *g_enc = dcgWriter->encodeGame(g); //"<<<<<<<<<<<<<<<<<<<<<< this is the cause of mem acc fault"
            //qDebug() << "enc ok";
            sg.writeRawData(*g_enc, g_enc->length());
            delete g_enc;
            header->headers->clear();
            if(header->headers!=0) {
               delete header->headers;
            }
            delete g;
        }
        std::cout << "\rsaving games: "<<size<< "/"<<size << std::endl;
      }
      fnGames.close();
    }
    fnIndex.close();
    delete header;
    delete pgnReader;
    delete dcgWriter;
    delete names;
    delete sites;
    delete database;
    return 0;
}
