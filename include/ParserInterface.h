//
// Created by joel on 14.02.24.
//

#ifndef PG_SHEET_FDW_PARSERINTERFACE_H
#define PG_SHEET_FDW_PARSERINTERFACE_H

// debug flag
#define PARSERINTERFACEDEBUG

#ifdef PARSERINTERFACEDEBUG
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...) ((void) 0)
#endif

#include "XlsxFile.h"
#include <map>
#include <string>
#include <memory>

// C interface for the fdw
#ifdef __cplusplus
extern "C"
{
#endif


    enum PGExcelCellType {
        T_NONE = 0, // blank cell
        T_NUMERIC = 1, // integer or double
        T_STRING_REF = 2, // we treat all string types like basic null terminated c strings
        T_STRING = 3,
        T_STRING_INLINE = 4,
        T_BOOLEAN = 5, // boolean is just int
        T_ERROR = 6,
        T_DATE = 7, // datetime value, already as unix timestamp (seconds since 1970), Excel stores as number of days since 1900

        T_SKIP = 8
    };

    struct PGExcelCell {
        union {
            double real;
            unsigned long long stringIndex;
            unsigned char boolean;  // Using char (1 byte) for boolean in C
        } data;
        unsigned char type;
    };

    unsigned long registerExcelFileAndSheetAsTable(const char *pathToFile, const char *sheetName, unsigned int tableOID, int numberOfThreads);
    unsigned long startNextRow(unsigned int tableOID);
    struct PGExcelCell getNextCell(unsigned int tableOID);
    struct PGExcelCell *getNextCellCast(unsigned int tableOID);
    char* readStaticString(unsigned int tableOID, unsigned long long stringIndex);
    char* readDynamicString(unsigned int tableOID, unsigned long long stringIndex);
    void dropTable(unsigned int tableOID);

#ifdef __cplusplus
}
#endif

// Data structures used
struct SheetReaderSettings {
    int skip_rows = 0;
    int skip_columns = 0;
    bool headers = false;
    int num_threads = -1;
    bool parallel = true;
    std::string filePath;
    std::string sheetName;
    std::shared_ptr<XlsxFile> file;
    std::shared_ptr<XlsxSheet> sheet;
    std::shared_ptr<std::pair<size_t, std::vector<XlsxCell>>> currentRow;
    unsigned int cellIt = 0;
};

struct RowDataWithDatatype{
    unsigned int datatype;

};

// functions only used internally
PGExcelCell ParserConvertToPGCell(const XlsxCell& cell, unsigned int tableOID);

// variables
std::map<unsigned int, SheetReaderSettings> ParserInterfaceSettingsMap;





#endif //PG_SHEET_FDW_PARSERINTERFACE_H
