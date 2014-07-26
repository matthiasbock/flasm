/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan, (c) 2005 Wang Zhen
All rights reserved. See LICENSE.TXT for terms of use.
*/

#ifndef    ACTION_H_INCLUDED
#define    ACTION_H_INCLUDED

/* math */
#define MATH_LOG2E                      1.442695040888963
#define MATH_LOG10E                     0.4342944819032518
#define MATH_LN2                        0.6931471805599453
#define MATH_LN10                       2.302585092994046
#define MATH_PI                         3.141592653589793
#define MATH_SQRT1_2                    0.7071067811865476
#define MATH_SQRT2                      1.414213562373095
#define MATH_DELTA                      10000000000.0
#define NUMBER_MAX_VALUE                1.79769313486231e+308
#define NUMBER_MIN_VALUE                5.e-324

/*
 * An SWF consists of a header followed by a series of blocks of a certain type ("tags"),
 * see SWF 19 spec page 28.
 *
 * Definition tags contain resources like bitmaps etc.
 * Control tags contain instructions.
 */
typedef    enum
{
    TAG_END                             = 0,  /* end tag for movie clip or swf */
    TAG_SHOWFRAME                       = 1,  /* frame is completely described now, please show */
    TAG_DEFINESHAPE                     = 2,
    TAG_FREECHARACTER                   = 3,
    TAG_PLACEOBJECT                     = 4,
    TAG_REMOVEOBJECT                    = 5,
    TAG_DEFINEBITS                      = 6,
    TAG_DEFINEBUTTON                    = 7,
    TAG_JPEGTABLES                      = 8,
    TAG_SETBACKGROUNDCOLOR              = 9,
    TAG_DEFINEFONT                      = 10,
    TAG_DEFINETEXT                      = 11,
    TAG_DOACTION                        = 12, /* normal action block */
    TAG_DEFINEFONTINFO                  = 13,
    TAG_DEFINESOUND                     = 14,
    TAG_STARTSOUND                      = 15,
    TAG_STOPSOUND                       = 16,
    TAG_DEFINEBUTTONSOUND               = 17,
    TAG_SOUNDSTREAMHEAD                 = 18,
    TAG_SOUNDSTREAMBLOCK                = 19,
    TAG_DEFINEBITSLOSSLESS              = 20,
    TAG_DEFINEBITSJPEG2                 = 21,
    TAG_DEFINESHAPE2                    = 22,
    TAG_DEFINEBUTTONCXFORM              = 23,
    TAG_PROTECT                         = 24, /* the author doesn't want the file to be opened */
    TAG_PATHSAREPOSTSCRIPT              = 25,
    TAG_PLACEOBJECT2                    = 26, /* possibly onClipEvents inside */

    TAG_REMOVEOBJECT2                   = 28,
    TAG_SYNCFRAME                       = 29,

    TAG_FREEALL                         = 31,
    TAG_DEFINESHAPE3                    = 32,
    TAG_DEFINETEXT2                     = 33,
    TAG_DEFINEBUTTON2                   = 34, /* possibly button events inside */
    TAG_DEFINEBITSJPEG3                 = 35,
    TAG_DEFINEBITSLOSSLESS2             = 36,
    TAG_DEFINEEDITTEXT                  = 37,
    TAG_DEFINEVIDEO                     = 38,
    TAG_DEFINEMOVIECLIP                 = 39, /* movie clip timeline comes */
    TAG_NAMECHARACTER                   = 40,
    TAG_SERIALNUMBER                    = 41,
    TAG_DEFINETEXTFORMAT                = 42,
    TAG_FRAMELABEL                      = 43,
    TAG_SOUNDSTREAMHEAD2                = 45,
    TAG_DEFINEMORPHSHAPE                = 46,
    TAG_GENFRAME                        = 47,
    TAG_DEFINEFONT2                     = 48,
    TAG_GENCOMMAND                      = 49,
    TAG_DEFINECOMMANDOBJ                = 50,
    TAG_CHARACTERSET                    = 51,
    TAG_FONTREF                         = 52,

    TAG_EXPORTASSETS                    = 56,
    TAG_IMPORTASSETS                    = 57,
    TAG_ENABLEDEBUGGER                  = 58,
    TAG_DOINITACTION                    = 59, /* formerly TAG_INITMOVIECLIP; flash 6 mc initialization actions (#initclip .. #endinitclip) */
    TAG_DEFINEVIDEOSTREAM               = 60,
    TAG_VIDEOFRAME                      = 61,
    TAG_DEFINEFONTINFO2                 = 62,
    TAG_DEBUGID                         = 63,
    TAG_ENABLEDEBUGGER2                 = 64,
    TAG_SCRIPTLIMITS                    = 65,
    TAG_SETTABINDEX                     = 66,
    TAG_DEFINESHAPE4                    = 67,

    TAG_FILEATTRIBUTES                  = 69,
    TAG_PLACEOBJECT3                    = 70, /* possibly onClipEvents inside */
    TAG_IMPORTASSETS2                   = 71,
    TAG_DOABCDEFINE                     = 72, /* not mentioned in Adobe's specification; from http://www.m2osw.com/swf_tags */
    TAG_DEFINEFONTINFO3                 = 73,
    TAG_DEFINETEXTINFO                  = 74,
    TAG_DEFINEFONT3                     = 75,
    TAG_SYMBOLCLASS                     = 76,
    TAG_METADATA                        = 77,
    TAG_SLICE9                          = 78,

    TAG_DOABC                           = 82, /* formerly named TAG_AVM2ACTION; new since SWF 9 */
    TAG_DEFINESHAPE5                    = 83,
    TAG_DEFINEMORPHSHAPE2               = 84,

    TAG_DEFINESCENEANDFRAMELABELDATA    = 86,
    TAG_DEFINEBINARYDATA                = 87,
    TAG_DEFINEFONTNAME                  = 88,
    TAG_STARTSOUND2                     = 89,
    TAG_DEFINEBITSJPEG4                 = 90,
    TAG_DEFINEFONT4                     = 91,
    TAG_ENABLETELEMETRY                 = 93,

    TAG_DEFINEBITSPTR                   = 1023
} tagheaderid;

