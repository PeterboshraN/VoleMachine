#include <bits/stdc++.h>
using namespace std;

class Register {
private:
    string hexValue;

public:
    Register() : hexValue("00") {}
    void load_value(const string &val) { hexValue = val; }
    string get_value() { return hexValue; }
    int get_int_value() { return stoi(hexValue, nullptr, 16); }
};

class Memory {
private:
    vector<string> cells;
    int programCounter;

public:
    Memory() : cells(256, "00"), programCounter(0) {}

    string read(int address) {
        return (address >= 0 && address < cells.size()) ? cells[address] : "00";
    }

    void write(int addr, const string &val) {
        if (addr >= 0 && addr + 1 < cells.size() && val.length() == 4) {
            // Split instruction into two parts and store them in adjacent memory cells
            string part1 = val.substr(0, 2);  // First two digits
            string part2 = val.substr(2, 2);  // Last two digits
            cells[addr] = part1;
            cells[addr + 1] = part2;
        }
    }

    void executeInstruction() {
        while (programCounter < cells.size()) {
            string instruction = cells[programCounter] + cells[programCounter + 1];

            if (instruction == "C000") {
                cout << " HALT instruction encountered. Stopping execution." << endl;
                break;
            }
            // More instruction handling...

            programCounter += 2;  // Move to next instruction
        }
    }

    void display() {
        cout << "\nMemory Display:\n";
        cout << "      "; // Offset for row header
        for (int j = 0; j < 16; j++) {
            cout << " " << hex << uppercase << j << " ";
        }
        cout << "\n     ------------------------------------------------\n";

        for (int i = 0; i < cells.size(); i++) {
            if (i % 16 == 0) {
                cout << setw(2) << setfill('0') << hex << uppercase << (i / 16) << " | ";
            }

            // Print current memory cell value
            cout << setw(2) << hex << uppercase << cells[i] << " ";

            // If it's the 16th cell, move to the next line
            if ((i + 1) % 16 == 0) {
                cout << endl;
            }
        }
        cout << "\n";
    }
};

class Instruction {
public:
    virtual void execute(Register *registers, Memory &memory, int &pc) = 0;
};

class LoadImmediate : public Instruction {
private:
    int regIndex;
    string value;

public:
    LoadImmediate(int reg, const string &val) : regIndex(reg), value(val) {}
    void execute(Register *registers, Memory &memory, int &pc) {
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
        int value1 = static_cast<int8_t>(registers[regSrc1].get_int_value());
        int value2 = static_cast<int8_t>(registers[regSrc2].get_int_value());
        int sum = value1 + value2;
        int result = static_cast<int8_t>(sum);
        stringstream ss;
        ss << uppercase << hex << setw(2) << setfill('0') << (result & 0xFF);
        registers[regDest].load_value(ss.str());
        cout << "ADD R" << regSrc1 << " and R" << regSrc2 << " into R" << regDest << " = " << registers[regDest].get_value() << endl;
    }
};
class AddFloat : public Instruction {
private:
    int regDest;  // Destination register
    int regSrc1;  // First source register
    int regSrc2;  // Second source register
    const int bias = 4;

public:
    // Constructor
    AddFloat(int dest, int src1, int src2) : regDest(dest), regSrc1(src1), regSrc2(src2) {}

