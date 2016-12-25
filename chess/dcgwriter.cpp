#include "dcgwriter.h"
#include "assert.h"
#include <QDebug>

namespace chess {

// TODO: rewrite by using QDataStream. Then appending
// is simply operator <<

DcgWriter::DcgWriter()
{
    this->gameBytes = new QByteArray();
}

DcgWriter::~DcgWriter()
{
    delete this->gameBytes;
}

void DcgWriter::traverseNodes(GameNode *current) {
    int cntVar = current->getVariations()->count();

    // first handle mainline move, if there are variations
    if(cntVar > 0) {
        GameNode* main_variation = current->getVariation(0);
        this->appendMove(main_variation->getMove());
        // encode nags
        int cntNags = main_variation->getNags()->count();
        if(cntNags > 0) {
            this->appendNags(main_variation);
        }
        // encode comment, if any
        if(!main_variation->getComment().isEmpty()) {
            this->appendComment(main_variation);
        }
    }

    // now handle all variations (sidelines)
    for(int i=1;i<cntVar;i++) {
        // first create variation start marker
        GameNode *var_i = current->getVariation(i);
        this->appendStartTag();
        this->appendMove(var_i->getMove());
        // encode nags
        int cntNags = var_i->getNags()->count();
        if(cntNags > 0) {
            this->appendNags(var_i);
        }
        // encode comment, if any
        if(!var_i->getComment().isEmpty()) {
            this->appendComment(var_i);
        }
        // recursive call for all childs
        this->traverseNodes(var_i);

        // print variation end
        this->appendEndTag();
    }

    // finally do the mainline
    if(cntVar > 0) {
        GameNode* main_variation = current->getVariation(0);
        this->traverseNodes(main_variation);
    }
}

QByteArray* DcgWriter::encodeGame(Game *game) {
    //qDebug() << "encoding";
    delete this->gameBytes;
    //qDebug() << "deleted gamebytes";
    this->gameBytes = new QByteArray();
    // add fen string tag if root is not initial position
    chess::Board* root = game->getRootNode()->getBoard();
    if(!root->is_initial_position()) {
        const QByteArray fen = root->fen().toUtf8();
        int l = fen.length();
        this->gameBytes->append(quint8(0x01));
        this->appendLength(l);
        this->gameBytes->append(fen);
    } else {
        this->gameBytes->append((char) (0x00));
    }
    //qDebug() << "before traversal";
    this->traverseNodes(game->getRootNode());
    // prepend length
    int l = this->gameBytes->size();
    this->prependLength(l);
    return new QByteArray(*this->gameBytes);
}

void DcgWriter::appendMove(Move *move) {
    if(move->is_null) {
        this->gameBytes->append(quint8(0x88));
    } else {
        QPoint fromPoint = move->fromAsXY();
        QPoint toPoint = move->toAsXY();
        quint8 from = fromPoint.y() * 8 + fromPoint.x();
        qint8 to = toPoint.y() * 8 + toPoint.x();
        quint16 move_binary = qint16(to) + (quint16(from) << 6);
        if(move->promotion_piece != 0) {
            move_binary += quint16(move->promotion_piece << 12);
        }
        this->append_as_uint16(this->gameBytes, move_binary);
    }
}

void DcgWriter::appendLength(int len) {
    if(len >= 0 && len < 127) {
        this->append_as_uint8(this->gameBytes, quint8(len));
    } else if(len >= 0 && len < 255) {
        this->append_as_uint8(this->gameBytes, quint8(0x81));
        this->append_as_uint8(this->gameBytes, quint8(len));
    } else if(len >= 0 && len < 65535) {
        this->append_as_uint8(this->gameBytes, quint8(0x82));
        this->append_as_uint16(this->gameBytes, quint16(len));
    } else if(len >= 0 && len < 16777215) {
        this->append_as_uint8(this->gameBytes, quint8(0x83));
        this->append_as_uint8(this->gameBytes, quint8(len >> 16));
        this->append_as_uint16(this->gameBytes, quint16(len));
    } else if(len >= 0 && len < 4294967) {
        this->append_as_uint8(this->gameBytes, quint8(0x84));
        this->append_as_uint32(this->gameBytes, quint32(len));
    }
}

void DcgWriter::prependLength(int len) {
    if(len >= 0 && len < 127) {
        this->prepend_as_uint8(this->gameBytes, quint8(len));
    } else if(len >= 0 && len < 255) {
        this->prepend_as_uint8(this->gameBytes, quint8(0x81));
        this->prepend_as_uint8(this->gameBytes, quint8(len));
    } else if(len >= 0 && len < 65535) {
        this->prepend_as_uint8(this->gameBytes, quint8(0x82));
        this->prepend_as_uint16(this->gameBytes, quint16(len));
    } else if(len >= 0 && len < 16777215) {
        this->prepend_as_uint8(this->gameBytes, quint8(0x83));
        this->prepend_as_uint8(this->gameBytes, quint8(len >> 16));
        this->prepend_as_uint16(this->gameBytes, quint16(len));
    } else if(len >= 0 && len < 4294967) {
        this->prepend_as_uint8(this->gameBytes, quint8(0x84));
        this->prepend_as_uint32(this->gameBytes, quint32(len));
    }
}

void DcgWriter::appendNags(GameNode* node) {
    QList<int>* nags = node->getNags();
    int l = nags->length();
    if(l>0) {
        this->gameBytes->append(quint8(0x87));
        this->appendLength(l);
        for(int i=0;i<nags->length();i++) {
            quint8 nag_i = quint8(nags->at(i));
            this->gameBytes->append(nag_i);
        }
    }
}

void DcgWriter::appendComment(GameNode* node) {
    const QByteArray comment_utf8 = node->getComment().toUtf8();
    int l = comment_utf8.size();
    if(l>0) {
        this->gameBytes->append(quint8(0x86));
        this->appendLength(l);
        this->gameBytes->append(comment_utf8);
    }
}

void DcgWriter::appendStartTag() {
    this->gameBytes->append(quint8(0x84));
}

void DcgWriter::appendEndTag() {
    this->gameBytes->append(quint8(0x85));
}

void DcgWriter::append_as_uint8(QByteArray* ba, quint8 r) {
    ba->append(r);
}

void DcgWriter::append_as_uint16(QByteArray* ba, quint16 r) {
    ba->append(quint8(r>>8));
    ba->append(quint8(r));
}

void DcgWriter::append_as_uint32(QByteArray* ba, quint32 r) {
    this->append_as_uint16(ba, quint16(r>>16));
    this->append_as_uint16(ba, quint16(r));
}

void DcgWriter::append_as_uint64(QByteArray* ba, quint64 r) {
    this->append_as_uint32(ba, quint32(r>>32));
    this->append_as_uint32(ba, quint32(r));
}


void DcgWriter::prepend_as_uint8(QByteArray* ba, quint8 r) {
    ba->prepend(r);
}

void DcgWriter::prepend_as_uint16(QByteArray* ba, quint16 r) {
    ba->prepend(quint8(r>>8));
    ba->prepend(quint8(r));
}

void DcgWriter::prepend_as_uint32(QByteArray* ba, quint32 r) {
    this->prepend_as_uint16(ba, quint16(r>>16));
    this->prepend_as_uint16(ba, quint16(r));
}

void DcgWriter::prepend_as_uint64(QByteArray* ba, quint64 r) {
    this->prepend_as_uint32(ba, quint32(r>>32));
    this->prepend_as_uint32(ba, quint32(r));
}


}
