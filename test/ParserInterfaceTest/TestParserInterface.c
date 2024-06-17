//
// Created by joel on 13.03.24.
//

#include "TestParserInterface.h"


char ** filepaths;

void freePaths(int i){
    i--;
    for(;i > 0; i--){
        free(filepaths[i]);
    }
    free(filepaths);
}

void printRow(unsigned long columnCount, unsigned int tableID){
    for (unsigned long i = 0; i < columnCount; i++) {
        struct PGExcelCell cell = getNextCell(tableID);
        char *c;
        switch (cell.type) {
            case T_STRING:
            case T_STRING_INLINE:
                c = readDynamicString(tableID, cell.data.stringIndex);
                printf("Cell %lu with content: %s\n", i, c);
                free(c);
                break;
            case T_STRING_REF:
                c = readStaticString(tableID, cell.data.stringIndex);
                printf("Cell %lu with content: %s\n", i, c);
                free(c);
                break;
            case T_BOOLEAN:
                printf("Cell %lu with boolean: ", i);
                if(cell.data.boolean) printf("TRUE\n");
                else printf("FALSE\n");
                break;
            case T_NUMERIC:
                printf("Cell %lu with number: %f\n", i, cell.data.real);
                break;
            case T_DATE:
                printf("Cell %lu with date: %f\n", i, cell.data.real);
                break;
            default:
                printf("Cell %lu with no content? Code: %d\n", i, cell.type);
                break;
        }
    }
}

void testOneSheet(){
    unsigned int id = 0;
    long unsigned int success = registerExcelFileAndSheetAsTable(filepaths[0], "", id);
    if(!success){
        printf("Register did not succeed!\n");
        freePaths(1);
        exit(1);
    }
    else printf("Register succeeded!\n");

    unsigned long columnCount = startNextRow(id);
    printf("NextRow column count: %lu\n", columnCount);
    while(columnCount != 0) {
        printRow(columnCount, id);
        columnCount = startNextRow(id);
        printf("NextRow column count: %lu\n", columnCount);
    }
    dropTable(id);
}

void testGetDynamicStringInOneCall(){
    unsigned int id = 0;
    long unsigned int success = registerExcelFileAndSheetAsTable(filepaths[0], "", id);
    if(!success){
        printf("Register did not succeed!\n");
        freePaths(1);
        exit(1);
    }
    else printf("Register succeeded!\n");

    unsigned long columnCount = startNextRow(id);
    printf("NextRow column count: %lu\n", columnCount);
    while(columnCount != 0) {
        for (unsigned long i = 0; i < columnCount; i++) {
            struct PGExcelCell *cell = getNextCellCast(id);
            char *c;
            switch (cell->type) {
                case T_STRING:
                case T_STRING_INLINE:
                    c = readDynamicString(id, cell->data.stringIndex);
                    printf("Cell %lu with content: %s\n", i, c);
                    free(c);
                    break;
                case T_STRING_REF:
                    c = readStaticString(id, cell->data.stringIndex);
                    printf("Cell %lu with content: %s\n", i, c);
                    free(c);
                    break;
                case T_BOOLEAN:
                    printf("Cell %lu with boolean: ", i);
                    if(cell->data.boolean) printf("TRUE\n");
                    else printf("FALSE\n");
                    break;
                case T_NUMERIC:
                    printf("Cell %lu with number: %f\n", i, cell->data.real);
                    break;
                case T_DATE:
                    printf("Cell %lu with date: %f\n", i, cell->data.real);
                    break;
                default:
                    printf("Cell %lu with no content? Code: %d\n", i, cell->type);
                    break;
            }
        }
        columnCount = startNextRow(id);
        printf("NextRow column count: %lu\n", columnCount);
    }
    dropTable(id);
}

void testTwoSheetSimultan(){
    unsigned int id1 = 0;
    unsigned int id2 = 155;
    long unsigned int success = registerExcelFileAndSheetAsTable(filepaths[0], "", id1);
    if(!success){
        printf("Register did not succeed!\n");
        freePaths(2);
        exit(1);
    }
    else printf("Register 1 succeeded!\n");

    success = registerExcelFileAndSheetAsTable(filepaths[1], "", id2);
    if(!success){
        printf("Register did not succeed!\n");
        freePaths(2);
        exit(1);
    }
    else printf("Register 2 succeeded!\n");

    // print rows from the tables interleaved
    unsigned long cc1 = startNextRow(id1);
    unsigned long cc2 = startNextRow(id2);

    while(cc1 != 0 && cc2 != 0){
        printf("==== NextRow of table %u column count: %lu\n", id1,cc1);
        printRow(cc1, id1);
        cc1 = startNextRow(id1);

        printf("==== NextRow of table %u column count: %lu\n", id2,cc2);
        printRow(cc2, id2);
        cc2 = startNextRow(id2);
    }

    dropTable(id1);
    dropTable(id2);
}

void printUsage(){
    printf("Usage:\n");
    printf("    ./output <testnumber> <filepath> {<filepath>}\n");
    printf("    ./output -h   //prints this help message\n");
    printf("Test numbers: \n");
    printf("    1 for reading a given sheet with copying of datastructures between c and c++\n");
    printf("    2 for reading 2 sheets interleaved\n");
    printf("    3 for reading a sheet with less copying and pointer magic\n");
    exit(0);
}

int handleOptions(int argc, char** argv){
    if((argc == 2 && strcmp(argv[1], "-h") == 0) || argc < 2){
        printUsage();
    }
    for(int i = 0; i < argc; i++){
        printf("argument %d: %s\n", i, argv[i]);
    }
    if(strcmp(argv[1], "1") == 0){
        if(argc != 3) {
            printf("Wrong number of arguments!\n");
            printUsage();
        }
        filepaths = calloc(1, sizeof (char*));
        filepaths[0] = (char *) calloc(strlen(argv[2]), sizeof(char));
        strcpy(filepaths[0], argv[2]);
        testOneSheet();
    }
    else if(strcmp(argv[1], "2") == 0){
        if(argc != 4) {
            printf("Wrong number of arguments!\n");
            printUsage();
        }
        filepaths = calloc(2, sizeof (char*));
        filepaths[0] = (char *) calloc(strlen(argv[2]), sizeof(char));
        strcpy(filepaths[0], argv[2]);
        filepaths[1] = (char *) calloc(strlen(argv[3]), sizeof(char));
        strcpy(filepaths[1], argv[3]);
        testTwoSheetSimultan();
    } else if(strcmp(argv[1], "3") == 0){
        if(argc != 3) {
            printf("Wrong number of arguments!\n");
            printUsage();
        }
        filepaths = calloc(1, sizeof (char*));
        filepaths[0] = (char *) calloc(strlen(argv[2]), sizeof(char));
        strcpy(filepaths[0], argv[2]);
        testGetDynamicStringInOneCall();
    }
    else {
        printUsage();
    }
    return 0;
}

int main(int argc, char** argv){;
    handleOptions(argc, argv);
    return 0;
}