/*
 * First tag of an SWF is usually the FileAttributes tag,
 * mandatorily since SWF 8.
 */
typedef    enum
{
    ATTR_USENETWORK                     = 0x01,
    ATTR_RELATIVEURLS                   = 0x02,
    ATTR_SUPPRESSCROSSDOMAINCACHE       = 0x04,
    ATTR_ACTIONSCRIPT3                  = 0x08,
    ATTR_HASMETADATA                    = 0x10
} fileattributes;

/*
 * Action block types:
 * all places in the SWF that may contain actions
 * */
typedef enum
{
    AB_FRAME,
    AB_INITMC,
    AB_MCEVENT,
    AB_BUTTONEVENT
} abtype;

/*
 * ActionScript instructions
 */
typedef    enum
{
    ACTION_END                       = 0x00,

/* v3 actions */
    ACTION_NEXTFRAME                 = 0x04,
    ACTION_PREVFRAME                 = 0x05,
    ACTION_PLAY                      = 0x06,
    ACTION_STOP                      = 0x07,
    ACTION_TOGGLEQUALITY             = 0x08,
    ACTION_STOPSOUNDS                = 0x09,
    ACTION_GOTOFRAME                 = 0x81,    /* >= 0x80 means record has args */
    ACTION_GETURL                    = 0x83,
    ACTION_IFFRAMELOADED             = 0x8A,
    ACTION_SETTARGET                 = 0x8B,
    ACTION_GOTOLABEL                 = 0x8C,

/* v4 actions */
    ACTION_ADD                       = 0x0A,
    ACTION_SUBTRACT                  = 0x0B,
    ACTION_MULTIPLY                  = 0x0C,
    ACTION_DIVIDE                    = 0x0D,
    ACTION_EQUALS                    = 0x0E,
    ACTION_LESSTHAN                  = 0x0F,
    ACTION_LOGICALAND                = 0x10,
    ACTION_LOGICALOR                 = 0x11,
    ACTION_LOGICALNOT                = 0x12,
    ACTION_STRINGEQ                  = 0x13,
    ACTION_STRINGLENGTH              = 0x14,
    ACTION_SUBSTRING                 = 0x15,
    ACTION_POP                       = 0x17,
    ACTION_INT                       = 0x18,
    ACTION_GETVARIABLE               = 0x1C,
    ACTION_SETVARIABLE               = 0x1D,
    ACTION_SETTARGETEXPRESSION       = 0x20,
    ACTION_STRINGCONCAT              = 0x21,
    ACTION_GETPROPERTY               = 0x22,
    ACTION_SETPROPERTY               = 0x23,
    ACTION_DUPLICATESPRITE           = 0x24,
    ACTION_REMOVESPRITE              = 0x25,
    ACTION_TRACE                     = 0x26,
    ACTION_STARTDRAGMOVIE            = 0x27,
    ACTION_STOPDRAGMOVIE             = 0x28,
    ACTION_STRINGLESSTHAN            = 0x29,
    ACTION_RANDOM                    = 0x30,
    ACTION_MBLENGTH                  = 0x31,
    ACTION_ORD                       = 0x32,
    ACTION_CHR                       = 0x33,
    ACTION_GETTIMER                  = 0x34,
    ACTION_MBSUBSTRING               = 0x35,
    ACTION_MBORD                     = 0x36,
    ACTION_MBCHR                     = 0x37,
    ACTION_IFFRAMELOADEDEXPRESSION   = 0x8D,
    ACTION_PUSHDATA                  = 0x96,
    ACTION_BRANCHALWAYS              = 0x99,
    ACTION_GETURL2                   = 0x9A,
    ACTION_BRANCHIFTRUE              = 0x9D,
    ACTION_CALLFRAME                 = 0x9E,
    ACTION_GOTOEXPRESSION            = 0x9F,

/* v5 actions */
    ACTION_DELETE                    = 0x3A,
    ACTION_DELETE2                   = 0x3B,
    ACTION_VAREQUALS                 = 0x3C,
    ACTION_CALLFUNCTION              = 0x3D,
    ACTION_RETURN                    = 0x3E,
    ACTION_MODULO                    = 0x3F,
    ACTION_NEW                       = 0x40,
    ACTION_VAR                       = 0x41,
    ACTION_INITARRAY                 = 0x42,
    ACTION_INITOBJECT                = 0x43,
    ACTION_TYPEOF                    = 0x44,
    ACTION_TARGETPATH                = 0x45,
    ACTION_ENUMERATE                 = 0x46,
    ACTION_NEWADD                    = 0x47,
    ACTION_NEWLESSTHAN               = 0x48,
    ACTION_NEWEQUALS                 = 0x49,
    ACTION_TONUMBER                  = 0x4A,
    ACTION_TOSTRING                  = 0x4B,
    ACTION_DUP                       = 0x4C,
    ACTION_SWAP                      = 0x4D,
    ACTION_GETMEMBER                 = 0x4E,
    ACTION_SETMEMBER                 = 0x4F,
    ACTION_INCREMENT                 = 0x50,
    ACTION_DECREMENT                 = 0x51,
    ACTION_CALLMETHOD                = 0x52,
    ACTION_NEWMETHOD                 = 0x53,
    ACTION_BITWISEAND                = 0x60,
    ACTION_BITWISEOR                 = 0x61,
    ACTION_BITWISEXOR                = 0x62,
    ACTION_SHIFTLEFT                 = 0x63,
    ACTION_SHIFTRIGHT                = 0x64,
    ACTION_SHIFTRIGHT2               = 0x65,
    ACTION_SETREGISTER               = 0x87,
    ACTION_CONSTANTPOOL              = 0x88,
    ACTION_WITH                      = 0x94,
    ACTION_DEFINEFUNCTION            = 0x9B,

/* v6 actions */
    ACTION_INSTANCEOF                = 0x54,
    ACTION_ENUMERATEVALUE            = 0x55,
    ACTION_STRICTEQUALS              = 0x66,
    ACTION_GREATERTHAN               = 0x67,
    ACTION_STRINGGREATERTHAN         = 0x68,
    ACTION_STRICTMODE                = 0x89,

/* v7 actions */
    ACTION_CAST                      = 0x2B,
    ACTION_IMPLEMENTS                = 0x2C,
    ACTION_EXTENDS                   = 0x69,
    ACTION_DEFINEFUNCTION2           = 0x8E,
    ACTION_TRY                       = 0x8F,
    ACTION_THROW                     = 0x2A,

/* FlashLite */
    ACTION_FSCOMMAND2                = 0x2D
} Action;