    void execute(Register *registers, Memory &memory, int &pc) override {
        // Interpret register values as 8-bit floating-point representations
        uint8_t val1 = registers[regSrc1].get_int_value();
        uint8_t val2 = registers[regSrc2].get_int_value();

        // Extract sign, exponent, and mantissa for val1
        int sign1 = (val1 >> 7) & 0x1;
        int exponent1 = ((val1 >> 4) & 0x7) - bias; // Apply bias to exponent
        int mantissa1 = val1 & 0xF;

        // Extract sign, exponent, and mantissa for val2
        int sign2 = (val2 >> 7) & 0x1;
        int exponent2 = ((val2 >> 4) & 0x7) - bias; // Apply bias to exponent
        int mantissa2 = val2 & 0xF;

        // Convert to normalized floating-point values
        double float1 = pow(-1, sign1) * (1 + mantissa1 / 16.0) * pow(2, exponent1);
        double float2 = pow(-1, sign2) * (1 + mantissa2 / 16.0) * pow(2, exponent2);

        // Perform floating-point addition
        double resultFloat = float1 + float2;

        // Determine sign of result
        int resultSign = resultFloat < 0 ? 1 : 0;
        resultFloat = abs(resultFloat);

        // Normalize result exponent and mantissa
        int resultExponent = 0;
        int resultMantissa = 0;

        if (resultFloat != 0) {
            resultExponent = static_cast<int>(log2(resultFloat));
            resultMantissa = static_cast<int>((resultFloat / pow(2, resultExponent)) * 16) & 0xF;

            // Apply the bias to the exponent
            resultExponent += bias;

            // Ensure exponent fits within 3 bits and mantissa within 4 bits
            if (resultExponent > 7) {
                resultExponent = 7;
                resultMantissa = 0xF;  // Set mantissa to max if exponent overflow
            } else if (resultExponent < 0) {
                resultExponent = 0;
                resultMantissa = 0;  // Set to zero if exponent underflow
            }
        }

        // Pack into 8-bit floating-point format
        uint8_t result = (resultSign << 7) | ((resultExponent & 0x7) << 4) | (resultMantissa & 0xF);

        // Store result in destination register
        stringstream ss;
        ss << uppercase << hex << setw(2) << setfill('0') << (result & 0xFF);
        registers[regDest].load_value(ss.str());

        // Display the result of the operation for debugging purposes
        cout << "ADD_FLOAT R" << regSrc1 << " and R" << regSrc2 << " into R" << regDest
             << " = " << registers[regDest].get_value() << endl;
    }
};

class StoreToFixedMemory : public Instruction {
private:
    int regIndex;

public:
    StoreToFixedMemory(int reg) : regIndex(reg) {}

    void execute(Register *registers, Memory &memory, int &pc) override {
        string hexValue = registers[regIndex].get_value();

        try {
            int intValue = stoi(hexValue, nullptr, 16);
            if (intValue >= 0 && intValue <= 127) { // Ensure it’s a valid ASCII range
                char asciiChar = static_cast<char>(intValue);

                // Retrieve current content in memory[0] and append the new ASCII character
                string currentString = memory.read(0);
                currentString += asciiChar;

                // Store the updated string back to memory[0]
                memory.write(0, currentString);

                cout << "STORE R" << regIndex << " to Memory[00] as ASCII '" << asciiChar << "'; updated Memory[00] = \"" << currentString << "\"" << endl;
            } else {
                cout << "Value in R" << regIndex << " is out of ASCII range for Memory[00]." << endl;
            }
        } catch (const invalid_argument& e) {
            cout << "Error: Invalid hex value in R" << regIndex << ": " << hexValue << endl;
        } catch (const out_of_range& e) {
            cout << "Error: Hex value in R" << regIndex << " is out of range: " << e.what() << endl;
        }
    }
};


class JumpIfEqual : public Instruction {
private:
    int regIndex; // Register to compare with R0
    int address;  // Address to jump to

public:
    JumpIfEqual(int reg, int addr) : regIndex(reg), address(addr) {}

    void execute(Register *registers, Memory &memory, int &pc) override {
        // Compare the contents of the specified register with R0
        if (registers[regIndex].get_value() == registers[0].get_value()) {
            // Set the program counter to the target address
            pc = address; // Set PC to the target address directly
            cout << "JUMP to instruction at memory address [" << pc << "]" << endl;
        } else {
            cout << "No JUMP: R" << regIndex << " (" << registers[regIndex].get_value()
                 << ") != R0 (" << registers[0].get_value() << ")" << endl;
        }
    }
};


class CopyRegister : public Instruction {
private:
    int sourceReg;
    int destReg;

public:
    CopyRegister(int srcReg, int destReg) : sourceReg(srcReg), destReg(destReg) {}

