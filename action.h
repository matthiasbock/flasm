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

typedef    enum
{
    SWFACTION_END                       = 0x00,

/* v3 actions */
    SWFACTION_NEXTFRAME                 = 0x04,
    SWFACTION_PREVFRAME                 = 0x05,
    SWFACTION_PLAY                      = 0x06,
    SWFACTION_STOP                      = 0x07,
    SWFACTION_TOGGLEQUALITY             = 0x08,
    SWFACTION_STOPSOUNDS                = 0x09,
    SWFACTION_GOTOFRAME                 = 0x81,    /* >= 0x80 means record has args */
    SWFACTION_GETURL                    = 0x83,
    SWFACTION_IFFRAMELOADED             = 0x8A,
    SWFACTION_SETTARGET                 = 0x8B,
    SWFACTION_GOTOLABEL                 = 0x8C,

/* v4 actions */
    SWFACTION_ADD                       = 0x0A,
    SWFACTION_SUBTRACT                  = 0x0B,
    SWFACTION_MULTIPLY                  = 0x0C,
    SWFACTION_DIVIDE                    = 0x0D,
    SWFACTION_EQUALS                    = 0x0E,
    SWFACTION_LESSTHAN                  = 0x0F,
    SWFACTION_LOGICALAND                = 0x10,
    SWFACTION_LOGICALOR                 = 0x11,
    SWFACTION_LOGICALNOT                = 0x12,
    SWFACTION_STRINGEQ                  = 0x13,
    SWFACTION_STRINGLENGTH              = 0x14,
    SWFACTION_SUBSTRING                 = 0x15,
    SWFACTION_POP                       = 0x17,
    SWFACTION_INT                       = 0x18,
    SWFACTION_GETVARIABLE               = 0x1C,
    SWFACTION_SETVARIABLE               = 0x1D,
    SWFACTION_SETTARGETEXPRESSION       = 0x20,
    SWFACTION_STRINGCONCAT              = 0x21,
    SWFACTION_GETPROPERTY               = 0x22,
    SWFACTION_SETPROPERTY               = 0x23,
    SWFACTION_DUPLICATECLIP             = 0x24,
    SWFACTION_REMOVECLIP                = 0x25,
    SWFACTION_TRACE                     = 0x26,
    SWFACTION_STARTDRAGMOVIE            = 0x27,
    SWFACTION_STOPDRAGMOVIE             = 0x28,
    SWFACTION_STRINGLESSTHAN            = 0x29,
    SWFACTION_RANDOM                    = 0x30,
    SWFACTION_MBLENGTH                  = 0x31,
    SWFACTION_ORD                       = 0x32,
    SWFACTION_CHR                       = 0x33,
    SWFACTION_GETTIMER                  = 0x34,
    SWFACTION_MBSUBSTRING               = 0x35,
    SWFACTION_MBORD                     = 0x36,
    SWFACTION_MBCHR                     = 0x37,
    SWFACTION_IFFRAMELOADEDEXPRESSION   = 0x8D,
    SWFACTION_PUSHDATA                  = 0x96,
    SWFACTION_BRANCHALWAYS              = 0x99,
    SWFACTION_GETURL2                   = 0x9A,
    SWFACTION_BRANCHIFTRUE              = 0x9D,
    SWFACTION_CALLFRAME                 = 0x9E,
    SWFACTION_GOTOEXPRESSION            = 0x9F,

/* v5 actions */
    SWFACTION_DELETE                    = 0x3A,
    SWFACTION_DELETE2                   = 0x3B,
    SWFACTION_VAREQUALS                 = 0x3C,
    SWFACTION_CALLFUNCTION              = 0x3D,
    SWFACTION_RETURN                    = 0x3E,
    SWFACTION_MODULO                    = 0x3F,
    SWFACTION_NEW                       = 0x40,
    SWFACTION_VAR                       = 0x41,
    SWFACTION_INITARRAY                 = 0x42,
    SWFACTION_INITOBJECT                = 0x43,
    SWFACTION_TYPEOF                    = 0x44,
    SWFACTION_TARGETPATH                = 0x45,
    SWFACTION_ENUMERATE                 = 0x46,
    SWFACTION_NEWADD                    = 0x47,
    SWFACTION_NEWLESSTHAN               = 0x48,
    SWFACTION_NEWEQUALS                 = 0x49,
    SWFACTION_TONUMBER                  = 0x4A,
    SWFACTION_TOSTRING                  = 0x4B,
    SWFACTION_DUP                       = 0x4C,
    SWFACTION_SWAP                      = 0x4D,
    SWFACTION_GETMEMBER                 = 0x4E,
    SWFACTION_SETMEMBER                 = 0x4F,
    SWFACTION_INCREMENT                 = 0x50,
    SWFACTION_DECREMENT                 = 0x51,
    SWFACTION_CALLMETHOD                = 0x52,
    SWFACTION_NEWMETHOD                 = 0x53,
    SWFACTION_BITWISEAND                = 0x60,
    SWFACTION_BITWISEOR                 = 0x61,
    SWFACTION_BITWISEXOR                = 0x62,
    SWFACTION_SHIFTLEFT                 = 0x63,
    SWFACTION_SHIFTRIGHT                = 0x64,
    SWFACTION_SHIFTRIGHT2               = 0x65,
    SWFACTION_SETREGISTER               = 0x87,
    SWFACTION_CONSTANTPOOL              = 0x88,
    SWFACTION_WITH                      = 0x94,
    SWFACTION_DEFINEFUNCTION            = 0x9B,

/* v6 actions */
    SWFACTION_INSTANCEOF                = 0x54,
    SWFACTION_ENUMERATEVALUE            = 0x55,
    SWFACTION_STRICTEQUALS              = 0x66,
    SWFACTION_GREATERTHAN               = 0x67,
    SWFACTION_STRINGGREATERTHAN         = 0x68,
    SWFACTION_STRICTMODE                = 0x89,

/* v7 actions */
    SWFACTION_CAST                      = 0x2B,
    SWFACTION_IMPLEMENTS                = 0x2C,
    SWFACTION_EXTENDS                   = 0x69,
    SWFACTION_DEFINEFUNCTION2           = 0x8E,
    SWFACTION_TRY                       = 0x8F,
    SWFACTION_THROW                     = 0x2A,

/* FlashLite */
    SWFACTION_FSCOMMAND2                = 0x2D
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
    ATTR_USENETWORK                     = 0x01,
    ATTR_RELATIVEURLS                   = 0x02,
    ATTR_SUPPRESSCROSSDOMAINCACHE       = 0x04,
    ATTR_ACTIONSCRIPT3                  = 0x08,                 
    ATTR_HASMETADATA                    = 0x10
} fileattributes;

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
    TAG_DOINITACTION                    = 59, /* flash 6 mc initialization actions (#initclip .. #endinitclip) */
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
    TAG_DOABCDEFINE                     = 72, /* not mentioned in Adobe's specification; http://www.m2osw.com/swf_tags */ 
    TAG_DEFINEFONTINFO3                 = 73,
    TAG_DEFINETEXTINFO                  = 74,
    TAG_DEFINEFONT3                     = 75,
    TAG_SYMBOLCLASS                     = 76,
    TAG_METADATA                        = 77,
    TAG_SLICE9                          = 78,
    
    TAG_AVM2ACTION                      = 82,
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

/* action block type - all places in SWF that may contain actions */
typedef enum 
{
    AB_FRAME,
    AB_INITMC,
    AB_MCEVENT,
    AB_BUTTONEVENT
} abtype;

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
