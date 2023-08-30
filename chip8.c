/* author: Alex R. Delp
 * Chip8 Emulator in c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


uint8_t fontset[] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80, //F
};


struct chip8 {
	
    uint16_t stack[16];
	uint8_t sp; //Stack Pointer

    uint8_t memory[4096];
    uint8_t v[16]; //V registers (V0-VF)

	uint16_t pc; //Program Counter

	uint16_t i; //Index Register

    uint8_t delaytimer;
	uint8_t soundtimer;

	uint8_t gfx[64 * 32]; //Screen Memory
    //64 pixels wide and 32 pixels tall
	
    uint8_t key[16]; //Memory Mapped Keyboard
};

struct chip8_rom{
    char* data;
    int length;
};

struct chip8 cpu;

/* Prints the internal state of the CPU.*/
void print_cpu_state(struct chip8 cpu) {
    printf("PC: 0x%X   I: 0x%X\n", cpu.pc, cpu.i);
	printf("V: %X %X %X %X   %X %X %X %X\n   %X %X %X %X   %X %X %X %X\n",
		cpu.v[0], cpu.v[1], cpu.v[2], cpu.v[3],
		cpu.v[4], cpu.v[5], cpu.v[6], cpu.v[7],
		cpu.v[8], cpu.v[9], cpu.v[10], cpu.v[11],
		cpu.v[12], cpu.v[13], cpu.v[14], cpu.v[15]);
}

/* Loads a program rom into memory at the given offset. Often this is 0x200*/
void load_rom(struct chip8_rom* rom, uint16_t offset) {
    for(int i = 0; i < rom->length; i++) {
        int a = i + offset;
        cpu.memory[a] = rom->data[i];
    }
}

struct chip8_rom* read_rom_from_file(char* filename) {   
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening ROM file.");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* file_contents = (char*) malloc(file_size + 1);
    if (file_contents == NULL) {
        perror("Memory allocation error");
        fclose(file);
        exit(1);
    }

    size_t read_size = fread(file_contents, 1, file_size, file);
    if (read_size != file_size) {
        perror("Error reading file");
        free(file_contents);
        fclose(file);
        exit(1);
    }

    file_contents[file_size] = '\0'; //null terminator

    fclose(file);

    struct chip8_rom* rom = malloc(sizeof(struct chip8_rom));
    rom->data = file_contents;
    rom->length = file_size - 1; //the ROM struct does not need the null terminator
    return rom;
}

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Usage: chip8 <rom-file-name>\n");
        exit(0);
    }

    struct chip8_rom* rom = read_rom_from_file(argv[1]);
    
    //TODO: Validate it?
    //TODO load it into memory
    load_rom(rom, 0x200);

    free(rom);

    //Begin running program
    //Fetch instruction
    //decode instruction
    //execute instruction

    print_cpu_state(cpu);
    //Vscode is stupid so we have to flush in order to see our terminal output
    fflush(stdout);
    //read input and update the memory mapped input?

    return 0;
}
