#The Simple Chess Database Format (.dc*)

    Version 1.1

    22.10.2018

## Motivation

The only interoperable format to store and exchange chess games is the PGN standard.
PGN is however very inefficient for database exchange, access, manipulation and search. 
Moroever, it is notoriously difficult to parse.

Some proprietary binary storage formats for chess games exist; they are usually the result
of a program author (commercial or open-source) creating an ad-hoc implementation and rarely
documented, if at all by some comments in the source code. Datatypes are also rarely 
standardized, and rely on the conventions by the compiler.

This standard propses a simple chess database format. It puts simplicity above _all_ other
design considerations. In particular writing an encoder/decoder should be a simple task for any program author.

It's advantages are:

* It is fully binary. Files are much smaller compared to PGN and operations on binary files are much more performant.
* It is designed to efficiently support basic database operations (e.g. efficiently deleting or replacing a game in a large database).
* It allows to create a very simple parser. There is no need e.g. for complex XML decoders or schemes.
* **No move generator / chess logic us necessary!**. No PerfT tests. No endless debugging. Not trying to recreate the precies move-generator from a specific implementation. No messing around with bitboards. This is especially important for intepreted and web-languages, where implementing
  a performant move generator is no trivial task.
* Extendibility. Database Program authors can easily add features without violating the standard, by e.g. creating/using additional indexing files or memory mapping to increase search speed for certain operations (like an opening tree etc.).
* Similar additional info (i.e. visual board annotations, images) can be stored by adding additional program-specific files without violating this standard.

These advantages come with a few distadvantages. Especially the following two should be mentioned:
* In the current standard, there is no support for Chess960, Bughouse or other variants - it's chess only. This decision is the result of trying to balance implementation complexity (to allow easy adoption) and feature-richness
* Some space is wasted compare to more optimized database implementations. Again this is a conscious decision to allow easier implementation.

Concerning the latter, moves are here stored with the originating square, the target square, and
possible promotion pieces. This simple encoding requires two bytes per move. This allows for easy parsing since no chess-logic or move generation needs to be implemented to parse and display games (think e.g. a web-interface to a database-file). Chess moves can be stored in a more optimal fashion, usually requiring only one byte.

Nevertheless the saved space is neglibile. Let's take the ChessBase Mega-Database, which stored approximately 6.7 million games and can be considered the biggest chess database available today. Assume that on average a chess game contains 40 moves. Comparing
an optimal encoding and the much simpler encoding here we have:

* 6.7 million games * 40 moves * 1 Byte per move = 268 megabytes for the optimized database and
* 6.7 million games * 40 moves * 2 Byte per move = 536 megabytes for this encoding.

As can be seen, such optimal encodings were very relevant in the age of floppy disks. But in
todays age the much simpler encoding of this standard can easily handled by even the slowest
available computers today, and can even very likely be accessed completely in-memory (in RAM) 
to speed up database operations.


## Overview

A simple chess database consists of two files:

1. an index file. The naming convenction is `database.dci`
2. a name file, containing all player names. The naming convention is `database.dcn`
3. a site file, containing all playing sites. The naming convention is `database.dcs`
4. a file containing all games, including comments
and annotation markers (such as +/-, ?! etc.).
The naming convenction is `database.dcg`

### Index File

