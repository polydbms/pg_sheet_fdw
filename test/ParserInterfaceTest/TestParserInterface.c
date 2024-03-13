//
// Created by joel on 13.03.24.
//

#include "TestParserInterface.h"


int main(){
    char * filepath = "../joel_test.xlsx";
    char * sheetname = "encoding";
    unsigned int id = 0;
    long unsigned int success = registerExcelFileAndSheetAsTable(filepath, sheetname, id);
    if(!success) printf("Register did not succeed!\n");
    else printf("Register succeeded!\n");

    unsigned long columnCount = startNextRow(id);
    printf("NextRow column count: %lu\n", columnCount);
    while(columnCount != 0) {
        for (unsigned long i = 0; i < columnCount; i++) {
            struct PGExcelCell cell = getNextCell(id);
            printf("got next cell!\n");
            switch (cell.type) {
                case T_STRING:
                case T_STRING_INLINE:
                case T_STRING_REF:
                    printf("Cell %lu with content: %s\n", i, cell.data.string);
                    free(cell.data.string);
                    break;
                case T_BOOLEAN:
                    printf("Cell %lu with boolean: %d\n", i, cell.data.boolean);
                    break;
                case T_NUMERIC:
                    printf("Cell %lu with number: %f\n", i, cell.data.real);
                    break;
                case T_DATE:
                    printf("Cell %lu with date: %f\n", i, cell.data.real);
                    break;
                default:
                    break;
            }
        }
        columnCount = startNextRow(id);
        printf("NextRow column count: %lu\n", columnCount);
    }
}
