#ifndef DCGWRITER_H
#define DCGWRITER_H

#include <QByteArray>
#include <QQueue>
#include "game.h"

namespace chess {

class DcgWriter
{
public:
    DcgWriter();
    ~DcgWriter();
    QByteArray* encodeGame(Game *game);
    QByteArray* encodeHeader();
    void traverseNodes(GameNode *current);
    void reset();

    void appendMove(Move *move);
    void appendLength(int len);
    void prependLength(int len);
    void appendNags(GameNode* node);
    void appendComment(GameNode* node);

    void appendStartTag();
    void appendEndTag();

    void append_as_uint8(QByteArray* ba, quint8 val);
    void append_as_uint16(QByteArray* ba, quint16 val);
    void append_as_uint32(QByteArray* ba, quint32 val);
    void append_as_uint64(QByteArray* ba, quint64 val);

    void prepend_as_uint8(QByteArray* ba, quint8 val);
    void prepend_as_uint16(QByteArray* ba, quint16 val);
    void prepend_as_uint32(QByteArray* ba, quint32 val);
    void prepend_as_uint64(QByteArray* ba, quint64 val);

    Game* decodeGame();

private:
    QByteArray* gameBytes;

};

}

#endif // DCGWRITER_H
