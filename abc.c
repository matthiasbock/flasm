/*
flasm, command line assembler & disassembler of flash actionscript bytecode
Copyright (c) 2001 Opaque Industries, (c) 2002-2007 Igor Kogan, (c) 2005 Wang Zhen
All rights reserved. See LICENSE.TXT for terms of use.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#include "abcaction.h"
#include "util.h"
#include "unflasm.h"
#include "action.h"

extern int byteorder;
extern int indent;
extern byte *buffer;

static byte *abc;
static unsigned long int pos;

static char strbuf[MAX_BUFFER];
static unsigned long int strbufpos = 0;

// AS3 constant pool
struct _namespace {
    byte kind;
    unsigned int name;
};

struct _namespaceset {
    unsigned int count;
    unsigned int ns[100];
};

struct _multiname {
	byte kind;
	unsigned int name;
	unsigned int ref;
};

struct _methodinfo {
	unsigned int numparams;
	unsigned int returntype;
	unsigned int name;
	byte flags;
};

struct _optionalparam {
	byte type;
	unsigned int value;
};

int cpints[100000];
unsigned int cpuints[100000];
double cpdoubles[100000];
char *cpstrings[100000];
struct _namespace cpnamespaces[100000];
struct _namespaceset cpnamespacesets[100000];
struct _multiname multinames[100000];
struct _methodinfo methodinfos[100000];
struct _optionalparam optionalparams[100000];
unsigned long int labels[100000];
static long int numLabels = 0;

static void initReader(byte *buf)
{
	pos = 0;
	abc = buf;
}

static byte readU8(void)
{
	return 255 & abc[pos++];
}

static char readS8(void)
{
	return abc[pos++];
}

static unsigned int readU16(void)
{
	return readU8() | readU8() << 8;
}

static signed int readS24(void)
{
	return readU8() | (readU8() << 8) | (readS8() << 16);
}

static unsigned int readU30(void)
{
	unsigned int result = readU8();
	if (0 == (result & 0x00000080))
		return result;
	result = (result & 0x0000007f) | readU8() << 7;
	if (0 == (result & 0x00004000))
		return result;
	result = (result & 0x00003fff) | readU8() << 14;
	if (0 == (result & 0x00200000))
		return result;
	result = (result & 0x001fffff) | readU8() << 21;
	if (0 == (result & 0x10000000))
		return result;
	return (result & 0x0fffffff) | readU8() << 28;
}

static signed int readS32(void)
{
	signed int result = readS8();
	if (0 == (result & 0x00000080))
		return result;
	result = (result & 0x0000007f) | readS8() << 7;
	if (0 == (result & 0x00004000))
		return result;
	result = (result & 0x00003fff) | readS8() << 14;
	if (0 == (result & 0x00200000))
		return result;
	result = (result & 0x001fffff) | readS8() << 21;
	if (0 == (result & 0x10000000))
		return result;
	return (result & 0x0fffffff) | readS8() << 28;
}

static unsigned int readU32(void)
{
	unsigned int result = readU8();
	if (0 == (result & 0x00000080))
		return result;
	result = (result & 0x0000007f) | readU8() << 7;
	if (0 == (result & 0x00004000))
		return result;
	result = (result & 0x00003fff) | readU8() << 14;
	if (0 == (result & 0x00200000))
		return result;
	result = (result & 0x001fffff) | readU8() << 21;
	if (0 == (result & 0x10000000))
		return result;
	return (result & 0x0fffffff) | readU8() << 28;
}

static double readD64(void)
{
	double result;

	byte *dp = (byte *) (&result);

	if (byteorder == FLASM_BIG_ENDIAN) {
		dp[7] = readU8();
		dp[6] = readU8();
		dp[5] = readU8();
		dp[4] = readU8();
		dp[3] = readU8();
		dp[2] = readU8();
		dp[1] = readU8();
		dp[0] = readU8();
	}
	else {
		memcpy(&result, abc + pos, 8);
		pos += 8;
	}

	return result;
}

static char *readString(void) {
	unsigned int len = readU32();
	memcpy(strbuf + strbufpos, (char *)(abc + pos), len);
	strbuf[strbufpos + len] = '\0';
	char *result = strbuf + strbufpos;
	strbufpos += len + 1;
	pos += len;
	return result;
}


static char *getString(unsigned int index, char *defaultval) {
	return index == 0 ? defaultval : cpstrings[index];
}


static char *getNamespace(unsigned int index) {
	return index == 0 ? "*" : cpstrings[cpnamespaces[index].name];
}


static char *getNamespaceset(unsigned int index) {
	unsigned int k;
	char result[100000] = "\0";
	for (k = 0; k < cpnamespacesets[index].count; ++k) {
		if (k > 0)
			strcat(result, ",");
		strcat(result, getNamespace(cpnamespacesets[index].ns[k]));
	}
	return strdup(result);
}


static char *getMultiname(unsigned int index) {
	if (index == 0)
		return "*";

	struct _multiname *multinameref = &multinames[index];

	char *ns = "";
	char *name = "";
	char fullname[100000] = "\0";

	switch (multinameref->kind) {
		case MN_QNAME:
		case MN_QNAMEA:
			ns = getNamespace(multinameref->ref);
		case MN_RTQNAME:
		case MN_RTQNAMEA:
			name = getString(multinameref->name, "*");
			break;

		case MN_MULTINAMEL:
		case MN_MULTINAMELA:
			ns = getNamespaceset(multinameref->ref);
		case MN_MULTINAME:
		case MN_MULTINAMEA:
			name = getString(multinameref->name, "*");
			break;

		case MN_RTQNAMEL:
		case MN_RTQNAMELA:
			break;
	}

	if (strlen(ns) > 0)	{
		strcat(fullname, ns);
		strcat(fullname, ":");
	}

	strcat(fullname, name);

	return strdup(fullname);
}


static char *getMethodName(unsigned int index) {
	return getString(methodinfos[index].name, "");
}


static int findLabel(unsigned long int addr)
{
	// Terribly inefficient. Use hash table instead.
	unsigned int i;
	for (i = 0; i < numLabels; ++i) {
		if (labels[i] == addr)
			return i;
	}
	return -1;
}


static void addLabel(unsigned long int addr)
{
	if (findLabel(addr) >= 0)
		return;

	labels[numLabels++] = addr;
}


unsigned int readOffset() {
	signed int offset = readS24();
	addLabel(pos + offset);
	return findLabel(pos + offset);
}


static void printConstantPool(void)
{
	unsigned int i, k;

	printf("\n");
	print("constantPool\n");
	++indent;

	cpints[0] = 0;
	unsigned int numints = readU30();
	if (numints > 0) {
		print("int ");
		for (i = 1; i < numints; ++i) {
			printf("%s%i", i > 1 ? ", " : "", cpints[i] = readS32());
		}
		putchar('\n');
	}

	cpuints[0] = 0;
	unsigned int numuints = readU30();
	if (numuints > 0) {
		print("uint ");
		for (i = 1; i < numuints; ++i) {
			printf("%s%u", i > 1 ? ", " : "", cpuints[i] = readS32());
		}
		putchar('\n');
	}

	cpdoubles[0] = 0.0/0.0;
	unsigned int numdoubles = readU30();
	if (numdoubles > 0) {
		print("double ");
		for (i = 1; i < numdoubles; ++i) {
			printf("%s%f", i > 1 ? ", " : "", cpdoubles[i] = readD64());
		}
		putchar('\n');
	}

	cpstrings[0] = "";
	unsigned int numstrings = readU30();
	if (numstrings > 0) {
		print("string ");
		for (i = 1; i < numstrings; ++i) {
			if (i > 1)
				printf("%s", ", ");
			printstr(cpstrings[i] = readString());
		}
		putchar('\n');
	}

	unsigned int numnamespaces = readU30();
	if (numnamespaces > 0) {
		print("namespace ");
		for (i = 1; i < numnamespaces; ++i) {
			cpnamespaces[i].kind = readU8();
			cpnamespaces[i].name = readU30();
			printf("%s'%s'(0x%02x)", i > 1 ? ", " : "", getNamespace(i), cpnamespaces[i].kind);
		}
		putchar('\n');
	}

	unsigned int numnamespacesets = readU30();
	for (i = 1; i < numnamespacesets; ++i) {
		cpnamespacesets[i].count = readU8();

		print("namespaceSet ");
		for (k = 0; k < cpnamespacesets[i].count; ++k) {
			cpnamespacesets[i].ns[k] = readU30();
			printf("%s'%s'", k > 0 ? ", " : "", getNamespace(cpnamespacesets[i].ns[k]));
		}
		putchar('\n');
	}

	unsigned int nummultinames = readU30();
	print("multiname");

	for (i = 1; i < nummultinames; ++i) {
		multinames[i].kind = readU8();

		switch (multinames[i].kind) {
			case MN_QNAME:
			case MN_QNAMEA:
				multinames[i].ref = readU30();
			case MN_RTQNAME:
			case MN_RTQNAMEA:
				multinames[i].name = readU30();
				break;

			case MN_MULTINAME:
			case MN_MULTINAMEA:
				multinames[i].name = readU30();
			case MN_MULTINAMEL:
			case MN_MULTINAMELA:
				multinames[i].ref = readU30();
				break;

			case MN_RTQNAMEL:
			case MN_RTQNAMELA:
				break;

			default:
				print("Unknown multiname type: 0x%02x", multinames[i].kind);
		}

		printf(" '%s'(0x%02x),", getMultiname(i), multinames[i].kind);
	}
	putchar('\n');

	--indent;
	print("end // of constantPool\n");
}


static void printDefaultParam(byte type, unsigned int index)
{
	switch (type) {
		case CONST_UNDEFINED:
			printf("%s", "undefined");
			break;

		case CONST_FALSE:
			printf("%s", "false");
			break;

		case CONST_TRUE:
			printf("%s", "true");
			break;

		case CONST_NULL:
			printf("%s", "null");
			break;

		case CONST_UTF8:
			printstr(cpstrings[index]);
			break;

		case CONST_INT:
			printf("%i", cpints[index]);
			break;

		case CONST_UINT:
			printf("%u", cpuints[index]);
			break;

		case CONST_DOUBLE:
			printf("%f", cpdoubles[index]);
			break;

		case CONST_NAMESPACE:
		case CONST_PACKAGENAMESPACE:
		case CONST_PROTECTEDNAMESPACE:
		case CONST_PACKAGEINTERNALNS:
		case CONST_EXPLICITNAMESPACE:
		case CONST_STATICPROTECTEDNS:
		case CONST_PRIVATENAMESPACE:
			printf("%s", getNamespace(index));
			break;;

		default:
			printf("Unknown param type: 0x%01x", type);
			break;
	}
}


static void printMethods(void)
{
	unsigned int i, k, m;
	unsigned int nummethods = readU30();
	unsigned int paramtypes[10000];
	unsigned int paramnames[10000];
	unsigned int numoptionalparams = 0;

	printf("\n");
	print("%u methods\n", nummethods);
	++indent;

	for (i = 0; i < nummethods; ++i) {
		struct _methodinfo *methodinforef = &methodinfos[i];

		methodinforef->numparams = readU30();
		methodinforef->returntype = readU30();

		for (k = 0; k < methodinforef->numparams; ++k) {
			paramtypes[k] = readU30();
		}

		methodinforef->name = readU30();
		methodinforef->flags = readU8();

		if (methodinforef->flags & METH_HASOPTIONAL) {
			numoptionalparams = readU30();
			for (m = 0; m < numoptionalparams; ++m) {
				optionalparams[m].value = readU30();
				optionalparams[m].type = readU8();
			}
		}

		print("function %s(", getMethodName(i));

		if (methodinforef->flags & METH_HASPARAMNAMES) {
			for (k = 0; k < methodinforef->numparams; ++k) {
				paramnames[k] = readU30();
				printf("%s%s:%s", k > 0 ? ", " : "",
					cpstrings[paramnames[k]], getMultiname(paramtypes[k]));

				if (methodinforef->flags & METH_HASOPTIONAL
					&& methodinforef->numparams - k <= numoptionalparams)
				{
					putchar('=');
					unsigned int p = k - methodinforef->numparams + numoptionalparams;
					printDefaultParam(optionalparams[p].type, optionalparams[p].value);
				}
			}
		}
		else {
			for (k = 0; k < methodinforef->numparams; ++k) {
				printf("%s%s", k > 0 ? ", " : "", getMultiname(paramtypes[k]));
			}
		}

		printf("): %s\n", getMultiname(methodinforef->returntype));
	}

	--indent;
	print("end // of methods\n");
}


static void printMetadata(void)
{
	unsigned int i, k;
	unsigned int nummetadatas = readU30();

	for (i = 0; i < nummetadatas; ++i) {
		unsigned int name = readU30();
		unsigned int numitems = readU30();

		for (k = 0; k < numitems; ++k) {
			unsigned int key = readU30();
			unsigned int value = readU30();
		}
	}
}


static void printTraits(void)
{
	unsigned int i, k;
	unsigned int numtraits = readU30();
	if (numtraits == 0)
		return;

	printf("\n");
	print("%u traits", numtraits);
	++indent;

	for (i = 0; i < numtraits; ++i) {
		unsigned int name = readU30();
		byte kind = readU8();
		unsigned int slotid = readU30();
		byte tag = kind & 0x0f;
		byte flags = kind >> 4;

		printf("\n");

		switch (kind & 0x0f) {
			case TRAIT_SLOT:
			case TRAIT_CONST:
				if (tag == TRAIT_SLOT)
					print("var");
				else
					print("const");
				printf(" %s", getMultiname(name));

				unsigned int typename = readU30();
				unsigned int valueid = readU30();

				printf(" slot %u type %s value %u ", slotid, getMultiname(typename), valueid);
				if (valueid != 0) {
					byte valuekind = readU8();
					printf ("kind %u ", valuekind);
				}
				break;

			case TRAIT_METHOD:
			case TRAIT_GETTER:
			case TRAIT_SETTER:
				print("%s %s", flags & 1 ? "final" : "virtual", flags & 2 ? "override" : "new");
				if (tag == TRAIT_METHOD)
					printf(" %s", "method");
				else if (tag == TRAIT_GETTER)
					printf(" %s", "getter");
				else
					printf(" %s", "setter");
				printf(" %s", getMultiname(name));

				unsigned int methodid = readU30();
				printf(" disp %u %s", slotid, getMethodName(methodid));
				break;

			case TRAIT_CLASS:
				print("class");
				printf(" %s", getMultiname(name));
				unsigned int classid = readU30();
				printf(" slot %u classid %u", slotid, classid);
				break;

			case TRAIT_FUNCTION:
				print("function");
				printf(" %s", getMultiname(name));
				unsigned int fmethodid = readU30();
				printf(" slot %u method %s", slotid, getMethodName(fmethodid));
				break;
		}

		if (flags & 4) {
			unsigned int nummetadatas = readU30();
			for (k = 0; k < nummetadatas; ++k) {
				unsigned int metadata = readU30();
				printf(" metadata %u", metadata);
			}
		}
	}

	--indent;
	putchar('\n');
	print("end // of traits\n");
}


static void printClasses(void)
{
	unsigned int i, k;
	unsigned int numclasses = readU30();

	printf("\n");
	print("%u classes\n", numclasses);
	++indent;

	for (i = 0; i < numclasses; ++i) {
		unsigned int name = readU30();
		unsigned int supername = readU30();

		byte flags = readU8();

		print("");

		if (flags & CLASS_SEALED) {
			printf("sealed ");
		}

		if (flags & CLASS_FINAL) {
			printf("final ");
		}

		if (flags & CLASS_INTERFACE) {
			printf("interface ");
		}

		if (flags & CLASS_PROTECTEDNS) {
			unsigned int protectedns = readU30();
			printf("protected(%s) ", getNamespace(protectedns));
		}

		printf("instance %s extends %s", getMultiname(name), getMultiname(supername));

		unsigned int numinterfaces = readU30();
		if (numinterfaces > 0) {
			printf(" implements ");
			for (k = 0; k < numinterfaces; ++k) {
				unsigned int interfacename = readU30();
				printf("%s, ", getMultiname(interfacename));
			}
		}

		unsigned int iinit = readU30();
		printf(" init %s %u", getMethodName(iinit), iinit);
		printTraits();

		putchar('\n');
	}

	for (i = 0; i < numclasses; ++i) {
		unsigned int cinit = readU30();
		// Why +1???
		print("static init %s %u\n", getMethodName(cinit + 1), cinit);
		printTraits();
	}

	--indent;
	putchar('\n');
	print("end // of classes\n");
}


static void printScripts(void)
{
	unsigned int i;
	unsigned int numscripts = readU30();

	printf("\n");
	print("%u scripts\n", numscripts);
	++indent;

	for (i = 0; i < numscripts; ++i) {
		unsigned int initid = readU30();
		print("init %s", getMethodName(initid));
		printTraits();
	}

	--indent;
	putchar('\n');
	print("end // of scripts\n");
}


static void printCode(unsigned int codelength)
{
	unsigned int endpos = pos + codelength;

	printf("\n");
	print("code\n");
	++indent;

	numLabels = 0;

	while (pos < endpos) {
		byte opcode = readU8();

		if (opcode == ABCACTION_LABEL)
			addLabel(pos - 1);

		int lnum = findLabel(pos - 1);
		if (lnum >= 0) {
			printIndent(-1);
			printf("label%u:\n", lnum);
		}

		switch (opcode) {
			case ABCACTION_ADD:
				println("add");
				break;

			case ABCACTION_ADD_D:
				println("add_d // undocumented");
				break;

			case ABCACTION_ADD_I:
				println("add_i");
				break;

			case ABCACTION_ADD_P:
				println("add_p %u // undocumented", readU30());
				break;

			case ABCACTION_APPLYTYPE:
				println("applyType %u // undocumented", readU30());
				break;

			case ABCACTION_ASTYPE:
				println("asType %s", getMultiname(readU30()));
				break;

			case ABCACTION_ASTYPELATE:
				println("asTypeLate");
				break;

			case ABCACTION_BITAND:
				println("bitAnd");
				break;

			case ABCACTION_BITNOT:
				println("bitNot");
				break;

			case ABCACTION_BITOR:
				println("bitOr");
				break;

			case ABCACTION_BITXOR:
				println("bitXor");
				break;

			case ABCACTION_BKPT:
				println("bkpt // undocumented");
				break;

			case ABCACTION_BKPTLINE:
				println("bkptLine %u // undocumented", readU30());
				break;

			case ABCACTION_CALL:
				// Number of arguments
				println("call (%u)", readU30());
				break;

			case ABCACTION_CALLMETHOD: {
				unsigned int method = readU30();
				unsigned int numargs = readU30();
				println("callMethod %s (%u)", getMethodName(method), numargs);
				break;
			}

			case ABCACTION_CALLPROPERTY: {
				unsigned int prop = readU30();
				unsigned int numargs = readU30();
				println("callProperty %s (%u)", getMultiname(prop), numargs);
				break;
			}

			case ABCACTION_CALLPROPLEX: {
				unsigned int prop = readU30();
				unsigned int numargs = readU30();
				println("callPropLex %s (%u)", getMultiname(prop), numargs);
				break;
			}

			case ABCACTION_CALLPROPVOID: {
				unsigned int prop = readU30();
				unsigned int numargs = readU30();
				println("callPropVoid %s (%u)", getMultiname(prop), numargs);
				break;
			}

			case ABCACTION_CALLSTATIC: {
				unsigned int method = readU30();
				unsigned int numargs = readU30();
				println("callStatic %s (%u)", getMethodName(method), numargs);
				break;
			}

			case ABCACTION_CALLSUPER: {
				unsigned int super = readU30();
				unsigned int numargs = readU30();
				println("callSuper %s (%u)", getMultiname(super), numargs);
				break;
			}

			case ABCACTION_CALLSUPERVOID: {
				unsigned int super = readU30();
				unsigned int numargs = readU30();
				println("callSuperVoid %s (%u)", getMultiname(super), numargs);
				break;
			}

			case ABCACTION_CHECKFILTER:
				println("checkFilter");
				break;

			case ABCACTION_COERCE:
				println("coerce %s", getMultiname(readU30()));
				break;

			case ABCACTION_COERCE_A:
				println("coerce_a");
				break;

			case ABCACTION_COERCE_B:
				println("coerce_b // undocumented");
				break;

			case ABCACTION_COERCE_D:
				println("coerce_d // undocumented");
				break;

			case ABCACTION_COERCE_I:
				println("coerce_i // undocumented");
				break;

			case ABCACTION_COERCE_O:
				println("coerce.o // undocumented");
				break;

			case ABCACTION_COERCE_S:
				println("coerce_s");
				break;

			case ABCACTION_COERCE_U:
				println("coerce_u // undocumented");
				break;

			case ABCACTION_CONSTRUCT:
				println("construct (%u)", readU30());
				break;

			case ABCACTION_CONSTRUCTPROP: {
				unsigned int prop = readU30();
				unsigned int numargs = readU30();
				println("constructProp %s (%u)", getMultiname(prop), numargs);
				break;
			}

			case ABCACTION_CONSTRUCTSUPER:
				println("constructSuper (%u)", readU30());
				break;

			case ABCACTION_CONVERT_B:
				println("convert_b");
				break;

			case ABCACTION_CONVERT_D:
				println("convert_d");
				break;

			case ABCACTION_CONVERT_I:
				println("convert_i");
				break;

			case ABCACTION_CONVERT_M:
				println("convert_m // undocumented");
				break;

			case ABCACTION_CONVERT_M_P:
				println("convert_mp %u // undocumented", readU30());
				break;

			case ABCACTION_CONVERT_O:
				println("convert_o");
				break;

			case ABCACTION_CONVERT_S:
				println("convert_s");
				break;

			case ABCACTION_CONVERT_U:
				println("convert_u");
				break;

			case ABCACTION_DEBUG: {
				char* debugtype = readU8() == 1 ? "local " : "";
				unsigned int regname = readU30();
				byte reg = readU8();
				unsigned int line = readU30();
				println("debug %s%s slot %u line %u", debugtype, cpstrings[regname], reg, line);
				break;
			}

			case ABCACTION_DEBUGFILE:
				println("debugFile %s", cpstrings[readU30()]);
				break;

			case ABCACTION_DEBUGLINE:
				println("debugLine %u", readU30());
				break;

			case ABCACTION_DECLOCAL:
				println("decLocal r%u", readU30());
				break;

			case ABCACTION_DECLOCAL_I:
				println("decLocal_i r%u", readU30());
				break;

			case ABCACTION_DECLOCAL_P: {
				unsigned int param = readU30();
				unsigned int reg = readU30();
				println("decLocal_p %u r%u // undocumented", param, reg);
				break;
			}

			case ABCACTION_DECREMENT:
				println("decrement");
				break;

			case ABCACTION_DECREMENT_I:
				println("decrement_i");
				break;

			case ABCACTION_DECREMENT_P:
				println("decrement_p %u // undocumented", readU30());
				break;

			case ABCACTION_DELETEDESCENDANTS:
				println("deleteDescendants %u // undocumented", readU30());
				break;

			case ABCACTION_DELETEPROPERTY:
				println("deleteProperty %s", getMultiname(readU30()));
				break;

			case ABCACTION_DIVIDE:
				println("divide");
				break;

			case ABCACTION_DIVIDE_P:
				println("divide_p %u // undocumented", readU30());
				break;

			case ABCACTION_DUP:
				println("dup");
				break;

			case ABCACTION_DXNS:
				println("dxns %s", cpstrings[readU30()]);
				break;

			case ABCACTION_DXNSLATE:
				println("dxnsLate");
				break;

			case ABCACTION_EQUALS:
				println("equals");
				break;

			case ABCACTION_ESC_XATTR:
				println("esc_xattr");
				break;

			case ABCACTION_ESC_XELEM:
				println("esc_xelem");
				break;

			case ABCACTION_FINDDEF:
				println("findDef %s // undefined", getMultiname(readU30()));
				break;

			case ABCACTION_FINDPROPERTY:
				println("findProperty %s", getMultiname(readU30()));
				break;

			case ABCACTION_FINDPROPSTRICT:
				println("findPropStrict %s", getMultiname(readU30()));
				break;

			case ABCACTION_GETDESCENDANTS:
				println("getDescendants %s", getMultiname(readU30()));
				break;

			case ABCACTION_GETGLOBALSCOPE:
				println("getGlobalScope");
				break;

			case ABCACTION_GETGLOBALSLOT:
				println("getLocalSlot %u", readU30());
				break;

			case ABCACTION_GETLEX:
				println("getLex %s", getMultiname(readU30()));
				break;

			case ABCACTION_GETLOCAL:
				println("getlocal r%u", readU30());
				break;

			case ABCACTION_GETLOCAL0:
				println("getLocal0");
				break;

			case ABCACTION_GETLOCAL1:
				println("getLocal1");
				break;

			case ABCACTION_GETLOCAL2:
				println("getLocal2");
				break;

			case ABCACTION_GETLOCAL3:
				println("getlocal3");
				break;

			case ABCACTION_GETPROPERTY:
				println("getProperty %s", getMultiname(readU30()));
				break;

			case ABCACTION_GETSCOPEOBJECT:
				println("getScopeObject %u", readU8());
				break;

			case ABCACTION_GETSLOT:
				println("getSlot %u", readU30());
				break;

			case ABCACTION_GETSUPER:
				println("getSuper %s", getMultiname(readU30()));
				break;

			case ABCACTION_GREATEREQUALS:
				println("greaterEquals");
				break;

			case ABCACTION_GREATERTHAN:
				println("greaterThan");
				break;

			case ABCACTION_HASNEXT:
				println("hasNext");
				break;

			case ABCACTION_HASNEXT2: {
				unsigned int objectreg = readU30();
				unsigned int indexreg = readU30();
				println("hasNext2 %u r%u", objectreg, indexreg);
				break;
			}

			case ABCACTION_IFEQ: {
				println("ifeq label%u", readOffset());
				break;
			}

			case ABCACTION_IFFALSE:
				println("iffalse label%u", readOffset());
				break;

			case ABCACTION_IFGE:
				println("ifge label%u", readOffset());
				break;

			case ABCACTION_IFGT:
				println("ifgt label%u", readOffset());
				break;

			case ABCACTION_IFLE:
				println("ifle label%u", readOffset());
				break;

			case ABCACTION_IFLT:
				println("iflt label%u", readOffset());
				break;

			case ABCACTION_IFNE:
				println("ifne label%u", readOffset());
				break;

			case ABCACTION_IFNGE:
				println("ifnge label%u", readOffset());
				break;

			case ABCACTION_IFNGT:
				println("ifngt label%u", readOffset());
				break;

			case ABCACTION_IFNLE:
				println("ifnle label%u", readOffset());
				break;

			case ABCACTION_IFNLT:
				println("ifnlt label%u", readOffset());
				break;

			case ABCACTION_IFSTRICTEQ:
				println("ifstricteq label%u", readOffset());
				break;

			case ABCACTION_IFSTRICTNE:
				println("ifstrictne label%u", readOffset());
				break;

			case ABCACTION_IFTRUE:
				println("iftrue label%u", readOffset());
				break;

			case ABCACTION_IN:
				println("in");
				break;

			case ABCACTION_INCLOCAL:
				println("incLocal r%u", readU30());
				break;

			case ABCACTION_INCLOCAL_I:
				println("incLocal_i r%u", readU30());
				break;

			case ABCACTION_INCLOCAL_P: {
				unsigned int param = readU30();
				unsigned int reg = readU30();
				println("incLocal_p %u r%u // undocumented", param, reg);
				break;
			}

			case ABCACTION_INCREMENT:
				println("increment");
				break;

			case ABCACTION_INCREMENT_I:
				println("increment_i");
				break;

			case ABCACTION_INCREMENT_P:
				println("decrement_p %u // undocumented", readU30());
				break;

			case ABCACTION_INITPROPERTY:
				println("initProperty %s", getMultiname(readU30()));
				break;

			case ABCACTION_INSTANCEOF:
				println("instanceOf");
				break;

			case ABCACTION_ISTYPE:
				println("isType %s", getMultiname(readU30()));
				break;

			case ABCACTION_ISTYPELATE:
				println("isTypeLate");
				break;

			case ABCACTION_JUMP:
				println("jump label%i", readOffset());
				break;

			case ABCACTION_KILL:
				println("kill r%u", readU30());
				break;

			case ABCACTION_LABEL:
				println("label");
				break;

			case ABCACTION_LESSEQUALS:
				println("lessEquals");
				break;

			case ABCACTION_LESSTHAN:
				println("lessThan");
				break;

			case ABCACTION_LOOKUPSWITCH: {
				unsigned long int startpos = pos - 1;
				signed int offset = readS24();
				addLabel(startpos + offset);

				print("lookupSwitch label%i", findLabel(startpos + offset));
				unsigned int k;
				unsigned int numcases = readU30();
				for (k = 0; k <= numcases; ++k) {
					offset = readS24();
					addLabel(startpos + offset);
					printf(", label%i", findLabel(startpos + offset));
				}
				putchar('\n');
				break;
			}

			case ABCACTION_LSHIFT:
				println("lshift");
				break;

			case ABCACTION_MODULO:
				println("modulo");
				break;

			case ABCACTION_MODULO_P:
				println("modulo_p %u // undocumented", readU30());
				break;

			case ABCACTION_MULTIPLY:
				println("multiply");
				break;

			case ABCACTION_MULTIPLY_I:
				println("multiply_i");
				break;

			case ABCACTION_MULTIPLY_P:
				println("multiply_p %u // undocumented", readU30());
				break;

			case ABCACTION_NEGATE:
				println("negate");
				break;

			case ABCACTION_NEGATE_I:
				println("negate_i");
				break;

			case ABCACTION_NEGATE_P:
				println("negate_p %u // undocumented", readU30());
				break;

			case ABCACTION_NEWACTIVATION:
				println("newActivation");
				break;

			case ABCACTION_NEWARRAY:
				println("newArray (%u)", readU30());
				break;

			case ABCACTION_NEWCATCH:
				println("newCatch %u", readU30());
				break;

			case ABCACTION_NEWCLASS:
				println("newClass %u", readU30());
				break;

			case ABCACTION_NEWFUNCTION:
				println("newFunction %s", getMethodName(readU30()));
				break;

			case ABCACTION_NEWOBJECT:
				println("newObject (%u)", readU30());
				break;

			case ABCACTION_NEXTNAME:
				println("nextName");
				break;

			case ABCACTION_NEXTVALUE:
				println("nextValue");
				break;

			case ABCACTION_NOP:
				println("nop");
				break;

			case ABCACTION_NOT:
				println("not");
				break;

			case ABCACTION_POP:
				println("pop");
				break;

			case ABCACTION_POPSCOPE:
				println("popScope");
				break;

			case ABCACTION_PUSHBYTE:
				println("pushByte %u", readU8());
				break;

			case ABCACTION_PUSHCONSTANT:
				println("pushConstant %s", getMultiname(readU30()));
				break;

			case ABCACTION_PUSHDECIMAL:
				println("pushDecimal %u // undocumented", readU30());
				break;

			case ABCACTION_PUSHDNAN:
				println("pushDNaN // undocumented");
				break;

			case ABCACTION_PUSHDOUBLE:
				println("pushDouble %f", cpdoubles[readU30()]);
				break;

			case ABCACTION_PUSHFALSE:
				println("pushFalse");
				break;

			case ABCACTION_PUSHINT:
				println("pushInt %i", cpints[readU30()]);
				break;

			case ABCACTION_PUSHNAMESPACE:
				println("pushNamespace %s", getNamespace(readU30()));
				break;

			case ABCACTION_PUSHNAN:
				println("pushNaN");
				break;

			case ABCACTION_PUSHNULL:
				println("pushNull");
				break;

			case ABCACTION_PUSHSCOPE:
				println("pushScope");
				break;

			case ABCACTION_PUSHSHORT:
				println("pushShort %u", readU30());
				break;

			case ABCACTION_PUSHSTRING:
				print("pushString ");
				printstr(cpstrings[readU30()]);
				putchar('\n');
				break;

			case ABCACTION_PUSHTRUE:
				println("pushTrue");
				break;

			case ABCACTION_PUSHUINT:
				println("pushUint %u", cpuints[readU30()]);
				break;

			case ABCACTION_PUSHUNDEFINED:
				println("pushUndefined");
				break;

			case ABCACTION_PUSHWITH:
				println("pushWith");
				break;

			case ABCACTION_RETURNVALUE:
				println("returnValue");
				break;

			case ABCACTION_RETURNVOID:
				println("returnVoid");
				break;

			case ABCACTION_RSHIFT:
				println("rshift");
				break;

			case ABCACTION_SETGLOBALSLOT:
				println("setLocalSlot %u", readU30());
				break;

			case ABCACTION_SETLOCAL:
				println("setlocal r%u", readU30());
				break;

			case ABCACTION_SETLOCAL0:
				println("setLocal0");
				break;

			case ABCACTION_SETLOCAL1:
				println("setLocal1");
				break;

			case ABCACTION_SETLOCAL2:
				println("setLocal2");
				break;

			case ABCACTION_SETLOCAL3:
				println("setlocal3");
				break;

			case ABCACTION_SETPROPERTY:
				println("setProperty %s", getMultiname(readU30()));
				break;

			case ABCACTION_SETSLOT:
				println("setSlot %u", readU30());
				break;

			case ABCACTION_SETSUPER:
				println("setSuper %s", getMultiname(readU30()));
				break;

			case ABCACTION_STRICTEQUALS:
				println("strictEquals");
				break;

			case ABCACTION_SUBTRACT:
				println("subtract");
				break;

			case ABCACTION_SUBTRACT_I:
				println("subtract_i");
				break;

			case ABCACTION_SUBTRACT_P:
				println("subtract_p %u // undocumented", readU30());
				break;

			case ABCACTION_SWAP:
				println("swap");
				break;

			case ABCACTION_THROW:
				println("throw");
				break;

			case ABCACTION_TIMESTAMP:
				println("timeStamp // undocumented");
				break;
			case ABCACTION_TYPEOF:
				println("typeOf");
				break;

			case ABCACTION_URSHIFT:
				println("urshift");
				break;

			case ABCACTION_ABS_JUMP:
			case ABCACTION_ALLOC:
			case ABCACTION_CALLINTERFACE:
			case ABCACTION_CALLSUPERID:
			case ABCACTION_CODEGENOP:
			case ABCACTION_CONCAT:
			case ABCACTION_DECODE:
			case ABCACTION_DOUBLETOATOM:
			case ABCACTION_MARK:
			case ABCACTION_PROLOGUE:
			case ABCACTION_SENDENTER:
			case ABCACTION_SWEEP:
			case ABCACTION_VERIFYOP:
			case ABCACTION_VERIFYPASS:
			case ABCACTION_WB:
			default:
				println("Unknown action code: 0x%02x", opcode);
				break;
		}
	}

	--indent;
	print("end // of code\n");
}

static void printMethodBodies(void)
{
	unsigned int i, k;
	unsigned int nummethods = readU30();

	printf("\n");
	print("%u methods", nummethods);
	++indent;

	for (i = 0; i < nummethods; ++i) {
		unsigned int methodinfo = readU30();
		unsigned int maxstack = readU30();
		unsigned int numregisters = readU30();
		unsigned int scopedepth = readU30();
		unsigned int maxscope = readU30();
		unsigned int codelength = readU30();

		putchar('\n');
		print("%s stack %u numregisters %u scope %u maxscope %u length %u\n", getMethodName(methodinfo),
			maxstack, numregisters, scopedepth, maxscope, codelength);

		printCode(codelength);

		unsigned int numexceptions = readU30();
		if (numexceptions > 0) {
			print("%u exceptions\n");
			++indent;

			for (k = 0; k < numexceptions; ++k) {
				unsigned int from = readU30();
				unsigned int to = readU30();
				unsigned int target = readU30();
				unsigned int exctype = readU30();
				unsigned int varname = readU30();
				print("from %u to %u target %u type %s name %s\n",
					from, to, target, getString(exctype, "*"), cpstrings[varname]);
			}

			--indent;
			putchar('\n');
			print("end // of exceptions\n");
		}

		printTraits();
	}

	--indent;
	putchar('\n');
	print("end // of methods\n");
}


void printAbcActions(unsigned long int length)
{
	++indent;

	initReader(buffer);

	unsigned int minorversion = readU16();
	unsigned int majorversion = readU16();
	print("version %u.%u\n", majorversion, minorversion);

	printConstantPool();
	printMethods();
	printMetadata();
	printClasses();
	printScripts();
	printMethodBodies();

	--indent;
}
