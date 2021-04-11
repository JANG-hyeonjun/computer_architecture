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
                                  0x0100,		0x0000,	// 0100: height = 0
                          0x0001,	// 0102: star= 1
                          0x0001,	// 0104: i = 1
                          0x0001,	// 0106: j = 1
                          0x0000,   // 0108: k = 0
                          0x0000,	// 010A:  '\0'
                          END_OF_ARG);

    // CODE section ----------------------------------------

    code_end = writeWords(code_bgn =
                                  0x0200,		0x1100,	// 0200: LDA height
                          0xA24C,	// 		 0202: BRZ -> HLT로
                          0x1100,   //       0204: LDA Height
                          0x4104,	// 		 0206: SUB i
                          0xA24C,	// 		 0208: BRZ -> HLT로
                          0x1108,   //       020A: LDA K
                          0x4108,   //       020C: SUB k
                          0x2108,   //       020E: STA k
                          0x1106,   //       0210: LDA j
                          0x4106,   //       0212: SUB j
                          0x8002,   //       0214: IAC
                          0x2106,   //       0216: STA j
                          0x1100,   //       0218: LDA height
                          0x4104,   //       021A: SUB i
                          0x4108,   //       021C: SUB k
                          0xA22A,   //       021E: BRZ 22A
                          0xC020,   //       0220: PRC ' '
                          0x1108,	// 		 0222: LDA k
                          0x8002,	// 		 0224: IAC
                          0x2108,	// 		 0226: STA k
                          0x5218,   //       0228: JMP 218
                          0x1102,   //       022A: LDA star
                          0x4106,   //       022C: SUB j
                          0xA23A,   //       022E: BRZ 23A
                          0xC023,   //       0230: PRC '#'
                          0x1106,   //       0232: LDA j
                          0x8002,   //       0234: IAC
                          0x2106,   //       0236: STA j
                          0x522A,   //       0238: JMP 22A
                          0x1102,   //       023A: LDA star
                          0x8002,   //       023C: IAC
                          0x8002,   //       023E: IAC
                          0x2102,   //       0240: STA star
                          0x1104,   //       0242: LDA i
                          0x8002,   //       0244: IAC
                          0x2104,   //       0246: STA i
                          0xC00A,   //       0248: PRC '\n'
                          0x5204,   //       024A: JMP 204
                          0x8000,	// 		 024C: HLT
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
    printf("Draw Pyramid\n");

    // input data
    inputNumber("0100: Height = ", 0x0100);


    // print DATA section for verify
    printMemory("DATA", data_bgn, data_end);
}

//========================================
// Definitions and Functions
// for runProgram()
//========================================

//여기서 부터 코딩하면 된다.

int c_num = 0;
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

UINT pc = 0;
UINT acc =0;
int ST_RUN = 0;
//========================================
// Run program
// - addr: start address of program
// - return exit state = 0: normal exit
//                       1: error exit
//========================================
int runProgram(UINT addr) {
    int status = ST_RUN;
    UINT mar;
    UINT mbr;
    UINT ir;
    UINT ir_i;
    UINT ir_a;
    UINT psw;

    pc = addr;

    while(status == ST_RUN) {
        //-------fetch cycle ------------/
        mar = pc;
        mbr = readWord(mar);
        ir = mbr;
        ir_i = ir & 0xF000;
        ir_a = ir & 0x0FFF;
        pc += (UINT)2;

       //----execution cycle---------------/

        if(ir_i == 0x0000){
            break;
        }else if (ir_i == 0x1000) //LDA
        {
            mar = ir_a;
            mbr = readWord(mar);
            acc = mbr;
            if (acc > 0x8000) psw = 0x0001;
            else psw = 0x0000;
        } else if (ir_i == 0x2000) //STA
        {
            mar = ir_a;
            writeWord(mbr, acc);
        } else if (ir_i == 0x3000) //ADD
        {
            mar = ir_a;
            mbr = readWord(mar);
            c_num = accnum2cint(acc) +  accnum2cint(mbr);
            acc = cint2accnum(c_num);
            if (acc > 0x8000) psw = 0x0001;
            else psw = 0x0000;
        } else if (ir_i == 0x4000) //SUB
        {
            mar = ir_a;
            mbr = readWord(mar);
            c_num = accnum2cint(acc) -  accnum2cint(mbr);
            acc = cint2accnum(c_num);
            if (acc > 0x8000) psw = 0x0001;
            else psw = 0x0000;
        } else if (ir_i == 0x5000) //JMP
        {
            mar = ir_a;
            mbr = readWord(mar);
            pc = mbr;
        } else if (ir_i == 0x7000)//MUL
        {
            mar = ir_a;
            mbr = readWord(mar);
            c_num = accnum2cint(acc) *  accnum2cint(mbr);
            acc = cint2accnum(c_num);
            if (acc > 0x8000) psw = 0x0001;
            else psw = 0x0000;
        } else if (ir_i == 0xA000) //BRN
        {
            if (psw == 0x0001) //음수일 경우 지정한곳으로 분기해야한다.
            {
                mar = ir_a;
                mbr = readWord(mar);
                pc = mbr;
            } else {
                continue;
            }
        } else if (ir_i == 0xB000) {
            mar = ir_a;
            mbr = readWord(mar);
            prt(mbr);
        } else if (ir_i == 0xC000) {

            mar = ir_a;
            mbr = readWord(mar);
            prc(mbr);

        } else if (ir_i == 0xD000) {
            mar = ir_a;
            mbr = readWord(mar);
            prs(mbr);
        } else if (ir_i == 0x8002) //IAC
        {
            acc = cint2accnum( accnum2cint(acc) + 1);
        } else if (ir_i == 0x8000) {
            ST_RUN = 1;
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
