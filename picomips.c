/*
 * picoMIPS.c - picoMIPS Computer Simulator
 *
 * Created by Seokhoon Ko <shko99@gmail.com>
 * Last modified at 2021-05-10
 * You are free to modify this code for learning purposes only
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <conio.h>

//========================================
// Global Definitions
//========================================

typedef unsigned char  UCHAR;
typedef unsigned int   UINT;
typedef unsigned short WORD;

#define MEM_SIZE	0x00010000	// memory size
#define REG_SIZE	8		// register size
#define END_OF_ARG	0xFFFF		// end of argument

UCHAR mem[MEM_SIZE];			// memory image
WORD reg[REG_SIZE] = { 0, };	// register file

UINT data_bgn;			// begin address of DATA section
UINT data_end;			// end address of DATA section
UINT code_bgn;			// begin address of CODE section
UINT code_end;			// end address of CODE section

//========================================
// Utility Functions
// for loadProgram(), inputData()
//========================================

// Read a word data from memory
WORD readWord(UINT addr) {
    return (WORD)((mem[addr] << 8) | mem[addr + 1]);
}

// Write a word data to memory
void writeWord(UINT addr, WORD data) {
    mem[addr	] = (UCHAR)((data & 0xFF00) >> 8);
    mem[addr + 1] = (UCHAR) (data & 0x00FF);
}

// Write variable # of words data to memory
UINT writeWords(UINT addr, WORD data1, ...) {
    va_list ap;
    WORD data;

    va_start(ap, data1);
    for (data = data1; data != END_OF_ARG; data = va_arg(ap, UINT)) {
        writeWord(addr, data);
        addr += 2;
    }
    va_end(ap);

    return addr;	// return last address
}

// Print memory addr1 ~ (addr2 - 1)
void printMemory(char *name, UINT addr1, UINT addr2) {
    const int COL = 8;	// column size
    UINT addr;
    int c = 0;

    if (name != NULL) printf("[%s]\n",name);

    for (addr = addr1; addr < addr2; addr += 2) {
        if (c == 0) printf("%04X:", addr);
        printf(" %04X", readWord(addr));
        if (c == COL - 1) printf("\n");
        c = (c + 1)%COL;
    }
    if (c != 0) printf("\n");
}

// Print registers
void printRegisters() {
    static WORD old_reg[REG_SIZE] = { 0, };	// previous register image
    int i, j;

    for (i = 0; i < 4; i++) {
        printf("\tr%d: %04X (%d)", i, old_reg[i], (short)old_reg[i]);
        if (reg[i] == old_reg[i])
            printf("\t\t\t");
        else {
            printf(" => %04X (%d)\t", reg[i], (short)reg[i]);
            old_reg[i] = reg[i];
        }
        j = i + 4;
        printf("  r%d: %04X (%d)", j, old_reg[j], (short)old_reg[j]);
        if (reg[j] == old_reg[j])
            printf("\n");
        else {
            printf(" => %04X (%d)\n", reg[j], (short)reg[j]);
            old_reg[j] = reg[j];
        }
    }
}

//========================================
// Load AccCom program to memory
// - return start address of program
//========================================
UINT loadProgram() {
    // reset whole memory
    memset(mem, 0, MEM_SIZE);

    /*
        Y = A*A + B*B

        0100: A = -10
        0102: B = 20
        0104: Y

        sub	r0, r0, r0		// r0 = 0
            0000 000 000 000 011		=> 0000 0000 0000 0011 => 0003
        addi r0, r0, 0x10	// r0 = 0x10
            1010 000 000 010000			=> 1010 0000 0001 0000 => A010
        mul	r0, r0, r0		// r0 = r0*r0 = 0x0100
            0000 000 000 000 100		=> 0000 0010 0000 0100 => 0004
        lw	r1, 0(r0)		// r1 = A (0x0100 + 0*2 = 0x0100)
            0100 000 001 000000 (100h)	=> 0100 0000 0100 0000 => 4040
        lw	r2,	1(r0)		// r2 = B (0x0100 + 1*2 = 0x0102)
            0100 000 010 000001 (102h)	=> 0100 0000 1000 0001 => 4081
        mul	r3, r1, r1		// r3 = r1*r1
            0000 001 001 011 100		=> 0000 0010 0101 1100 => 025C
        mul	r4, r2, r2		// r4 = r2*r2
            0000 010 010 100 100		=> 0000 0100 1010 0100 => 04A4
        add	r5, r3, r4		// r5 = r3 + r4
            0000 011 100 101 010		=> 0000 0111 0010 1010 => 072A
        sw	r5, 02(r0)		// Y = r5 (0x0100 + 2*2 = 0x0104)
            0101 000 101 000010 (104h)	=> 0101 0001 0100 0010 => 5142
        halt
            1111 0000 0000 0000								   => F000
    */

    // DATA section ----------------------------------------
    //FFFB -> -5
    //FFF6 -> -10
    //FF9C -> -100
    data_end = writeWords(data_bgn =
                                  0x0100,		0x0064, //	0100:	A = -10
                          0x0FF9C, //	0102:	B = 20
                          0x0000, //	0104:	Y
                          END_OF_ARG);

    // CODE section ----------------------------------------

    code_end = writeWords(code_bgn =
                                  0x0200,		0x0003,
                          0xA010,
                          0x0004,
                          0x4040,
                          0x4081,
                          0x025C,
                          0x04A4,
                          0x072A,
                          0x5142,
                          0xF000,
                          END_OF_ARG);

    // -----------------------------------------------------

    // print memory for verify
    printMemory("DATA", data_bgn, data_end);
    printMemory("CODE", code_bgn, code_end);

    return 0x0200;	// return start address of program
}

