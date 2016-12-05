#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <QDebug>
#include "chess/pgn_reader.h"
#include "chess/dcgwriter.h"

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
    // guess the encoding of the pgn file
    const char* encoding = pgnReader->detect_encoding(pgnFileName);

    std::cout << "scanning headers... " << std::endl;
    QList<chess::HeaderOffset*> *headers = pgnReader->scan_headers(pgnFileName, encoding);
    QMap<QString, quint32> *names = new QMap<QString, quint32>();
    QMap<QString, quint32> *sites = new QMap<QString, quint32>();
    for(int i=0; i<headers->size();i++) {
        std::cout << "converting game " << (i+1) << " of " << headers->size() << std::endl;
        chess::Game *gi = pgnReader->readGameFromFile(pgnFileName, encoding, headers->value(i)->offset);
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
    chess::DcgWriter *dcgWriter = new chess::DcgWriter();

    QString fnIndexString = pgnFileName.left(pgnFileName.size()-3).append("dci");
    QFile fnIndex(fnIndexString);

    QString fnGamesString = pgnFileName.left(pgnFileName.size()-3).append("dcg");
    QFile fnGames(fnGamesString);

    if(fnIndex.open(QFile::WriteOnly)) {
      QDataStream si(&fnIndex);
      QByteArray magicIndexString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x69");
      si.writeRawData(magicIndexString, magicIndexString.length());

      if(fnGames.open(QFile::WriteOnly)) {
        QDataStream sg(&fnGames);
        QByteArray magicGamesString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x62");
        sg.writeRawData(magicGamesString, magicGamesString.length());

        for(int i=0;i<headers->size();i++) {

            chess::HeaderOffset *header_i = headers->at(i);
            qDebug() << "loop1";

            // first write index entry
            // marker
            si << quint8(0x00);
            qDebug() << "loop2";

            // game offset
            QByteArray *offset = new QByteArray();
            dcgWriter->append_as_uint64(offset, fnGames.pos());
            qDebug() << "loop3";

            si.writeRawData(*offset, offset->length());

            // white offset
            QString white = header_i->headers->value("White");
            QByteArray *offset_white = new QByteArray();
            dcgWriter->append_as_uint32(offset_white, quint32(names->value(white)));
            si.writeRawData(*offset_white, offset_white->length());
            // black offset
            QString black = header_i->headers->value("Black");
            QByteArray *offset_black = new QByteArray();
            dcgWriter->append_as_uint32(offset_black, quint32(names->value(black)));
            si.writeRawData(*offset_black, offset_black->length());
            // round
            quint32 round = header_i->headers->value("Round").toUInt();
            si << round;
            // site offset
            quint32 site_offset = sites->value(header_i->headers->value("Site"));
            si << site_offset;
            // elo white
            quint16 elo_white = header_i->headers->value("Elo White").toUInt();
            si << elo_white;
            quint16 elo_black = header_i->headers->value("Elo White").toUInt();
            si << elo_black;
            // result TODO
            si << 0x00;
            // ECO
            si << QString("A00");
            // year
            si << quint16(1981);
            // month
            si << quint8(3);
            // day
            si << quint8(20);

            chess::Game *g = pgnReader->readGameFromFile(pgnFileName, encoding, header_i->offset);
            QByteArray *g_enc = dcgWriter->encodeGame(g);
            sg.writeRawData(*g_enc, g_enc->length());

        }

      }
      fnGames.close();
    }
    fnIndex.close();


    return 0;
}
