#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <memory>

using namespace std;

class Register {
private:
    string hexValue;  // Store as hexadecimal string

public:
    Register() : hexValue("00") {}
    void load_value(const string &val) { hexValue = val; }
    string get_value() { return hexValue; }
    int get_int_value() { return stoi(hexValue, nullptr, 16); } // Convert to int for calculations
};

class Memory {
private:
    vector<string> cells;

public:
    Memory() : cells(256, "00") {}
    string read(int address) { return (address >= 0 && address < cells.size()) ? cells[address] : "00"; }
    void write(int address, const string &value) { if (address >= 0 && address < cells.size()) cells[address] = value; }

    void display() {
        for (int i = 0; i < cells.size(); ++i) {
            if (cells[i] != "00") // Only print non-empty memory cells
                cout << "Memory[" << setw(3) << setfill('0') << i << "] = " << cells[i] << endl;
        }
    }
};

class Instruction {
public:
    virtual void execute(Register *registers, Memory &memory, int &pc) = 0;
    virtual ~Instruction() {}
};

class LoadImmediate : public Instruction {
private:
    int regIndex;
    string value;

public:
    LoadImmediate(int reg, const string &val) : regIndex(reg), value(val) {}
    void execute(Register *registers, Memory &memory, int &pc) override {
        registers[regIndex].load_value(value);
        cout << "LOAD R" << regIndex << " immediate value = " << registers[regIndex].get_value() << endl;
    }
};

class LoadFromMemory : public Instruction {
private:
    int regIndex;
    int address;

public:
    LoadFromMemory(int reg, int addr) : regIndex(reg), address(addr) {}
    void execute(Register *registers, Memory &memory, int &pc) override {
        registers[regIndex].load_value(memory.read(address));
        cout << "LOAD R" << regIndex << " from Memory[" << address << "] = " << registers[regIndex].get_value() << endl;
    }
};

class StoreToMemory : public Instruction {
private:
    int regIndex;
    int address;

public:
    StoreToMemory(int reg, int addr) : regIndex(reg), address(addr) {}
    void execute(Register *registers, Memory &memory, int &pc) override {
        memory.write(address, registers[regIndex].get_value());
        cout << "STORE R" << regIndex << " to Memory[" << address << "]" << endl;
    }
};

class Add : public Instruction {
private:
    int regDest;
    int regSrc1;
    int regSrc2;

public:
    Add(int dest, int src1, int src2) : regDest(dest), regSrc1(src1), regSrc2(src2) {}
    void execute(Register *registers, Memory &memory, int &pc) override {
        int sum = registers[regSrc1].get_int_value() + registers[regSrc2].get_int_value();
        stringstream ss;
        ss << uppercase << hex << setw(2) << setfill('0') << sum; // Convert sum to hexadecimal string
        registers[regDest].load_value(ss.str());
        cout << "ADD R" << regSrc1 << " and R" << regSrc2 << " into R" << regDest << " = " << registers[regDest].get_value() << endl;
    }
};

class JumpIfEqual : public Instruction {
private:
    int regIndex;
    int target;

public:
    JumpIfEqual(int reg, int tgt) : regIndex(reg), target(tgt) {}
    void execute(Register *registers, Memory &memory, int &pc) override {
        if (registers[regIndex].get_value() == registers[0].get_value()) {
            pc = target;
            cout << "JUMP to instruction " << pc << " because R" << regIndex << " == R0" << endl;
        }
    }
};

class Halt : public Instruction {
public:
    void execute(Register *registers, Memory &memory, int &pc) override {
        pc = -1; // Halt execution
        cout << "HALT execution." << endl;
    }
};

class Machine {
private:
    vector<Register> registers;
    Memory memory;
    int programCounter;
    vector<unique_ptr<Instruction>> instructionSet;

public:
    Machine() : registers(16), programCounter(0) {}

    void run() {
        while (programCounter >= 0 && programCounter < instructionSet.size()) {
            instructionSet[programCounter]->execute(registers.data(), memory, programCounter);
            display_status();
            programCounter++; // Move to the next instruction
        }
    }

    void display_status() {
        cout << "\nRegisters Status:\n";
        for (int i = 0; i < registers.size(); ++i) {
            cout << "Register[" << i << "] = " << registers[i].get_value() << endl;
        }
        cout << "\nMemory Status:\n";
        memory.display(); // Show only non-empty memory cells
        cout << "Program Counter = " << programCounter << endl;
    }

    void manual_input() {
        string instruction;
        cout << "Enter instructions (4 characters each, or 'C000' to finish): \n";
        while (true) {
            cout << "Instruction: ";
            cin >> instruction;
            if (instruction == "C000") {
                break; // Exit if the user types "C000"
            }
            if (instruction.length() == 4) {
                char opcode = instruction[0];
                int regIndex = instruction[1] - '0'; // Convert char to int for register index
                string hexValue = instruction.substr(2); // Get the last two characters as hex string
                switch (opcode) {
                    case '2': // LOAD immediate
                        instructionSet.push_back(make_unique<LoadImmediate>(regIndex, hexValue));
                        break;
                    case '3': // LOAD from memory
                        instructionSet.push_back(make_unique<LoadFromMemory>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case '4': // STORE
                        instructionSet.push_back(make_unique<StoreToMemory>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case '5': // ADD
                        instructionSet.push_back(make_unique<Add>(regIndex, regIndex, stoi(hexValue, nullptr, 16))); // Example usage
                        break;
                    case 'B': // JUMP if equal
                        instructionSet.push_back(make_unique<JumpIfEqual>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case 'C': // HALT
                        instructionSet.push_back(make_unique<Halt>());
                        break;
                    default:
                        cout << "Invalid opcode: " << opcode << endl;
                        break;
                }
                memory.write(instructionSet.size() - 1, instruction); // Store instruction in memory
                cout << "Instruction '" << instruction << "' added." << endl;
            } else {
                cout << "Invalid instruction length. Instructions must be 4 characters long.\n";
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
                    run();
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
        while (file >> instruction) {
            if (instruction.length() == 4) {
                char opcode = instruction[0];
                int regIndex = instruction[1] - '0';
                string hexValue = instruction.substr(2);
                switch (opcode) {
                    case '2': // LOAD immediate
                        instructionSet.push_back(make_unique<LoadImmediate>(regIndex, hexValue));
                        break;
                    case '3': // LOAD from memory
                        instructionSet.push_back(make_unique<LoadFromMemory>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case '4': // STORE
                        instructionSet.push_back(make_unique<StoreToMemory>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case '5': // ADD
                        instructionSet.push_back(make_unique<Add>(regIndex, regIndex, stoi(hexValue, nullptr, 16))); // Example usage
                        break;
                    case 'B': // JUMP if equal
                        instructionSet.push_back(make_unique<JumpIfEqual>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case 'C': // HALT
                        instructionSet.push_back(make_unique<Halt>());
                        break;
                    default:
                        cout << "Invalid opcode: " << opcode << endl;
                        break;
                }
            }
        }
        cout << "Program loaded from " << filename << endl;
    }
};

int main() {
    Machine machine;
    machine.menu();
    return 0;
}
