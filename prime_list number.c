#define _CRT_SECURE_NO_WARNINGS

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
//#include <conio.h>





//========================================

// Global Definitions

//========================================



typedef unsigned char UCHAR;
typedef unsigned int UINT;



#define MEM_SIZE 0x0FFF // memory size
#define END_OF_ARG 0xFFFF // end of argument



UCHAR mem[MEM_SIZE]; // memory image



UINT data_bgn; // begin address of DATA section
UINT data_end; // end address of DATA section
UINT code_bgn; // begin address of CODE section
UINT code_end; // end address of CODE section



int tos = 0; // top of stack



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
    mem[addr] = (UCHAR)((data & 0xFF00) >> 8);
    mem[addr + 1] = (UCHAR)(data & 0x00FF);
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
    return addr; // return last address
}



// Print memory addr1 ~ (addr2 - 1)

void printMemory(char *name, UINT addr1, UINT addr2) {
    const int COL = 8; // column size
    UINT addr;
    int c = 0;

    if (name != NULL) printf("[%s]\n", name);



    for (addr = addr1; addr < addr2; addr += 2) {
        if (c == 0) printf("%04X:", addr);
        printf(" %04X", readWord(addr));
        if (c == COL - 1) printf("\n");
        c = (c + 1) % COL;
    }
    if (c != 0) printf("\n");
}



// Convert AccCom number to C int type

int accnum2cint(UINT n) {
    // printf("\n!!!!!!UINT n:%04x\n",n);
    UINT sign_n = n & 0x8000; // sign of n
    // printf("\n!!!!!!UINT sign_n:%04x\n",sign_n);
    UINT data_n = n & 0x7FFF; // absolute value of n
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
    printf("%s", msg);
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

    A=7 // input data

    B=-5 // input data

    X // result

    10 // constant 10

    "X=" // constant "X="



    X = A*B

    X = X + 10

    print("X=", X, '\n')

    */



    // DATA section ----------------------------------------



    data_end = writeWords(data_bgn =
                                  0x0100, 0x0000, // 0100: a = 0
                          0x0000, // 0102: b= 0
                          0x0000, // 0104: n
                          0x0000, // 0106: flag
                          0x0000, // 0108: i
                          0x0001, // 010A: true
                          0x0000,//  010C: false
                          0x0002, // 010E: init = 2
                          0x0001, // 0110: j
                          0x0000, // 0112: div
                          0x0001, // 0114: one
                          0x0000, // 0116: zero
                          END_OF_ARG);



    // CODE section ----------------------------------------



    code_end = writeWords(code_bgn =
                                  //#isDivisor
                                  0x0200,0x110E, // 0200: LDA init
                                  0x2108, // 0202: STA i
                                  0x1104, // 0204: LDA n
                          0x4114, // 0206: SUB one
                          0x4108, // 0208: SUB i
                          0xA240, // 020A: BRN 240
                          0x1116, // 020C: LDA zero
                          0x2110, // 020E: STA j
                          0x1108, // 0210: LDA i
                          0x7110, // 0212: MUL j
                          0x2112, // 0214: STA div
                          0x1104, // 0216: LDA n
                          0x4112, // 0218: SUB div
                          0xA238, // 021A: BRN 238
                          0x1104, // 021C: LDA n
                          0x4112, // 021E: SUB div
                          0x9224, // 0220: BRZ 224
                          0x522A, // 0222: JMP 22A
                          0x110A, // 0224: LDA true
                          0x2106, // 0226: STA flag
                          0x8005, // 0228: RET
                          0x1110, // 022A: LDA j
                          0x8002, // 022C: IAC
                          0x2110, // 022E: STA j
                          0x1108, // 0230: LDA i
                          0x7110, // 0232: MUL j
                          0x2112, // 0234: STA div
                          0x5216, // 0236: JMP 216
                          0x1108, // 0238: LDA i
                          0x8002, // 023A: IAC
                          0x2108, // 023C: STA i
                          0x5204, // 023E: JMP 204
                          0x8005, // 0240: RET
                                //#isPrime
                          0x6200, // 0242: CAL #isDivisor
                          0x8005, // 0244: RET
                                //main
                          0x1100, //0246: LDA a
                          0x2104, //0248: STA n
                          0x110E, //024A: LDA init
                          0x4100, //024C: SUB a
                          0xA254, //024E: BRN 254
                          0x110E, //0250: LDA init
                          0x2104, //0252: STA n
                          0x1102, //0254: LDA b
                          0x4104, //0256: SUB n
                          0xA272, //0258: BRN 272
                          0x110C, //025A: LDA false
                          0x2106, //025C: STA flag
                          0x6242, //025E: cal #isPrime
                          0x1106, //0260: LDA flag
                          0x410A, //0262: SUB true
                          0x926A, //0264: BRZ 26A
                          0xB104, //0266: PRC (n)
                          0xC020, //0268: PRC ' '
                          0x1104, //026A: LDA n
                          0x8002, //026C: IAC
                          0x2104, //026E: STA n
                          0x5254, //0270: JMP 254
                          0x8000, //0272: HLT
                          END_OF_ARG);



    // -----------------------------------------------------

    // print memory for verify

    printMemory("DATA", data_bgn, data_end);
    printMemory("CODE", code_bgn, code_end);

    return 0x0246; // return start address of program
}



