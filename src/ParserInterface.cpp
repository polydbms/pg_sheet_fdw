//
// Created by joel on 14.02.24.
//

#include "ParserInterface.h"

/*
 * if numberOfThreads is -1, automatically take a high number of threads, that makes sense for system.
 */
unsigned long registerExcelFileAndSheetAsTable(const char *pathToFile, const char *sheetName, unsigned int tableOID, int numberOfThreads){
    try {
        // TODO first check, if already registered on the id. Also check for same names if already registered.

        debug_print("[%s] Start registering.\n", __func__);
        // then register with standard settings
        SheetReaderSettings settings;
        settings.filePath = pathToFile;
        settings.sheetName = sheetName;

        debug_print("[%s] Setting thread number:", __func__);
        // set number of threads for Sheet Reader
        settings.num_threads = numberOfThreads;
        if (settings.num_threads < 1) {
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
        debug_print("%d\n", settings.num_threads);


        debug_print("[%s] Building file object.\n", __func__);
        std::shared_ptr<XlsxFile> file = std::make_shared<XlsxFile>(settings.filePath);
        file->mParallelStrings = settings.parallel;
        file->parseSharedStrings();


        debug_print("[%s] Building sheet object.\n", __func__);
        std::shared_ptr<XlsxSheet> fsheet = (settings.sheetName == "") ? std::make_shared<XlsxSheet>(file->getSheet(1))
                                                                       : std::make_shared<XlsxSheet>(
                        file->getSheet(settings.sheetName));
        fsheet->mHeaders = settings.headers;
        // if parallel we need threads for string parsing
        // for interleaved, both sheet & strings need additional thread for decompression (meaning min is 2)
        int act_num_threads = settings.num_threads - settings.parallel * 2 - (settings.num_threads > 1);
        if (act_num_threads <= 0) act_num_threads = 1;
        debug_print("[%s] Setting interleaving threads.\n", __func__);
        bool success = fsheet->interleaved(settings.skip_rows, settings.skip_columns, act_num_threads);
        debug_print("[%s] Finalizing sheet object.\n", __func__);
        file->finalize();


        debug_print("[%s] Finished.\n", __func__);
        if (!success) return 0;
        else {
            settings.file = file;
            settings.sheet = fsheet;
            ParserInterfaceSettingsMap[tableOID] = settings;

            return ParserInterfaceSettingsMap[tableOID].sheet->mDimension.second;
        }
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        return 0;
    }
}

unsigned long startNextRow(unsigned int tableOID){
    try {
        ParserInterfaceSettingsMap[tableOID].currentRow = std::make_shared<std::pair<size_t, std::vector<XlsxCell>>>(
                std::pair<size_t, std::vector<XlsxCell>>(ParserInterfaceSettingsMap[tableOID].sheet->nextRow()));
        ParserInterfaceSettingsMap[tableOID].cellIt = 0;
        return ParserInterfaceSettingsMap[tableOID].currentRow->second.size();
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
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
        case CellType::T_SKIP:
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
    return strdup(ParserInterfaceSettingsMap[tableOID].file->getDynamicString(0,stringIndex).c_str());
}

// memory of string has to be freed afterwards!
char* readStaticString(unsigned int tableOID, unsigned long long stringIndex){
    return strdup(ParserInterfaceSettingsMap[tableOID].file->getString(stringIndex).c_str());
}