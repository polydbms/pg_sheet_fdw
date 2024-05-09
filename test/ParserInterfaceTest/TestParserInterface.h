//
// Created by joel on 13.03.24.
//

#ifndef PG_SHEET_FDW_TESTPARSERINTERFACE_H
#define PG_SHEET_FDW_TESTPARSERINTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Custom c++ functions and structs
enum PGExcelCellType {
    T_NONE = 0, // blank cell
    T_NUMERIC = 1, // integer or double
    T_STRING_REF = 2, // we treat all string types like basic null terminated c strings
    T_STRING = 3,
    T_STRING_INLINE = 4,
    T_BOOLEAN = 5, // boolean is just int
    T_ERROR = 6,
    T_DATE = 7 // datetime value, already as unix timestamp (seconds since 1970), Excel stores as number of days since 1900
};

struct PGExcelCell {
    union {
        double real;
        unsigned long long stringIndex;
        unsigned char boolean;  // Using int for boolean in C
    } data;
    unsigned char type;
};

extern int getTestInt();
extern unsigned long registerExcelFileAndSheetAsTable(const char *pathToFile, const char *sheetName, unsigned int tableOID);
extern unsigned long startNextRow(unsigned int tableOID);
extern struct PGExcelCell getNextCell(unsigned int tableOID);
struct PGExcelCell *getNextCellCast(unsigned int tableOID);
char* readStaticString(unsigned int tableOID, unsigned long long stringIndex);
char* readDynamicString(unsigned int tableOID, unsigned long long stringIndex);
extern void dropTable(unsigned int tableOID);


// test functions







#endif //PG_SHEET_FDW_TESTPARSERINTERFACE_H
