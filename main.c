/*
 * AccCom.c - Accumulator Computer Simulator
 *
 * Created by Seokhoon Ko <shko99@gmail.com>
 * Last modified at 2021-03-13
 * You are free to modify this code for learning purposes only
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <curses.h>

//========================================
// Global Definitions
//========================================

typedef unsigned char UCHAR;
typedef unsigned int  UINT;

#define MEM_SIZE	0x0FFF	// memory size
#define END_OF_ARG	0xFFFF	// end of argument

UCHAR mem[MEM_SIZE];	// memory image

UINT data_bgn;			// begin address of DATA section
UINT data_end;			// end address of DATA section
UINT code_bgn;			// begin address of CODE section
UINT code_end;			// end address of CODE section

int tos = 0;			// top of stack

//========================================
// Utility Functions
// for loadProgram(), inputData()
//========================================

// Read a word data from memory
UINT readWord(UINT addr) {
    return (mem[addr] << 8) | mem[addr + 1];
}

// Write a word data to memory
void writeWord(UINT addr, UINT data) {
    mem[addr	] = (UCHAR)((data & 0xFF00) >> 8);
    mem[addr + 1] = (UCHAR) (data & 0x00FF);
}

// Write variable # of words data to memory
UINT writeWords(UINT addr, UINT data1, ...) {
    va_list ap;
    UINT data;
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

// Convert AccCom number to C int type
int accnum2cint(UINT n) {
   // printf("\n!!!!!!UINT n:%04x\n",n);
    UINT sign_n = n & 0x8000;	// sign of n
   // printf("\n!!!!!!UINT sign_n:%04x\n",sign_n);
    UINT data_n = n & 0x7FFF;	// absolute value of n
   // printf("\nUINT data_n:!!!!!!%d\n",data_n);
    int i = (sign_n ? -1 : 1)*data_n;
   // printf("\nint i: !!!!!!%d\n",i);
    return i;
}

// Convert C int to AccCom number type
UINT cint2accnum(int i) {
    UINT sign_n = (UINT)((i < 0) ? 0x8000 : 0);
    UINT data_n = (UINT)abs(i) & 0x7FFF;
    UINT n = sign_n | data_n;
    return n;
}

// Scan a number and write to memory
void inputNumber(char* msg, UINT addr) {
    int n;

    printf("%s",msg);
    scanf("%d", &n);
    writeWord(addr, cint2accnum(n));
}

// stack push function
// - stack area: mem[0] ~ mem[0x00FF]
void push(UINT addr) {
    if (tos == 0x00FFFF) {
        printf("Error: Stack full");
        exit(-1);
    }
    writeWord(tos, addr);
    tos += 2;
}

// stack pop function
UINT pop() {
    if (tos == 0) {
        printf("Error: Stack empty");
        exit(-1);
    }
    tos -= 2;
    return readWord(tos);
}

//========================================
// Load AccCom program to memory
// - return start address of program
//========================================
UINT loadProgram() {
    // reset whole memory
    memset(mem, 0, MEM_SIZE);

    /*
        A=7		// input data
        B=-5	// input data
        X		// result
        10		// constant 10
        "X="	// constant "X="

        X = A*B
        X = X + 10
        print("X=", X, '\n')
    */

    // DATA section ----------------------------------------

    data_end = writeWords(data_bgn =
                                  0x0100,		0x0007,	// 0100: A=7
                          0x8005,	// 0102: B=-5
                          0x0000,	// 0104: C
                          0x0000,	// 0106: X
                          0x4158,   // 0108: AX
                          0x5E32,   // 010A: ^2
                          0x2B42,   // 010C: +B
                          0x582B,   // 010E: X+
                          0x433d,   // 010F: C=
                          0x0000,   // 0112: Y
                          0x0000,	// 0114:  '\0'
                          END_OF_ARG);

    // CODE section ----------------------------------------

    code_end = writeWords(code_bgn =
                                  0x0200,		0x1100,	// 0200: LDA A
                          0x7106,	// 		 MUL X
                          0x7106,	// 		 MUL X
                          0x2112,	// 		 STA Y
                          0x1102,   //       LDA B
                          0x7106,   //       MUL X
                          0x3112,   //       ADD Y
                          0x2112,   //       STA Y
                          0x1112,   //       LDA Y
                          0x3104,   //       ADD C
                          0x2112,   //       STA Y
                          0xD108,	// 		 PRS STR
                          0xB112,	// 		 PRT Y
                          0xC00A,	// 		 PRC '\n'
                          0x8000,	// 		 HLT
                          END_OF_ARG);

    // -----------------------------------------------------

    // print memory for verify
    printMemory("DATA", data_bgn, data_end);
    printMemory("CODE", code_bgn, code_end);

    return code_bgn;	// return start address of program
}