An index file containging N games has the following format:

    [ MagicBytesIndex  |  Version Number | IndexEntry #1 | IndexEntry #2 | ... | IndexEntry #N ]
      10 Bytes            1 Byte           61 Byte         61 Byte               61 Byte

The next sections describe the above blocks in details.

**MagicBytesIndex**

The ten byte sequence

    0x53 0x69 0xed 0x70 0x6c 0x65 0x43 0x44 0x62 0x69
i.e. "SimpleCDbi" in ASCII without a string terminator.

**Version Number**

One byte `0x01` to denote the version of this document.

**IndexEntry**

    IndexEntry = [ Status | Offset | WhiteRef | BlackRef   | Round   | SiteRef | EventRef | Elo White | Elo Black  | Result | ECO    | Year   | Month | Day | Halfmoves | FinPosMaterial | PawnMoveData  ]
                   1 Byte   8 Byte   4 Byte     4 Byte       2 Byte    4 Byte    4 Byte     2 Byte      2 Byte       1 Byte   3 Byte   2 Byte   1 Byte  1 Byte  2 Byte  4 Byte  9 Byte

Games may not be deleted immediately to speed up writing out changes made by a user. Hence games
can be marked as deleted, add a game with the applied changes at the end of the file,
and then in a separate step the database can be compacted by removing
such marked games (potentially requiring lots of disk-intensive rewrites). The status byte is used to mark this.

**Status**
Status is `0x00` normally. Otherwise status is `0xFF` if the game is marked for deletion

**Offset**
Offset is a unisgned int64 denoting the position (offset) of the game in the game file (i.e. database.dcg)

**WhiteRef**
Offset (unsigned 32-bit integer) pointing to the White player entry in `database.dcn`.

**BlackRef**
Offset (unsigned 32-bit integer) pointing to the White player entry in `database.dcn`.

**Round**
unsigned 16 bit integer denoting the round the game was played in. `0x00` if unknown.

**SiteRef**
Offset (unsigned 32-bit integer) pointing to the White player entry in `database.dcn`.

**Elo White**
Elo White is an unsigend 16 bit integer denoting the ELO number of the White player

**Elo Black**
Elo Black is an unsigend 16 bit integer denoting the ELO number of the Black player

**Result**
Result denotes the game result:

- `0x00` = result undefined
- `0x01` = White wins
- `0x02` = Black wins
- `0x03` = Draw

**ECO**
ECO: 3 ASCII characters (not 0-terminated) denoting the ECO code of the game

**Year**
Year: unsigned 16 bit integer denoting the year the game was played. `0x00` if unknown

**Month**
Month: unisgned 8 bit integer denoting the month the game was played. `0x00` if unknown

**Day**
Day: unsigned 8 bit integer denoting the day the game was played. `0x00` if unknown.

**Halfmoves**
unsigned 16 bit integer denoting the number of half moves of the game.

**FinPosMaterial**
unsigned 32 bit integer denoting the material of the final position of the game. This information can be
used to speed up position searches. The encoding is as follows:

    Bit Position  0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
    -----------------------------------------------------------------------------
    Meaning/Value AVL BP1 BP2 BP3 BN1 BN2 BB1 BB2 BR1 BR2 BQ1 BQ2 RFU RFU RFU RFU
    
    Bit Position  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31
    -----------------------------------------------------------------------------
    Meaning/Value RFU WP1 WP2 WP3 WN1 WN2 WB1 WB2 WR1 WR2 WQ1 WQ2 RFU RFU RFU RFU
    
Here AVL is set to 1, if FinPosMaterial stores the information of the material count of the last position. 
It is set to 0, if this information is not available for this index entry. RFU denotes bit positions
reserved for future use, i.e. their meaning is undefined.

For the rest of the values, the bis sequences are interpreted as an unsigned 
integers (Big Endian, i.e. BP1, BN1, etc. is the highest bit). They encode the material count
in the final position of the game.

- [ BP1 BP2 BP3 ] : The number of black pawns
- [ BN1 BN2 ] : The number of black knights
- [ BB1 BB2 ] : The number of black bishops
- [ BR1 BR2 ] : The number of black rooks
- [ BQ1 BQ2 ] : The number of black queens

- [ WP1 WP2 WP3 ] : The number of white pawns
- [ WN1 WN2 ] : The number of white knights
- [ WB1 WB2 ] : The number of white bishops
- [ WR1 WR2 ] : The number of white rooks
- [ WQ1 WQ2 ] : The number of white queens

According to this encoding, there can be at most 8 pawns in the position, but each peach at most 4 times.  
If the final position of the game contains a material count that cannot be encoded (for example there are 5 white queens
on the board), AVL must be set to 0.

**PawnMoveData**
A 16 byte array. Stores the sequence of how pawns in the game left their initial position. This information can be
used to speed up position searches.

    [ WhiteFirstMovedPawn | BlackFirstMovedPawn | WhiteSecondMovedPawn | ... | BlackEigthMovedPawn ]

where the pawns are encoded as follows:

- 0x00: White Pawn on A2
- 0x01: White Pawn on B2
- 0x02: White Pawn on C2
- 0x03: White Pawn on D2
- 0x04: White Pawn on E2
- 0x05: White Pawn on F2
- 0x06: White Pawn on G2
- 0x07: White Pawn on H2
- 0x08: Black Pawn on A7
- 0x09: Black Pawn on B7
- 0x0A: Black Pawn on C7
- 0x0B: Black Pawn on D7
- 0x0C: Black Pawn on E7
- 0x0D: Black Pawn on F7
- 0x0E: Black Pawn on G7
- 0x0F: Black Pawn on H7
- 0x10: No information available

If any byte is set to 0x10, any bytes right from that index position must be set to 0x10 as well.

Example:

Consider the game 1. g4 e5 2. f3 Qh4#. The corresponding array is [ 0x06, 0x0C, 0x05, 0x10, ... 0x10 ].

## Name File

The name file consists of a sequence of names:

    [ MagicBytesName | Name#1 | Name#2 | ... | Name #N ]

**MagicBytesName**

The ten byte sequence

    \x53\x69\x6d\x70\x6c\x65\x43\x44\x62\x6e
i.e. "SimpleCDbn" in ASCII without a string terminator.

**Name**

Name is a fixed sequence of 36 Bytes, with the name of the player encoded in UTF-8. If the 
name is less than 36 Bytes, the name is padded with spaces (`0x20`).

## Site File

The name file consists of a sequence of names:

    [ MagicBytesName | Site#1 | Site#2 | ... | Site #N ]

**MagicBytesSite**

The ten byte sequence

    0x53 0x69 0xed 0x70 0x6c 0x65 0x43 0x44 0x62 0x73
i.e. "SimpleCDbs" in ASCII without a string terminator.

**Site**

Site is a fixed sequence of 36 Bytes, with the site (location) of the tournament encoded in UTF-8. If the 
site is less than 36 Bytes, the name is padded with spaces (`0x20`).

#### MagicBytesGame

The nince byte sequence

    0x53 0x69 0xed 0x70 0x6c 0x65 0x43 0x44 0x62 0x67
i.e. "SimpleCDbg" in ASCII without a string terminator.


## Database File

The database file is a sequence of N games:

    [ MagicBytesGame | Game #1 | Game #2 | ... | Game #N ]

A game is build as

    Game = [ GameLength [FenMarker or FenMarker | FenLen | Fen] 
                          [ Move or 
                           BeginOfVariation or 
		    		       EndOfVariation or 
			    	       [ StartofComment CommentLength Comment] or 
				           [ AnnotationsFollow AnnotationLength Annotations ] or
				           NullMove
				         ]
    	   ]

**MagicBytesGame**

The nince byte sequence

    0x53 0x69 0xed 0x70 0x6c 0x65 0x43 0x44 0x62
i.e. "SimpleCDbg" in ASCII without a string terminator.

**GameLength**

GameLength is a sequence of bytes denoting the length of one Game (the number of bytes
the record consists of).
GameLength consists of one, two, three, four or five bytes, depending
on the number range it encodes. It uses the BER-TLV encoding scheme
of as defined in ISO/IEC 7816.

Length    |  1st Byte    |       2nd Byte |   3rd Byte  |   4th Byte  |     5th Byte  |      Number Range 
----------|--------------|----------------|-------------|-------------|---------------|-------------------
1 Byte    | 0x00 to 0x7F |  -             | -           |     -       |     -         |   0 to 127
2 Bytes   | 0x81         |  0x00 to 0xFF  | -           | -           |     -         |   0 to 255
3 Bytes   | 0x82         |  0x0000        | to 0xFFFF   |    -        | -             |   0 to 65535
4 Bytes   | 0x83         |  0x000000      | to          | 0xFFFFFF    |               | 0 to 16777215
5 Bytes   | 0x84         |  0x00000000    | to          |             | 0xFFFFFFFF    | 0 to 4 294 967 295

This means that if all the encoded moves, comments and annotations make up less than 127 byte, then
GameLength is a single byte value. If the encoded moves, comments and annotations make up 129 byte, then
GameLength uses two bytes, and so on.

**FenMarker**

FenMarker is one byte. If the game starts from the initial position, then FenMarker is 0x00.
If the game does not start from the initial position, then FenMarker is 0x01. If FenMarker is 0x01,
FenLen and Fen MUST follow.

**FenLen**
The length of Fen using the BER-TLV encoding.

**Fen**
The FEN string of the initial position, encoded as UTF-8.

**Move**

Move is a two byte value. To understand the encoding, first we enumerate the fields on a chessboard like this:

      a    b     c    d    e    f    g    h
    ------------------------------------------  
    | 56 |  57 | 58 | 59 | 60 | 61 | 62 | 63 |  8
    |    |     |    |    |    |    | 54 | 55 |  7
    |    |     |    |    |    |    |    |    |  6
    |    |     |    |    |    |    |    |    |  5
    |    |     |    |    |    |    |    |    |  4
    |    |     |    |    |    |    |    |    |  3
    |  8 |   9 | 10 | 11 | 12 | 13 | 14 | 15 |  2
    |  0 |   1 |  2 |  3 |  4 |  5 |  6 |  7 |  1
    ------------------------------------------

In other words we start counting at the lower left corner from White's perspective (i.e. A1) and 
count right-upward.

Below we list the meaning of the bits of the two bytes. We use Big Endian encoding, 
i.e. 0 denotes the hightest bit.

Move encoding

    Bit Position  0      1   2   3   4   5   6   7   8   9   10   11  12  13  14  15  
    ---------------------------------------------------------------------------------
    Meaning/Value 0x00   RFU PR1 PR2 F1  F2  F3  F4  F5  F6  T1   T2  T3  T4  T5  T6

The bit at position 0 is always zero.

The bit at position 1 is reserved for future use. It's value is not defined.

The bits [PR1 PR2] interpreted as an unsigned integer (Big Endian, i.e. PR1 is the highest bit) denote
piece promotion as follows:

- 0: no promotion
- 1: promote to Knight
- 2: promote to Bishop
- 3: promote to Rook
- 4: promote to Queen

The bits [ F1 F2 F3 F4 F5 F6 ] interpreted as an unsigned integer (Big Endian, i.e. F1 is the highest bit) denote the field where the piece moves _from_, i.e. 000001 denotes A1, 111111 denotes H8 and so on.

The bits [ T1 T2 T3 T4 T5 T6 ] interpreted as an unsigned integer (Big Endian, i.e. F1 is the highest bit) denote the field where the piece moves _to_, i.e. 000001 denotes A1, 111111 denotes H8 and so on.


**BeginOfVariation**
This is the single byte 0x80. Note that this is the bit sequence 10000000. We can always distinguish
tags like BeginOfVariation, EndOfVariation, StartOfComment, AnnotationFollows and NullMove by checking
the highest bit. If that bit is 0, we have the start of a two byte move sequence. Otherwise we have
a one byte tag.

**EndOfVariation**
Single byte 0x85 (bit sequence 10000101).

**StartOfComment**
Single byte 0x86 (bit sequence 10000110).

**AnnotationsFollow**
Single byte 0x87 (bit sequence 10000111).

**NullMove**
Single byte 0x88 (bit sequence 10001000).

**CommentLength**
BER-TLV length value of the following comment (a comment MUST follow). See GameLength
for the precise encoding.

**Comment**
Comment is a UTF8-encoded string  

**AnnotationLength**
BER-TLV length value of the following annotations (at least one annotation must follow). See GameLength
for the precise encoding.

**Annotations**
is a sequence of bytes. One byte corresponds to one annotation. The annoation relates to the last
move that occured before the annotation in the byte stream of a Game.
The encoding of the annotation (codes) are a subset of the Numeric Annotation Glyphs (NAGs) of the
PGN standard. The integer values below are stored as an unsigned 8-bit integer.

*NAG_GOOD_MOVE = 1*
A good move. Can also be indicated by ``!`` in PGN notation.

*NAG_MISTAKE = 2*
A mistake. Can also be indicated by ``?`` in PGN notation.

*NAG_BRILLIANT_MOVE = 3*
A brilliant move. Can also be indicated by ``!!`` in PGN notation.

*NAG_BLUNDER = 4*
A blunder. Can also be indicated by ``??`` in PGN notation.

*NAG_SPECULATIVE_MOVE = 5*
A speculative move. Can also be indicated by ``!?`` in PGN notation.

*NAG_DUBIOUS_MOVE = 6*
A dubious move. Can also be indicated by ``?!`` in PGN notation.

*NAG_FORCED_MOVE = 7*

*NAG_DRAWISH_POSITION = 10*

*NAG_UNCLEAR_POSITION = 13*

*NAG_WHITE_MODERATE_ADVANTAGE = 16*

*NAG_BLACK_MODERATE_ADVANTAGE = 17*

*NAG_WHITE_DECISIVE_ADVANTAGE = 18*

*NAG_BLACK_DECISIVE_ADVANTAGE = 19*

*NAG_WHITE_ZUGZWANG = 22*

*NAG_BLACK_ZUGZWANG = 23*

*NAG_WHITE_HAS_ATTACK = 40*

*NAG_BLACK_HAS_ATTACK = 41*

*NAG_WHITE_MODERATE_COUNTERPLAY = 132*

*NAG_BLACK_MODERATE_COUNTERPLAY = 133*
