#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COLUMNS 10
#define MAX_COLUMN_NAME 50
#define MAX_TABLE_NAME 50
#define MAX_ROW_SIZE 512
#define PAGE_SIZE 1024
#define MAX_STRING_SIZE 100

typedef enum {
    INTEGER,
    VARCHAR
} ColumnType;

typedef struct {
    char name[MAX_COLUMN_NAME];
    ColumnType type;
} Column;

typedef struct {
    char name[MAX_TABLE_NAME];
    int columnCount;
    Column columns[MAX_COLUMNS];
} Table;

typedef struct {
    char tableName[MAX_TABLE_NAME];
    time_t creationTime;
    int rowCount;
} PageHeader;

void createTable(Table *table) {
    printf("Enter table name: ");
    scanf("%s", table->name);
    table->columnCount = 0;
    
    while (1) {
        printf("Enter column type (INTEGER or VARCHAR) and name (or END to finish): ");
        char type[10], name[MAX_COLUMN_NAME];
        scanf("%s", type);
        
        if (strcmp(type, "END") == 0) {
            break;
        }
        
        scanf("%s", name);
        
        Column column;
        strcpy(column.name, name);
        
        if (strcmp(type, "INTEGER") == 0) {
            column.type = INTEGER;
        } else if (strcmp(type, "VARCHAR") == 0) {
            column.type = VARCHAR;
        } else {
            printf("Invalid column type. Please use INTEGER or VARCHAR.\n");
            continue;
        }
        
        table->columns[table->columnCount++] = column;
    }
    
    char filename[100];
    sprintf(filename, "%s.bin", table->name);
    FILE *file = fopen(filename, "wb");
    
    PageHeader header;
    strcpy(header.tableName, table->name);
    header.creationTime = time(NULL);
    header.rowCount = 0;
    
    fwrite(&header, sizeof(PageHeader), 1, file);
    fclose(file);
}

void insertIntoTable(Table *table) {
    char filename[100];
    sprintf(filename, "%s.bin", table->name);
    FILE *file = fopen(filename, "rb+");
    
    if (!file) {
        printf("Table not found.\n");
        return;
    }
    
    PageHeader header;
    fread(&header, sizeof(PageHeader), 1, file);
    
    fseek(file, 0, SEEK_END);
    
    char row[MAX_ROW_SIZE];
    memset(row, 0, MAX_ROW_SIZE);
    
    int offset = 0;
    
    for (int i = 0; i < table->columnCount; ++i) {
        if (table->columns[i].type == INTEGER) {
            int value;
            printf("Enter INTEGER value for %s: ", table->columns[i].name);
            scanf("%d", &value);
            memcpy(row + offset, &value, sizeof(int));
            offset += sizeof(int);
        } else if (table->columns[i].type == VARCHAR) {
            char value[MAX_STRING_SIZE];
            printf("Enter VARCHAR value for %s: ", table->columns[i].name);
            scanf("%s", value);
            strncpy(row + offset, value, MAX_STRING_SIZE);
            offset += MAX_STRING_SIZE;
        }
    }
    
    fwrite(row, MAX_ROW_SIZE, 1, file);
    header.rowCount++;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(PageHeader), 1, file);
    fclose(file);
}

void selectFromTable(Table *table, char columns[][MAX_COLUMN_NAME], int columnCount) {
    char filename[100];
    sprintf(filename, "%s.bin", table->name);
    FILE *file = fopen(filename, "rb");
    
    if (!file) {
        printf("Table not found.\n");
        return;
    }
    
    PageHeader header;
    fread(&header, sizeof(PageHeader), 1, file);
    
    char row[MAX_ROW_SIZE];
    int columnOffsets[MAX_COLUMNS];
    int offset = 0;
    
    for (int i = 0; i < table->columnCount; ++i) {
        columnOffsets[i] = offset;
        if (table->columns[i].type == INTEGER) {
            offset += sizeof(int);
        } else if (table->columns[i].type == VARCHAR) {
            offset += MAX_STRING_SIZE;
        }
    }
    
    printf("Table: %s\n", header.tableName);
    for (int i = 0; i < columnCount; ++i) {
        printf("%s\t", columns[i]);
    }
    printf("\n");
    
    for (int i = 0; i < header.rowCount; ++i) {
        fread(row, MAX_ROW_SIZE, 1, file);
        for (int j = 0; j < columnCount; ++j) {
            for (int k = 0; k < table->columnCount; ++k) {
                if (strcmp(columns[j], table->columns[k].name) == 0) {
                    if (table->columns[k].type == INTEGER) {
                        int value;
                        memcpy(&value, row + columnOffsets[k], sizeof(int));
                        printf("%d\t", value);
                    } else if (table->columns[k].type == VARCHAR) {
                        char value[MAX_STRING_SIZE];
                        strncpy(value, row + columnOffsets[k], MAX_STRING_SIZE);
                        printf("%s\t", value);
                    }
                }
            }
        }
        printf("\n");
    }
    
    fclose(file);
}

int main() {
    Table table;
    while (1) {
        char command[100];
        printf("Enter command (CREATE TABLE, INSERT INTO, SELECT, or EXIT): ");
        scanf(" %[^\n]", command);
        
        if (strncmp(command, "CREATE TABLE", 12) == 0) {
            createTable(&table);
        } else if (strncmp(command, "INSERT INTO", 11) == 0) {
            insertIntoTable(&table);
        } else if (strncmp(command, "SELECT", 6) == 0) {
            char columns[MAX_COLUMNS][MAX_COLUMN_NAME];
            int columnCount = 0;
            char *token = strtok(command + 7, ", ");
            while (token) {
                strcpy(columns[columnCount++], token);
                token = strtok(NULL, ", ");
            }
            selectFromTable(&table, columns, columnCount);
        } else if (strncmp(command, "EXIT", 4) == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }
    return 0;
}