//========================================
// Keyboard input for specific variables
//========================================
void inputData() {
    // print problem summary
    printf("Y = AX^2+BX+C\n");

    // input data
    inputNumber("0100: A = ", 0x0100);
    inputNumber("0102: B = ", 0x0102);
    inputNumber("0104: C = ",0x0104);
    inputNumber("0106: X = ",0x0106);

    // print DATA section for verify
    printMemory("DATA", data_bgn, data_end);
}

//========================================
// Definitions and Functions
// for runProgram()
//========================================

//여기서 부터 코딩하면 된다.
int PC;
int MAR;
int MBR;
int IR;
int c_num = 0;
UINT ACC;
int find_instruction;
char instruction[10];
char temp_is[10];
char address[5];
//fetch cycle 과 executuion cycle에 쓰일 변수

//여기다 divided conquer가법으로 함수를 작성하는건 어떨까


// PRT (PRinT) instruction
// print a AccCom number at mem[addr]
void prt(UINT addr) {
    UINT n = readWord(addr);
    int i = accnum2cint(n);
    printf("%d", i);
}

// PRC (PRint Char) instruction
// print a ASCII char
void prc(int ch) {
    printf("%c", ch);
}

// PRS (PRint String) instruction
// print string at mem[addr]
void prs(UINT addr) {
    int ch = (int)mem[addr];
    while (ch != '\0') {
        printf("%c", ch);
        ch = (int)mem[++addr];
    }
}

//========================================
// Run program
// - addr: start address of program
// - return exit state = 0: normal exit
//                       1: error exit
//========================================
int runProgram(UINT addr) {

   for(int PC= addr; PC < code_end;)
   {
        int IR_address = 0;
        int temp = 0;
        sprintf(instruction,"%02x%02x",mem[PC],mem[PC+1]);

        for(int j =0; j < 4; j++)
        {
            if(instruction[j] >= 'a' && instruction[j] <= 'z')
                instruction[j] = instruction[j] - 32;
        }
        if(instruction[0] == '1') //LDA
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);
            temp = strtol(instruction,NULL,16);
            IR_address = strtol(address,NULL,16);

            PC = PC + 2;
            IR = temp;
            sprintf(temp_is,"%02x%02x",mem[IR_address],mem[IR_address+1]);
            c_num =  accnum2cint(readWord(IR_address));
            ACC = cint2accnum(c_num);
        }
        else if(instruction[0] == '2') //STA
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);

            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);
            IR = temp;
            PC = PC + 2;
            //이제 메모리를 바꿔야한다.
            writeWord(IR_address,ACC);
            //
        }
        else if(instruction[0] == '3') //ADD
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);
            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);
            PC = PC + 2;
            int temp_num =  accnum2cint(readWord(IR_address));
            c_num = accnum2cint(ACC) + temp_num;
            ACC = cint2accnum(c_num);
        }
        else if(instruction[0] == '4') //SUB
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);
            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);
            PC = PC + 2;
            int temp_num =  accnum2cint(readWord(IR_address));
            c_num = accnum2cint(ACC) - temp_num;
            ACC = cint2accnum(c_num);
        }
        else if(instruction[0] == '5') //JMP
        {
            printf("JMP처리\n");
            PC = PC + 2;
        }
        else if(instruction[0] == '7')//MUL
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);
            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);
            PC = PC + 2;
            int temp_num =  accnum2cint(readWord(IR_address));
            c_num = accnum2cint(ACC) * temp_num;
            ACC = cint2accnum(c_num);

        }
        else if(instruction[0] == 'B')
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);
            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);
            PC = PC + 2;
            prt(IR_address);

        }
        else if(instruction[0] == 'C')
        {

            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);

            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);


           PC = PC + 2;
            prc(IR_address);

        }
        else if(instruction[0] == 'D')
        {
            sprintf(address,"%c%c%c",instruction[1],instruction[2],instruction[3]);

            IR_address = strtol(address,NULL,16);
            temp = strtol(instruction,NULL,16);
            PC = PC + 2;

            prs(IR_address);
        }
        else if(strcmp(instruction,"8002") == 0)
        {
            printf("IAC처리\n");
            PC = PC + 2;
            ACC = cint2accnum(c_num + 1);
        }
        else if(strcmp(instruction,"8000")== 0) {
            PC = PC + 2;
            return 0;
        }
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
    printf(" AccCom: Accumulator Computer Simulator\n");
    printf("     modified by 201602955 Jang Hyeonjun\n");
    printf("========================================\n");

    printf("*** Load ***\n");
    start_addr = loadProgram();

    printf("*** Input ***\n");
    inputData();

    printf("*** Run ***\n");
    exit_code = runProgram(start_addr);

    printf("*** Exit %d ***\n", exit_code);
}
