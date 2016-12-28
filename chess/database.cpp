#include "database.h"
#include "chess/game.h"
#include "chess/pgn_reader.h"
#include "chess/dcgwriter.h"
#include "assert.h"
#include <iostream>
#include <QFile>
#include <QDataStream>

chess::Database::Database(QString &filename)
{
    this->filenameBase = filename;
    this->filenameGames = QString(filename).append(".dcg");
    this->filenameIndex = QString(filename).append(".dci");
    this->filenameNames = QString(filename).append(".dcn");
    this->filenameSites = QString(filename).append(".dcs");
    this->magicNameString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x6e");
    this->magicIndexString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x69");
    this->magicGamesString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x67");
    this->magicSitesString = QByteArrayLiteral("\x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x73");
}

void chess::Database::importPgn(QString &pgnfile) {

}

void chess::Database::importPgnNamesSites(QString &pgnfile, QMap<QString, quint32> *names, QMap<QString, quint32> *sites) {

    std::cout << "scanning names and sites from " << pgnfile.toStdString() << std::endl;
    chess::PgnReader *pgnReader = new chess::PgnReader();
    // guess the encoding of the pgn file
    const char* encoding = pgnReader->detect_encoding(pgnfile);

    chess::HeaderOffset* header = new chess::HeaderOffset();

    quint64 offset = 0;
    bool stop = false;

    std::cout << "scanning at 0";
    int i = 0;
    while(!stop) {
        if(i%100==0) {
            std::cout << "\rscanning at " << offset;
        }
        i++;
        int res = pgnReader->readNextHeader(pgnfile, encoding, &offset, header);
        if(res < 0) {
            stop = true;
            continue;
        }
        if(header->headers != 0) {
            if(header->headers->contains("Site")) {
                sites->insert(header->headers->value("Site"), 0);
            }
            if(header->headers->contains("White")) {
                names->insert(header->headers->value("White"), 0);
            }
            if(header->headers->contains("Black")) {
                names->insert(header->headers->value("Black"), 0);
            }
        }
        header->headers->clear();
        if(header->headers!=0) {
           delete header->headers;
        }
    }
    std::cout << std::endl << "scanning finished" << std::flush;
    delete pgnReader;
    delete header;
}

// save the map at the _end_ of file with filename (i.e. apend new names or sites)
// update the offset while saving
void chess::Database::importPgnAppendNames(QMap<QString, quint32> *names) {
    QFile fnNames(this->filenameNames);
    if(fnNames.open(QFile::Append)) {
        if(fnNames.pos() == 0) {
            fnNames.write(this->magicNameString, this->magicNameString.length());
        }
        QList<QString> keys = names->keys();
        for (int i = 0; i < keys.length(); i++) {
            QByteArray name_i = keys.at(i).toUtf8();
            // truncate if too long
            if(name_i.size() > 36) {
                name_i = name_i.left(36);
            }
            // pad to 36 byte if necc.
            int pad_n = 36 - name_i.length();
            if(pad_n > 0) {
                for(int j=0;j<pad_n;j++) {
                    name_i.append(0x20);
                }
            }
        quint32 offset = fnNames.pos();
        fnNames.write(name_i,36);
        names->insert(name_i, offset);
        }
    }
    fnNames.close();
}

// save the map at the _end_ of file with filename (i.e. apend new names or sites)
// update the offset while saving
void chess::Database::importPgnAppendSites(QMap<QString, quint32> *sites) {
    QFile fnSites(this->filenameSites);
    if(fnSites.open(QFile::Append)) {
        if(fnSites.pos() == 0) {
            fnSites.write(this->magicNameString, this->magicNameString.length());
        }
        QList<QString> keys = sites->keys();
        for (int i = 0; i < keys.length(); i++) {
            QByteArray site_i = keys.at(i).toUtf8();
            // truncate if too long
            if(site_i.size() > 36) {
                site_i = site_i.left(36);
            }
            // pad to 36 byte if necc.
            int pad_n = 36 - site_i.length();
            if(pad_n > 0) {
                for(int j=0;j<pad_n;j++) {
                    site_i.append(0x20);
                }
            }
        quint32 offset = fnSites.pos();
        fnSites.write(site_i,36);
        sites->insert(site_i, offset);
        }
    }
    fnSites.close();
}

