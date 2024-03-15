//
// Created by joel on 13.03.24.
//

#include "TestParserInterface.h"


int main(int argc, char** argv){
    if(argc < 2 || argc > 3) {
        printf("Wrong number of arguments!\n");
        printf("Usage:\n");
        printf("    ./output <filepath> [sheetname]\n");
        return 1;
    }
    for(int i = 0; i < argc; i++){
        printf("argument %d: %s\n", i, argv[i]);
    }
    char * filepath = (char *) calloc(strlen(argv[1]), sizeof(char));
    strcpy(filepath, argv[1]);

    char * sheetname;
    if(argc == 3){
        sheetname = (char *) calloc(strlen(argv[1]), sizeof(char));
        strcpy(sheetname, argv[1]);
    } else{
        sheetname = "";
    }
    unsigned int id = 0;
    long unsigned int success = registerExcelFileAndSheetAsTable(filepath, sheetname, id);
    if(!success) printf("Register did not succeed!\n");
    else printf("Register succeeded!\n");

    unsigned long columnCount = startNextRow(id);
    printf("NextRow column count: %lu\n", columnCount);
    while(columnCount != 0) {
        for (unsigned long i = 0; i < columnCount; i++) {
            struct PGExcelCell cell = getNextCell(id);
            switch (cell.type) {
                case T_STRING:
                case T_STRING_INLINE:
                case T_STRING_REF:
                    printf("Cell %lu with content: %s\n", i, cell.data.string);
                    double t;
                    sscanf(cell.data.string, "%lf", &t);
                    printf("Conversion to double: %lf\n", t);
                    free(cell.data.string);
                    break;
                case T_BOOLEAN:
                    printf("Cell %lu with boolean: ", i);
                    if(cell.data.boolean) printf("TRUE\n");
                    else printf("FALSE\n");
                    break;
                case T_NUMERIC:
                    printf("Cell %lu with number: %f\n", i, cell.data.real);
                    long f = cell.data.real;
                    printf("cast to int: %ld\n", f);
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
    return 0;
}
