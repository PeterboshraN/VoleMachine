#include <bits/stdc++.h>
using namespace std;

class Register {
private:
    string value;

public:
    Register() : value("00") {}
    void load_value(string val) { value = val; }
    string get_value() { return value; }
};

class Memory {
private:
    vector<string> cells;

public:
    Memory() : cells(256, "00") {}
    string read(int address) { return (address >= 0 && address < cells.size()) ? cells[address] : "00"; }
    void write(int address, string value) { if (address >= 0 && address < cells.size()) cells[address] = value; }
    void display() {
        for (int i = 0; i < cells.size(); ++i) {
            if (i % 16 == 0 && i != 0) cout << endl;
            cout << setw(3) << cells[i] << " ";
        }
        cout << "\n";
    }

};

class Instruction {
private:
    char opcode;
    int operand1;
    int operand2;

public:
    Instruction(char op, int op1, int op2 = 0) : opcode(op), operand1(op1), operand2(op2) {}

    void execute(Register *registers, Memory &memory, int &pc) {
        stringstream ss;
        ss << hex <<operand2;
        string str=ss.str();
        transform(str.begin(), str.end(), str.begin(), ::toupper);
        int regIndex = operand1;
        switch (opcode) {
            case '1': // LOAD from memory
                registers[regIndex].load_value(memory.read(operand2));
                cout << "LOAD R" << regIndex << " from Memory[" << operand2 << "] = " << registers[regIndex].get_value() << endl;
                break;
            case '2': // LOAD immediate
                registers[regIndex].load_value(str);
                cout << "LOAD R" << regIndex << " immediate value = " << registers[regIndex].get_value() << endl;
                break;
            case '3': // STORE
                memory.write(operand2, registers[regIndex].get_value());
                cout << "STORE R" << regIndex << " to Memory[" << operand2 << "]" << endl;
                break;
            case '4': // MOVE
                registers[operand2].load_value(registers[regIndex].get_value());
                cout << "MOVE R" << regIndex << " to R" << operand2 << endl;
                break;
            case '5': { // ADD as two’s complement
                int sum = stoi(registers[operand1].get_value()) + stoi(registers[operand2].get_value());
                registers[regIndex].load_value(to_string(sum));
                cout << "ADD R" << operand1 << " and R" << operand2 << " into R" << regIndex << " = " << registers[regIndex].get_value() << endl;
                break;
            }
            case 'B': // JUMP if equal
                if (registers[regIndex].get_value() == registers[0].get_value()) {
                    pc = operand2;
                    cout << "JUMP to instruction " << pc << " because R" << regIndex << " == R0" << endl;
                }
                break;
            case 'C':
                pc = -1;
                cout << "HALT execution." << endl;
                break; // HALT
            default:
                cout << "Invalid opcode: " << opcode << endl;
                break;
        }
    }
};

class Machine {
private:
    vector<Register> registers;
    Memory memory;
    int programCounter;

public:
    Machine() : registers(16), programCounter(0) {}

    void run() {
        while (programCounter >= 0) {
            string part1=memory.read(programCounter++);
            string part2=memory.read(programCounter++);
            string instruction = part1+part2;
            if (instruction == "00") continue;
            char opcode = instruction[0];
            int operand1 = instruction[1] - '0';
            int operand2 = stoi(instruction.substr(2), nullptr, 16);
            Instruction instr(opcode, operand1, operand2);
            instr.execute(registers.data(), memory, programCounter);
            display_status(); // Display status after each instruction execution
        }
    }

    void display_status() {
        cout << "\nRegisters Status:\n";
        for (int i = 0; i < registers.size(); ++i) {
            cout << "Register[" << i << "] = " << registers[i].get_value() << endl;
        }
        cout << "\nMemory Status:\n";
        memory.display();
        cout << "Program Counter = " << programCounter << endl;
    }

    void manual_input() {
        string instruction;
        cout << "Enter instructions (4 characters each, or 'done' to finish): \n";
        while (true) {
            cout << "Instruction: ";
            cin >> instruction;
            if (instruction == "done") {
                break; // Exit if the user types "done"
            }
            int pos = 0;
            while (pos < instruction.length()) {
                string chunk = instruction.substr(pos, 2);  // قسم النص إلى أجزاء كل جزء به حرفين
                if (chunk.length() == 1) chunk += "0";  // إذا كان الجزء الأخير حرف واحد فقط، أضف صفر
                memory.write(programCounter++, chunk);  // احفظ كل جزء في خلية منفصلة
                pos += 2;
            }
        }
        programCounter = 0; // Reset program counter for execution
    }

    void menu() {
        int choice;
        do {
            cout << "\n1. Load Program\n2. Run\n3. Display Status\n4. Enter Instructions Manually\n5. Exit\nChoice: ";
            cin >> choice;
            switch (choice) {
                case 1: {
                    string filename;
                    cout << "Enter program file path: ";
                    cin >> filename;
                    load_program(filename);
                    break;
                }
                case 2: run(); break;
                case 3: display_status(); break;
                case 4: {
                    manual_input();
                    break;
                }
                case 5: cout << "Exiting...\n"; break;
                default: cout << "Invalid choice.\n"; break;
            }
        } while (choice != 5);
    }

    void load_program(const string &filename) {
        ifstream file(filename);
        string instruction;
        int index = 0;
        while (file >> instruction && index < 256) {
            memory.write(index++, instruction);
        }
    }
};

int main() {
    Machine machine;
    machine.menu();
    return 0;
}