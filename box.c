#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Set locale for proper wide-character handling.
    setlocale(LC_ALL, "");

    // If no file is provided, print a fatal error like clang.
    if (argc < 2) {
        fprintf(stderr, "fatal error: no input files\n");
        return 1;
    }

    char *inputFileName = argv[1];

    // Verify that the file has a .indi extension.
    char *ext = strrchr(inputFileName, '.');
    if (ext == NULL || strcmp(ext, ".box") != 0) {
        fprintf(stderr, "error: invalid file extension (expected .indi)\n");
        return 1;
    }

    // Derive the base name (everything before ".indi")
    size_t baseLen = ext - inputFileName;
    char baseName[256];
    if (baseLen >= sizeof(baseName)) {
        fprintf(stderr, "error: filename too long\n");
        return 1;
    }
    strncpy(baseName, inputFileName, baseLen);
    baseName[baseLen] = '\0';

    // Create the output file name by appending ".c" to the base name.
    char outputCFile[256];
    snprintf(outputCFile, sizeof(outputCFile), "%s.c", baseName);

    // Open the input .indi file.
    FILE *inFile = fopen(inputFileName, "r");
    if (inFile == NULL) {
        perror("error opening input file");
        return 1;
    }
    fwide(inFile, 1);

    // Open (or create) the output .c file.
    FILE *outFile = fopen(outputCFile, "w");
    if (outFile == NULL) {
        perror("error creating output file");
        fclose(inFile);
        return 1;
    }
    fwide(outFile, 1);

    // Read from the input file (wide characters) and write to the output file.
    wint_t ch;
    while ((ch = fgetwc(inFile)) != WEOF) {
        if (ch == L'┌' || ch == L'│' || ch == L'─' || ch == L'└' ||
            ch == L'╔' || ch == L'║' || ch == L'═' || ch == L'╚')
            fputwc(L' ', outFile);
        else if (ch == L'┐')
            fputwc(L'(', outFile);
        else if (ch == L'┘')
            fputwc(L')', outFile);
        else if (ch == L'╗')
            fputwc(L'{', outFile);
        else if (ch == L'╝')
            fputwc(L'}', outFile);
        else
            fputwc(ch, outFile);
    }

    fclose(inFile);
    fclose(outFile);

    // Build the clang command.
    char compileCommand[1024];
    snprintf(compileCommand, sizeof(compileCommand), "clang %s", outputCFile);

    // Append additional command-line arguments (from argv[2] onward).
    for (int i = 2; i < argc; i++) {
        strncat(compileCommand, " ", sizeof(compileCommand) - strlen(compileCommand) - 1);
        strncat(compileCommand, argv[i], sizeof(compileCommand) - strlen(compileCommand) - 1);
    }

    // Execute clang and return its exit status.
    return system(compileCommand);
}