typedef    enum
{
    PF_MOVE                             = 0x01,  /* this place moves an exisiting object */
    PF_CHARACTER                        = 0x02,  /* there is a character tag (if no tag, must be a move) */
    PF_MATRIX                           = 0x04,  /* there is a matrix (matrix) */
    PF_COLORTRANSFORM                   = 0x08,  /* there is a color transform (cxform    with alpha) */
    PF_RATIO                            = 0x10,  /* there is a blend ratio (word) */
    PF_NAME                             = 0x20,  /* there is an object name (string) */
    PF_DEFINECLIP                       = 0x40,  /* this shape should open or close a clipping bracket (character != 0 to open, character == 0 to close) */
    PF_ONCLIPEVENTS                     = 0x80,  /* there are onClipEvents */
    PF_FILTERS                          = 0x100, /* there are filters */
    PF_BLENDMODE                        = 0x200, /* there is a blend mode */
    PF_BITMAPCACHING                    = 0x400  /* use runtime bitmap caching */
} placeflags;

typedef    enum
{
    FILTER_DROPSHADOW                   = 0,
    FILTER_BLUR                         = 1,
    FILTER_GLOW                         = 2,
    FILTER_BEVEL                        = 3,
    FILTER_GRADIENTGLOW                 = 4,
    FILTER_ADJUSTCOLOR                  = 6,
    FILTER_GRADIENTBEVEL                = 7
} filtertype;

typedef    enum
{
    MODE_DECOMPRESS,
    MODE_COMPRESS,
    MODE_ASSEMBLE,
    MODE_ASBYTECODE,
    MODE_DISASSEMBLE,
    MODE_UPDATE,
    MODE_IDE,
    MODE_FLASH_HELP
} processingmode;

#endif /* ACTION_H_INCLUDED    */