//========================================

// Keyboard input for specific variables

//========================================

void inputData() {
    // print problem summary
    printf("Prime list of A to B\n");
    // input data
    inputNumber("0100: A = ", 0x0100);
    inputNumber("0102: B = ", 0x0102);
    // print DATA section for verify
    printMemory("DATA", data_bgn, data_end);
}



//========================================

// Definitions and Functions

// for runProgram()

//========================================



//????????? ?????? ???????????? ??????.



int c_num = 0;

//fetch cycle ??? executuion cycle??? ?????? ??????



//????????? divided conquer???????????? ????????? ??????????????? ?????????





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
UINT acc = 0;

int ST_RUN = 0;

//========================================

// Run program

// - addr: start address of program

// - return exit state = 0: normal exit

// 1: error exit

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



    while (status == ST_RUN) {
        //-------fetch cycle ------------/
        mar = pc;
        //printf("%04x\n",pc);
        mbr = readWord(mar);
        ir = mbr;
        ir_i = ir & 0xF000;
        ir_a = ir & 0x0FFF;

        pc += (UINT)2;



        //----execution cycle---------------/
        if (ir_i == 0x0000) {
            break;
        }

        else if (ir_i == 0x1000) //LDA
        {
            mar = ir_a;
            mbr = readWord(mar);
            acc = mbr;
            if (acc > 0x8000) psw = 0x1000;
            else if (acc == 0x0000) psw = 0x0001;
            else psw = 0x0000;
        }

        else if (ir_i == 0x2000) //STA
        {
            mar = ir_a;
            writeWord(mar, acc);
        }

        else if (ir_i == 0x3000) //ADD
        {
            mar = ir_a;
            mbr = readWord(mar);
            c_num = accnum2cint(acc) + accnum2cint(mbr);
            acc = cint2accnum(c_num);
            if (acc > 0x8000) psw = 0x1000;
            else if (acc == 0x0000) psw = 0x0001;
            else psw = 0x0000;
        }

        else if (ir_i == 0x4000) //SUB
        {
            mar = ir_a;
            mbr = readWord(mar);
            c_num = accnum2cint(acc) - accnum2cint(mbr);
           //printf("SuB result: %d\n",c_num);
            acc = cint2accnum(c_num);
            if (acc > 0x8000) psw = 0x1000;
            else if (acc == 0x0000) psw = 0x0001;
            else psw = 0x0000;
        }

        else if (ir_i == 0x5000) //JMP
        {
            mar = ir_a;
            pc = mar;
        }
        else if(ir_i == 0x6000)//CAL
        {
            mar = ir_a;
            push(pc);
            pc = ir_a;
        }
        else if (ir_i == 0x7000)//MUL
        {
            mar = ir_a;
            mbr = readWord(mar);
            c_num = accnum2cint(acc) * accnum2cint(mbr);
            acc = cint2accnum(c_num);
            if (acc > 0x8000) psw = 0x1000;
            else if (acc == 0x0000) psw = 0x0001;
            else psw = 0x0000;

        }
        else if(ir_i == 0x9000){
          // printf("psw:%04x\n",psw);
            if((psw & 0x0FFF) == 0x0001)
            {
                mar = ir_a;
                pc = mar;
            }
            else{
                continue;
            }
        }
        else if (ir_i == 0xa000) //BRN
        {
            if ((psw & 0xF000) == 0x1000) //????????? ?????? ?????????????????? ??????????????????.
            {
                mar = ir_a;
                pc = mar;
            }

            else {
                continue;
            }
        }

        else if (ir_i == 0xb000) {
            mar = ir_a;
            prt(mar);
        }

        else if (ir_i == 0xc000) {
            mar = ir_a;
            prc(mar);
        }

        else if (ir_i == 0xd000) {
            mar = ir_a;
            prs(mar);
        }

        else if (ir_i == 0x8000) {
            if(ir_a == 0x0005)
            {
                pc = pop();
                // printf("!!!!!! %04x\n",pc);
                continue;
            }
            else if (ir_a == 0x0002)
            {
                acc = cint2accnum(accnum2cint(acc) + 1);
            }
            else ST_RUN = 1;
        }
    }
    return 0;
}



//========================================

// Main Function

//========================================

int main() {

    int exit_code; // 0: normal exit, 1: error exit

    UINT start_addr; // start address of program



    printf("========================================\n");

    printf(" AccCom: Accumulator Computer Simulator\n");

    printf(" modified by 201602955 Jang Hyeonjun\n");

    printf("========================================\n");



    printf("*** Load ***\n");

    start_addr = loadProgram();



    printf("*** Input ***\n");

    inputData();



    printf("*** Run ***\n");

    exit_code = runProgram(start_addr);



    printf("\n*** Exit %d ***\n", exit_code);

}