    void execute(Register *registers, Memory &memory, int &pc) override {
        registers[destReg].load_value(registers[sourceReg].get_value());
        cout << "COPY from R" << sourceReg << " to R" << destReg << " = " << registers[destReg].get_value() << endl;
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
            int oldPC = programCounter;  // Save the current program counter
            instructionSet[programCounter]->execute(registers.data(), memory, programCounter);

            display_status(); // Show register and memory status after each instruction

            // Only increment the program counter if it wasn't modified by the instruction
            if (programCounter == oldPC) {
                programCounter++;
            }

            // If the program counter is set to -1 (by a Halt instruction), stop the program.
            if (programCounter == -1) {
                break;
            }
        }
    }

    void display_status() {
        cout << "\nRegisters Status:\n";
        for (int i = 0; i < registers.size(); ++i) {
            cout << "Register[" << i << "] = " << registers[i].get_value() << endl;
        }

        cout << "\nMemory Status:\n";
        memory.display(); // Show only non-empty memory cells

        // Check if memory[0] is not empty and has a valid value for display
        string hexValue = memory.read(0);
        if (!hexValue.empty() && hexValue != "00") { // Ensure non-empty and non-default
            try {
                // Convert hex value "20" directly to a space character
                if (hexValue == "20") {
                    cout << "Expected value: <space>" << endl; // Explicitly display a space character
                } else { // Handle other valid hex values
                    int intValue = stoi(hexValue, nullptr, 16); // Convert hex string to integer
                    if (intValue >= 0 && intValue <= 127) { // Printable ASCII range check
                        char asciiChar = static_cast<char>(intValue);
                        cout << "Expected value: " << asciiChar << endl;
                    } else {
                        cout << "Expected value: Non-printable ASCII character." << endl;
                    }
                }
            } catch (const invalid_argument& e) {
                cout << "Error: Invalid hex value in Memory[00]: " << hexValue << endl;
            } catch (const out_of_range& e) {
                cout << "Error: Hex value in Memory[00] is out of range: " << e.what() << endl;
            }
        } else {
            cout << "Memory[00] is empty or contains default value '00'." << endl;
        }

        cout << "Program Counter = " << programCounter << endl;
    }


    void manual_input() {
        int startAddress;
        cout << "Enter the starting memory address to store instructions: ";
        cin >> startAddress;

        string instruction;
        cout << "Enter instructions (4 characters each, or 'C000' to finish): \n";
        int address = startAddress;
        while (true) {
            cout << "Instruction: ";
            cin >> instruction;
            if (instruction.length() == 4) {
                char opcode = instruction[0];
                int regIndex = instruction[1] - '0'; // Convert char to int for register index
                string hexValue = instruction.substr(2); // Get the last two characters as hex string
                switch (opcode) {
                    case '1':{

                        int memAddress = stoi(hexValue, nullptr, 16); // Convert the last two characters to an integer address
                        instructionSet.push_back(make_unique<LoadFromMemory>(regIndex, memAddress));
                        break;
                    }
                    case '2': // LOAD immediate
                        instructionSet.push_back(make_unique<LoadImmediate>(regIndex, hexValue));
                        break;
                    case '3': // LOAD from memory
                        instructionSet.push_back(make_unique<StoreToMemory>(regIndex, stoi(hexValue, nullptr, 16))); // Always store to memory
                        break;

                    case '4': // STORE or COPY
                        if (instruction[1] == '0') { // Detecting the 40 prefix for copy
                            int destReg = instruction[3] - '0'; // Get the destination register index
                            instructionSet.push_back(make_unique<CopyRegister>(10, destReg)); // Assuming 'A' corresponds to index 10
                        }
                        break;
                    case '5': { // ADD
                        int regDest = regIndex;
                        int regSrc1 = instruction[2] - '0';
                        int regSrc2 = instruction[3] - '0';
                        instructionSet.push_back(make_unique<Add>(regDest, regSrc1, regSrc2));
                        break;
                    }
                    case '6': { // Floating-point ADD
                        int regDest = regIndex;
                        int regSrc1 = instruction[2] - '0';
                        int regSrc2 = instruction[3] - '0';
                        instructionSet.push_back(make_unique<AddFloat>(regDest, regSrc1, regSrc2));
                        break;
                    }
                    case 'B': {

                        int memAddress = stoi(hexValue); // Convert the last two characters to an integer address
                        instructionSet.push_back(make_unique<JumpIfEqual>(regIndex,memAddress));
                        break;
                    }// JUMP if equal
                    case 'C': // HALT
                        instructionSet.push_back(make_unique<Halt>());
                        memory.write(address, instruction); // Store HALT in memory
                        cout << "HALT instruction added at Memory[" << address << "]. Stopping instruction input.\n";
                        programCounter = 0; // Reset program counter for execution
                        return; // Stop taking further instructions
                    default:
                        cout << "Invalid opcode: " << opcode << endl;
                        break;
                }
                memory.write(address, instruction); // Store instruction in memory at specified address
                cout << "Instruction '" << instruction << "' added at Memory[" << address << "]." << endl;
                address++;
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
        if (!file.is_open()) {
            cout << "Error: Unable to open file. Please check the file path and try again.\n";
            return;
        }

        cout << "File loaded successfully.\n";
        string instruction;
        int startAddress;
        cout << "Enter the starting memory address to store instructions: ";
        cin >> startAddress;
        int address = startAddress;

        while (file >> instruction) {
            if (instruction.length() == 4) {
                char opcode = instruction[0];
                int regIndex = instruction[1] - '0';
                string hexValue = instruction.substr(2);

                switch (opcode) {
                    case '1': {
                        int memAddress = stoi(hexValue, nullptr, 16); // Convert last two characters to integer address
                        instructionSet.push_back(make_unique<LoadFromMemory>(regIndex, memAddress));
                        break;
                    }
                    case '2': // LOAD immediate
                        instructionSet.push_back(make_unique<LoadImmediate>(regIndex, hexValue));
                        break;
                    case '3': // STORE to memory
                        instructionSet.push_back(make_unique<StoreToMemory>(regIndex, stoi(hexValue, nullptr, 16)));
                        break;
                    case '4': { // STORE or COPY
                        if (hexValue[0] == '0') { // Detect the 40 prefix for copy
                            int destReg = hexValue[1] - '0'; // Get destination register index
                            instructionSet.push_back(make_unique<CopyRegister>(10, destReg)); // Assuming 'A' corresponds to index 10
                        }
                        break;
                    }
                    case '5': { // ADD
                        int regDest = regIndex;
                        int regSrc1 = instruction[2] - '0';
                        int regSrc2 = instruction[3] - '0';
                        instructionSet.push_back(make_unique<Add>(regDest, regSrc1, regSrc2));
                        break;
                    }
                    case '6': { // Floating-point ADD
                        int regDest = regIndex;
                        int regSrc1 = instruction[2] - '0';
                        int regSrc2 = instruction[3] - '0';
                        instructionSet.push_back(make_unique<AddFloat>(regDest, regSrc1, regSrc2));
                        break;
                    }
                    case 'B': { // JUMP if equal
                        int memAddress = stoi(hexValue);
                        instructionSet.push_back(make_unique<JumpIfEqual>(regIndex, memAddress));
                        break;
                    }
                    case 'C': // HALT
                        instructionSet.push_back(make_unique<Halt>());
                        memory.write(address, instruction); // Store HALT in memory
                        cout << "HALT instruction found. Stopping program loading at Memory[" << address << "].\n";
                        programCounter = 0; // Reset program counter
                        file.close(); // Close the file after processing
                        return; // Stop reading further instructions
                    default:
                        cout << "Invalid opcode in file: " << opcode << endl;
                        break;
                }

                memory.write(address, instruction); // Store instruction in memory at specified address
                address++;
            } else {
                cout << "Skipping invalid instruction in file: " << instruction << endl;
            }
        }
        programCounter = 0; // Reset program counter
    }
};
int main() {
    Machine machine;
    machine.menu();
    return 0;
}