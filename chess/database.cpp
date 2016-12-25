#include "database.h"
#include "chess/game.h"
#include "chess/pgn_reader.h"
#include <iostream>

chess::Database::Database(QString &filename)
{
    this->filename = filename;
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

