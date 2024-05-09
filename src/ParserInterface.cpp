//
// Created by joel on 14.02.24.
//

#include "ParserInterface.h"

unsigned long registerExcelFileAndSheetAsTable(const char *pathToFile, const char *sheetName, unsigned int tableOID){
    try {
        // first check, if already registered on the id. Also check for same names if already registered.


        // then register with standard settings
        SheetReaderSettings settings;
        settings.filePath = pathToFile;
        settings.sheetName = sheetName;

        if (settings.num_threads == -1) {
            // automatically decide number of threads
            settings.num_threads = std::thread::hardware_concurrency();
            if (settings.num_threads <= 0) {
                settings.num_threads = 1;
            }
            // limit impact on user machine
            if (settings.num_threads > 6 && settings.num_threads <= 10) settings.num_threads = 6;
            // really diminishing returns with higher number of threads
            if (settings.num_threads > 10) settings.num_threads = 10;
        }
        if (settings.num_threads <= 1) {
            settings.num_threads = 1;
            settings.parallel = false;
        }

        std::shared_ptr<XlsxFile> file = std::make_shared<XlsxFile>(settings.filePath);
        file->mParallelStrings = settings.parallel;
        file->parseSharedStrings();

        std::shared_ptr<XlsxSheet> fsheet = (settings.sheetName == "") ? std::make_shared<XlsxSheet>(file->getSheet(1))
                                                                       : std::make_shared<XlsxSheet>(
                        file->getSheet(settings.sheetName));
        fsheet->mHeaders = settings.headers;
        // if parallel we need threads for string parsing
        // for interleaved, both sheet & strings need additional thread for decompression (meaning min is 2)
        int act_num_threads = settings.num_threads - settings.parallel * 2 - (settings.num_threads > 1);
        if (act_num_threads <= 0) act_num_threads = 1;
        bool success = fsheet->interleaved(settings.skip_rows, settings.skip_columns, act_num_threads);
        file->finalize();

        if (!success) return 0;
        else {
            settings.file = file;
            settings.sheet = fsheet;
            ParserInterfaceSettingsMap[tableOID] = settings;

            return ParserInterfaceSettingsMap[tableOID].sheet->mDimension.second;
        }
    } catch (...) {
        return 0;
    }
}

unsigned long startNextRow(unsigned int tableOID){
    try {
        ParserInterfaceSettingsMap[tableOID].currentRow = std::make_shared<std::pair<size_t, std::vector<XlsxCell>>>(
                std::pair<size_t, std::vector<XlsxCell>>(ParserInterfaceSettingsMap[tableOID].sheet->nextRow()));
        ParserInterfaceSettingsMap[tableOID].cellIt = 0;
        return ParserInterfaceSettingsMap[tableOID].currentRow->second.size();
    } catch (...) {
        return 0;
    }
}

// copies all values of the next cell into a new PGExcelCell
PGExcelCell getNextCell(unsigned int tableOID){
    try {
        const XlsxCell &cell = ParserInterfaceSettingsMap[tableOID].currentRow->second[ParserInterfaceSettingsMap[tableOID].cellIt];
        PGExcelCell nextCell = ParserConvertToPGCell(cell, tableOID);
        ParserInterfaceSettingsMap[tableOID].cellIt++;
        return nextCell;
    } catch (...) {
        PGExcelCell error{};
        error.type = PGExcelCellType::T_ERROR;
        return error;
    }
}

// returns a pointer on the next cell
PGExcelCell *getNextCellCast(unsigned int tableOID){
    try {
        PGExcelCell *nextCell = (PGExcelCell *)&(ParserInterfaceSettingsMap[tableOID].currentRow->second[ParserInterfaceSettingsMap[tableOID].cellIt]);
        ParserInterfaceSettingsMap[tableOID].cellIt++;
        return nextCell;
    } catch (...) {
        return nullptr;
    }
}

PGExcelCell ParserConvertToPGCell(const XlsxCell& cell, unsigned int tableOID){
    PGExcelCell cCell{};
    switch(cell.type) {
        case CellType::T_DATE:
        case CellType::T_NUMERIC:
            cCell.data.real = cell.data.real;
            break;
        case CellType::T_STRING_REF:
        case CellType::T_STRING:
        case CellType::T_STRING_INLINE:
            cCell.data.stringIndex = cell.data.integer;
            break;
        case CellType::T_BOOLEAN:
            cCell.data.boolean = cell.data.boolean;
            break;
        case CellType::T_NONE:
        case CellType::T_ERROR:
            break;
    }
    cCell.type = static_cast<PGExcelCellType>(cell.type);
    return cCell;
}


void dropTable(unsigned int tableOID){
    ParserInterfaceSettingsMap.erase(tableOID);
}

// memory of string has to be freed afterwards!
char* readDynamicString(unsigned int tableOID, unsigned long long stringIndex){
    return strdup(ParserInterfaceSettingsMap[tableOID].file->getDynamicString(stringIndex).c_str());
}

// memory of string has to be freed afterwards!
char* readStaticString(unsigned int tableOID, unsigned long long stringIndex){
    return strdup(ParserInterfaceSettingsMap[tableOID].file->getString(stringIndex).c_str());
}