//
// Created by joel on 14.02.24.
//

#include "ParserInterface.h"

int getTestInt(){
    return 5;
}

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

// IMPORTANT. AFTER USAGE, FREE THE STRING COMPONENT! ( NOT PALLOCED BUT MALLOCED)
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

PGExcelCell ParserConvertToPGCell(const XlsxCell& cell, unsigned int tableOID){
    PGExcelCell cCell{};
    switch(cell.type) {
        case CellType::T_NUMERIC:
        case CellType::T_DATE:
            cCell.data.real = cell.data.real;
            break;
        case CellType::T_STRING_REF:
            cCell.data.string = strdup(ParserInterfaceSettingsMap[tableOID].file->getString(cell.data.integer).c_str());
            break;
        case CellType::T_STRING:
        case CellType::T_STRING_INLINE:
            cCell.data.string = strdup(ParserInterfaceSettingsMap[tableOID].file->getDynamicString(cell.data.integer).c_str());
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