void chess::Database::importPgnAppendGamesIndices(QString &pgnfile, QMap<QString, quint32> *names, QMap<QString, quint32> *sites) {

    // now save everything
    chess::HeaderOffset *header = new chess::HeaderOffset();
    quint64 offset = 0;
    QFile pgnFile(pgnfile);
    quint64 size = pgnFile.size();
    bool stop = false;

    chess::DcgWriter *dcgWriter = new chess::DcgWriter();
    chess::PgnReader *pgnReader = new chess::PgnReader();

    const char* encoding = pgnReader->detect_encoding(pgnfile);

    QFile fnIndex(this->filenameIndex);
    QFile fnGames(this->filenameGames);
    if(fnIndex.open(QFile::Append)) {
        if(fnGames.open(QFile::Append)) {
            if(fnIndex.pos() == 0) {
                fnIndex.write(this->magicIndexString, this->magicIndexString.length());
            }
            if(fnGames.pos() == 0) {
                fnGames.write(magicGamesString, magicGamesString.length());
            }

            std::cout << "\nsaving games: 0/"<< size;
            int i = 0;
            while(!stop) {
                if(i%100==0) {
                    std::cout << "\rsaving games: "<<offset<< "/"<<size << std::flush;
                }
                i++;
                int res = pgnReader->readNextHeader(pgnfile, encoding, &offset, header);
                if(res < 0) {
                    stop = true;
                    continue;
                }
                // the current index entry
                QByteArray iEntry;
                // first write index entry
                // status
                dcgWriter->append_as_uint8(&iEntry, quint8(0x00));
                // game offset
                dcgWriter->append_as_uint64(&iEntry, fnGames.pos());
                // white offset
                QString white = header->headers->value("White");
                quint32 whiteOffset = names->value(white);
                dcgWriter->append_as_uint32(&iEntry, whiteOffset);
                // black offset
                QString black = header->headers->value("Black");
                quint32 blackOffset = names->value(black);
                dcgWriter->append_as_uint32(&iEntry, blackOffset);
                // round
                quint16 round = header->headers->value("Round").toUInt();
                dcgWriter->append_as_uint16(&iEntry, round);
                // site offset
                quint32 site_offset = sites->value(header->headers->value("Site"));
                dcgWriter->append_as_uint32(&iEntry, site_offset);
                // elo white
                quint16 elo_white = header->headers->value("Elo White").toUInt();
                dcgWriter->append_as_uint16(&iEntry, elo_white);
                quint16 elo_black = header->headers->value("Elo White").toUInt();
                dcgWriter->append_as_uint16(&iEntry, elo_black);
                // result
                if(header->headers->contains("Result")) {
                    QString res = header->headers->value("Result");
                    if(res == "1-0") {
                        dcgWriter->append_as_uint8(&iEntry, quint8(0x01));
                    } else if(res == "0-1") {
                        dcgWriter->append_as_uint8(&iEntry, quint8(0x02));
                    } else if(res == "1/2-1/2") {
                        dcgWriter->append_as_uint8(&iEntry, quint8(0x03));
                    } else {
                        dcgWriter->append_as_uint8(&iEntry, quint8(0x00));
                    }
                } else  {
                    dcgWriter->append_as_uint8(&iEntry, quint8(0x00));
                }
                // ECO
                if(header->headers->contains("ECO")) {
                    QByteArray eco = header->headers->value("ECO").toUtf8();
                    iEntry.append(eco);
                } else {
                    QByteArray eco = QByteArrayLiteral("\x00\x00\x00");
                    iEntry.append(eco);
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
                    dcgWriter->append_as_uint16(&iEntry, year);
                    dcgWriter->append_as_uint8(&iEntry, month);
                    dcgWriter->append_as_uint8(&iEntry, day);
                } else {
                    dcgWriter->append_as_uint8(&iEntry, quint8(0x00));
                    dcgWriter->append_as_uint8(&iEntry, quint8(0x00));
                    dcgWriter->append_as_uint8(&iEntry, quint8(0x00));
                }
                assert(iEntry.size() == 35);
                fnIndex.write(iEntry, iEntry.length());
                //qDebug() << "just before reading back file";
                chess::Game *g = pgnReader->readGameFromFile(pgnfile, encoding, header->offset);
                //qDebug() << "READ file ok";
                QByteArray *g_enc = dcgWriter->encodeGame(g); //"<<<<<<<<<<<<<<<<<<<<<< this is the cause of mem acc fault"
                //qDebug() << "enc ok";
                fnGames.write(*g_enc, g_enc->length());
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
}



/*
 write sites into file
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
*/

    /*
void chess::Database::saveToFile() {

}

void chess::Database::writeSites() {

}

void chess::Database::writeNames() {

}

void chess::Database::writeIndex() {

}

void chess::Database::writeGames() {

}

*/
