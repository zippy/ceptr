#define OPERANDS_SIZE (sizeof(Xaddr) * 2)

enum Opcodes {
    RETURN, PUSH_IMMEDIATE
};

typedef int Opcode;

typedef struct {
    Opcode opcode;
    char operands[OPERANDS_SIZE];
} Instruction;


//    testRun();