//========================================
// Definitions and Functions
// for runProgram()
//========================================



//========================================
// Run program
// - addr: start address of program
// - return exit state = 0: normal exit
//                       1: error exit
//========================================
UINT pc = 0;
int ST_RUN = 0;
int runProgram(UINT code_addr) {
    pc = code_addr;
    UINT ir;
    UINT ir_op;
    UINT ir_fn;
    UINT ir_rd;
    UINT ir_rs;
    UINT ir_rt;
    UINT ir_imm;
    UINT ir_addr;
    UINT ir_jaddr;

    int status = ST_RUN;

    while(status == ST_RUN) {
        //-------fetch cycle ------------/
        ir = pc;
        ir = readWord(ir);
        ir_op = ir & 0xF000;
        pc += (UINT)2;


        //----execution cycle---------------/
        if(ir_op ==0x0000)// ??????????????? R?????? ?????????
        {
             ir_fn = ir & 0x0007;
             ir_rs = (ir & 0x0E00) >> 1;
             ir_rt = (ir & 0x01C0) >> 2;
             ir_rd = (ir & 0x0038) >> 3;
             ir_rs = ir_rs >> 8;
             ir_rt = ir_rt >> 4;


             signed short temp_rs = (signed short)(ir_rs);
             signed short temp_rt = (signed short)(ir_rt);
             signed short temp_rd = (signed short)(ir_rd);

             if(ir_fn == 0x0000) //and
             {
                 reg[temp_rd] = reg[temp_rs] & reg[temp_rt];
             }
            else if(ir_fn == 0x0001) //or
             {
                 reg[temp_rd] = reg[temp_rs] | reg[temp_rt];
             }
             else if(ir_fn == 0x002) //add
             {
                 reg[temp_rd] = reg[temp_rs] + reg[temp_rt];
             }
            else if(ir_fn == 0x0003) //sub
             {
                reg[temp_rd] = reg[temp_rs] - reg[temp_rt];
             }
            else if(ir_fn == 0x0004) //mul
             {
                reg[temp_rd] = reg[temp_rs] * reg[temp_rt];
             }
            else if(ir_fn == 0x0005) //div
             {
                 reg[temp_rd] = reg[temp_rs] / reg[temp_rt];
             }
        }
        else{
            ir_rs = (ir & 0x0E00) >> 1;
            ir_rt = (ir & 0x01C0) >> 2;
            ir_imm = (ir & 0x003F);
            ir_addr = (ir & 0x003F);
            ir_jaddr = (ir & 0x0FFF);
            ir_rs = ir_rs >> 8;
            ir_rt = ir_rt >> 4;
            signed short temp_rs = (signed short)(ir_rs);
            signed short temp_rt = (signed short)(ir_rt);
            signed short temp_imm = (signed short)(ir_imm);
            signed short temp_addr = (signed short)(ir_addr);
            if(ir_op == 0xF000) ST_RUN = 1;
            else if(ir_op== 0xA000) //addi
            {
                reg[temp_rt] = reg[temp_rs] + temp_imm;
            }
            else if(ir_op == 0xB000) //subi
            {
                reg[temp_rt] = reg[temp_rs] - temp_imm;
            }
            else if(ir_op == 0x4000) //lw
            {
                reg[temp_rt] = readWord(reg[temp_rs] + temp_addr * 2);
            }
            else if(ir_op == 0x5000) //sw
            {
                writeWord(reg[temp_rs] + temp_addr * 2,reg[temp_rt]);
            }
            else if(ir_op == 0x1000) //beq
            {
                if(reg[temp_rs] == reg[temp_rt])
                {
                    pc += temp_addr * 2;
                }
                else {
                    printf("%04x\n",pc-2);
                    printRegisters();
                    continue;
                }
            }
            else if(ir_op == 0x3000) //Jump
            {
                printf("%04x\n",pc-2);
                printRegisters();
                ir_jaddr = ir_jaddr | 0xF000;
                signed short temp_jaddr = (signed short)(ir_jaddr);
                pc +=  temp_jaddr * 2;
            }
        }
        printf("%04x\n",pc-2);
        printRegisters();
    }
    return 0;
}

//========================================
// Main Function
//========================================
int main() {
    int exit_code;		// 0: normal exit, 1: error exit
    UINT start_addr;	// start address of program

    printf("========================================\n");
    printf(" picoMIPS Computer Simulator\n");
    printf("     modified by 201602955 Jang Hyeonjun\n");
    printf("========================================\n");

    printf("*** Load ***\n");
    start_addr = loadProgram();

    printf("*** Run ***\n");
    exit_code = runProgram(start_addr);

    printf("*** Exit %d ***\n", exit_code);

    printMemory("DATA", data_bgn, data_end);
}
