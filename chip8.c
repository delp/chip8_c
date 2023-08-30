/* author: Alex R. Delp
 * Chip8 Emulator in c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

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

void emulate_cycle() {
    
    //fetch opcode
    uint8_t a = cpu.memory[cpu.pc];
    uint8_t b = cpu.memory[cpu.pc + 0x01];

    //build opcode
    uint16_t opcode = a;
    opcode = opcode << 8;
    opcode +=  b;
    printf("Opcode: 0x%X\n", opcode);

	// increment pc
	cpu.pc += 0x02;

	// Decode Opcode
	/*
	   "X: The second nibble. Used to look up one of the 16 registers (VX) from V0 through VF.
	    Y: The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
	    N: The fourth nibble. A 4-bit number.
	    NN: The second byte (third and fourth nibbles). An 8-bit immediate number.
	    NNN: The second, third and fourth nibbles. A 12-bit immediate memory address."
	*/

    //nibble 1
    uint16_t foo = opcode & 0xF000;
    foo = foo >> 8;
    uint8_t prefix = (uint8_t)foo;

    //X
    foo = opcode & 0x0F00;
    foo = foo >> 8;
    uint8_t x = (uint8_t)foo;

    //Y
    foo = opcode & 0X00F0;
    foo = foo >> 8;
    uint8_t y = (uint8_t)foo;

    uint8_t N = opcode & 0x000F;
    uint8_t NN = (uint8_t) opcode & 0x00FF;
    uint16_t NNN = opcode & 0x0FFF;

    printf("Decoded Opcode: X: %X, Y: %X, N: %X, NN:: %X, NNN:: %X\n",x, y, N, NN, NNN);

    switch(prefix) {
        case 0x00:
            if(NN == 0xE0) {
                for(int i = 0; i < 64 * 32; i++) {
                    cpu.gfx[i] = 0;
                }
                break;
            } else if(NN == 0xEE) {
                cpu.pc = cpu.stack[cpu.sp];
                cpu.sp--;
                break;
            }

        case 0x10:
            printf("JP addr\n\n");
            cpu.pc = NNN;
            break;

        case 0x20:
            printf("CALL addr\n\n");
            cpu.sp++;
            cpu.stack[cpu.sp] = cpu.pc;
            cpu.pc = NNN;
            break;

        case 0x30:
            printf("SE Vx\n\n");
            if(cpu.v[x] == NN) {
                cpu.pc += 0x02;
            }
            break;

        case 0x40:
            printf("SNE Vx\n\n");
            if(cpu.v[x] != NN) {
                cpu.pc += 0x02;
            }
            break;

        case 0x50:
            printf("SE Vx, Vy\n\n");
            if(cpu.v[x] == cpu.v[y]) {
                cpu.pc += 0x02;
            }
            break;

        case 0x60:
            printf("Set Register\n\n");
            cpu.v[x] = NN;
            break;

        case 0x70:
            printf("ADD Vx, byte\n\n");
            cpu.v[x] = cpu.v[x] + NN;
            break;

        case 0x90:
            printf("SNE Vx, Vy\n\n");
            if(cpu.v[x] != cpu.v[y]) {
                cpu.pc += 0x02;
            }
            break;

        case 0xA0:
            printf("Load I\n\n");
            cpu.i = NNN;
            break;

        case 0xB0:
            printf("JP V0, addr");
            cpu.pc = cpu.v[0] + NNN;
            break;
        
        case 0xC0:
            printf("RND Vx, byte");
            srand(time(NULL));
            uint8_t random_byte = (uint8_t)(rand() % 256);
            cpu.v[x] = random_byte & NN;
            break;
       
        case 0xE0:
            if(NN == 0x9E) {
                printf("SKP Vx");
                    if(cpu.key[x]) {
                        cpu.pc += 0x02;
                    }
                break;

            } else if (NN == 0xA1) {
                printf("SKNP Vx");
                    if(!cpu.key[x]) {
                        cpu.pc += 0x02;
                    }
                break;
            }
    }
}

/* Zero out all the cpu's stuff*/
void init_cpu() {
    for(int i = 0; i < 16; i++) {
        cpu.stack[i] = 0;
    }
    cpu.sp = 0;
    for(int i = 0; i < 4096; i++) {
        cpu.memory[i] = 0;
    }
    for(int i = 0; i < 16; i++) {
        cpu.v[i] = 0;
    }
    cpu.pc = 0;
    cpu.i = 0;
    cpu.delaytimer = 0;
    cpu.soundtimer = 0;
    for(int i = 0; i < 64 * 32; i++) {
        cpu.gfx[i] = 0;
    }
    for(int i = 0; i < 16; i++) {
        cpu.key[i] = 0;
    }
}

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Usage: chip8 <rom-file-name>\n");
        exit(0);
    }
    
    init_cpu();
    cpu.pc = 0x200;

    struct chip8_rom* rom = read_rom_from_file(argv[1]);
    
    //TODO: Validate it?
    load_rom(rom, 0x200);

    free(rom);

    //Begin running program
    //Fetch instruction
    //decode instruction
    //execute instruction

    print_cpu_state(cpu);
    //Vscode is stupid so we have to flush in order to see our terminal output
    fflush(stdout);

    while(1) {
        //read input and update the memory mapped input?
        emulate_cycle();
        //draw the screen
    }

    return 0;
}
