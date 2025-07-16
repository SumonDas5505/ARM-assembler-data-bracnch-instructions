#include <iostream>
#include <sstream>
#include <bitset>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

unordered_map<string, int> regMap = {
    {"r0", 0},{"r1", 1},{"r2", 2},{"r3", 3},{"r4", 4},{"r5", 5},{"r6", 6},{"r7", 7},
    {"r8", 8},{"r9", 9},{"r10", 10},{"r11", 11},{"r12", 12},{"r13", 13},{"r14", 14},{"r15", 15},
    {"a1", 0},{"a2", 1},{"a3", 2},{"a4", 3},{"v1", 4},{"v2", 5},{"v3", 6},{"v4", 7},{"v5", 8},
    {"v6", 9},{"v7", 10},{"v8", 11},{"ip", 12},{"sp", 13},{"lr", 14},{"pc", 15}
};

unordered_map<string, string> opcodeMap = {
    {"AND", "0000"}, {"EOR", "0001"}, {"SUB", "0010"}, {"RSB", "0011"},
    {"ADD", "0100"}, {"ADC", "0101"}, {"SBC", "0110"}, {"RSC", "0111"},
    {"TST", "1000"}, {"TEQ", "1001"}, {"CMP", "1010"}, {"CMN", "1011"},
    {"ORR", "1100"}, {"MOV", "1101"}, {"BIC", "1110"}, {"MVN", "1111"}
};

unordered_map<string, string> condMap = {
    {"EQ","0000"},{"NE","0001"},{"CS","0010"},{"HS","0010"},{"CC","0011"},{"LO","0011"},
    {"MI","0100"},{"PL","0101"},{"VS","0110"},{"VC","0111"},{"HI","1000"},{"LS","1001"},
    {"GE","1010"},{"LT","1011"},{"GT","1100"},{"LE","1101"},{"AL","1110"}
};

unordered_map<string, string> shiftTypeMap = {
    {"LSL", "00"}, {"LSR", "01"}, {"ASR", "10"}, {"ROR", "11"}
};


// Data Processing Function

string encodeOperand2(const string& op2, bool isImmediate) {
    if (isImmediate) {
        int immVal = stoi(op2.substr(op2.find("#") + 1));
        bitset<8> immBits(immVal);
        bitset<4> rotate(0); // Default rotate = 0
        return rotate.to_string() + immBits.to_string(); // 12 bits
    }

    size_t comma = op2.find(',');
    if (comma == string::npos) {
        bitset<8> shiftBits(0);
        bitset<4> Rm(regMap[op2]);
        return shiftBits.to_string() + Rm.to_string(); // 12 bits
    }

    string rmStr = op2.substr(0, comma);
    string shiftPart = op2.substr(comma + 1);
    rmStr.erase(remove(rmStr.begin(), rmStr.end(), ' '), rmStr.end());

    if (shiftPart.find("#") != string::npos) {
        // Example: LSL #3
        shiftPart.erase(remove(shiftPart.begin(), shiftPart.end(), ' '), shiftPart.end());
        string shiftType = shiftPart.substr(0, shiftPart.find('#'));
        int shiftAmt = stoi(shiftPart.substr(shiftPart.find('#') + 1));

        bitset<5> shiftAmtBits(shiftAmt);                   // Bits 11–7
        string typeBits = shiftTypeMap[shiftType];          // Bits 6–5
        string reserved = "00";                             // Bits 4–3
        bitset<4> rmBits(regMap[rmStr]);                    // Bits 3–0

        string operand2 = shiftAmtBits.to_string() + typeBits + reserved + rmBits.to_string();

        if (operand2.length() != 12) {
            cerr << "Error: operand2 is not 12 bits\n";
        }
        return operand2;
    }

    // Shift by register
    string shiftType = shiftPart.substr(0, shiftPart.find(' '));
    string rsStr = shiftPart.substr(shiftPart.find(' ') + 1);
    rsStr.erase(remove(rsStr.begin(), rsStr.end(), ' '), rsStr.end());

    string typeBits = shiftTypeMap[shiftType];
    bitset<4> Rs(regMap[rsStr]);
    bitset<4> Rm(regMap[rmStr]);

    return Rs.to_string() + "1" + typeBits + Rm.to_string(); // 12 bits
}

// Main assembler function
string assemble(string line) {
    line.erase(remove(line.begin(), line.end(), ','), line.end());
    stringstream ss(line);
    string opcodeCondStr, rdStr, rnStr, op2Str;
    ss >> opcodeCondStr >> rdStr >> rnStr >> op2Str;

    transform(opcodeCondStr.begin(), opcodeCondStr.end(), opcodeCondStr.begin(), ::toupper);
    transform(rdStr.begin(), rdStr.end(), rdStr.begin(), ::tolower);
    transform(rnStr.begin(), rnStr.end(), rnStr.begin(), ::tolower);
    transform(op2Str.begin(), op2Str.end(), op2Str.begin(), ::tolower);

    string condBits = "1110";
    string opcodeStr = opcodeCondStr;
    bool setS = false;

// Extract condition bits and opcode
for (auto it = condMap.begin(); it != condMap.end(); ++it) {
    if (opcodeStr.find(it->first) != string::npos) {
        condBits = it->second;
        opcodeStr.erase(opcodeStr.find(it->first), it->first.length());
        break;
    }
}

    // Detect 'S' in opcode
    if (opcodeStr.size() > 0 && opcodeStr.back() == 'S') {
        setS = true;
        opcodeStr.pop_back();
    }

    string opcodeBits = opcodeMap[opcodeStr];
    bitset<4> Rd(regMap[rdStr]);
    bitset<4> Rn(regMap[rnStr]);

    bool isImmediate = op2Str.find('#') != string::npos;
    string operand2Bits = encodeOperand2(op2Str, isImmediate);

    string I = isImmediate ? "1" : "0";
    string S = setS ? "1" : "0";
    string fixed = "00";

    string binaryStr = condBits + fixed + I + opcodeBits + S + Rn.to_string() + Rd.to_string() + operand2Bits;
    bitset<32> finalBinary(binaryStr);

    stringstream out;
    out << "\nBinary: " << finalBinary << "\n Hex: 0x" << hex << uppercase << finalBinary.to_ulong();
    cout << "Binary string length: " << binaryStr.length() << endl;

    return out.str();
}



//Brach assembler function

string assembleBranch(string line) {
    stringstream ss(line);
    vector<string> tokens;
    string token;
    
    // Parse all tokens
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    string instruction, target;
    bool isLabelFormat = false;
    
    if (tokens.size() == 2) {
        // Format: "INSTRUCTION target" (e.g., "BEQ -6", "BL 100")
        instruction = tokens[0];
        target = tokens[1];
    } else if (tokens.size() == 3) {
        // Format: "currentLabel INSTRUCTION targetLabel" (e.g., "here BEQ here")
        instruction = tokens[1];
        target = tokens[2];
        isLabelFormat = true;
        // For label format, check if it's same label (branch to self)
        if (tokens[0] == tokens[2]) {
            target = "-2"; 
        }
    } else {
        // Invalid format
        return "Error: Invalid instruction format";
    }
    
    transform(instruction.begin(), instruction.end(), instruction.begin(), ::toupper);
    
    // Default condition is AL (always)
    string condBits = "1110";
    string opcodeStr = instruction;
    bool isLinkBranch = false;
    
    if (opcodeStr.find("BL") == 0) {
        isLinkBranch = true;
        opcodeStr = opcodeStr.substr(2); 
    } else if (opcodeStr.find("B") == 0) {
        opcodeStr = opcodeStr.substr(1); 
    }
    
    for (auto it = condMap.begin(); it != condMap.end(); ++it) {
        if (opcodeStr == it->first) {
            condBits = it->second;
            break;
        }
    }
    
    //offset
    int offset = 0;
    if (target.find("0x") == 0 || target.find("0X") == 0) {
        // Hex offset
        offset = stoi(target, nullptr, 16);
    } else if (isdigit(target[0]) || target[0] == '-') {
        // Decimal offset
        offset = stoi(target);
    } else {
        offset = -2;
    }
    
    
    //24-bit signed offset (sign extend if negative)
    bitset<24> offsetBits;
    if (offset < 0) {
        // Two's complement for negative numbers
        offsetBits = bitset<24>(offset & 0xFFFFFF);
    } else {
        offsetBits = bitset<24>(offset);
    }
    
    // Build instruction components
    string fixed = "101";           // Bits 27-25 (always 101 for branch)
    string L = isLinkBranch ? "1" : "0";  // Bit 24 (1 for BL, 0 for B)
    
    // Construct the 32-bit instruction
    string binaryStr = condBits + fixed + L + offsetBits.to_string();
    bitset<32> finalBinary(binaryStr);
    
    // Output formatting
    stringstream out;
    out << "\nInstruction: " << line << endl;
    out << "Condition: " << condBits << " (" << (condBits == "1110" ? "AL" : "conditional") << ")" << endl;
    out << "Link bit: " << L << " (" << (isLinkBranch ? "BL" : "B") << ")" << endl;
    out << "Offset: " << offset << " (24-bit: " << offsetBits << ")" << endl;
    out << "Binary: " << finalBinary << endl;
    out << "Hex: 0x" << hex << uppercase << finalBinary.to_ulong() << endl;
    
    return out.str();
}


int main() {
    cout<<"\nFor Data Processing Instruction, Enter: 1\n\nFor Branch instruction, Enter: 2 \n\n";
    int choice;
    cin >> choice;
    if (choice == 1) {
        cin.ignore(); 
            string input;
            cout << "\nEnter ARM Instruction (e.g., SUBEQ a1, v7, r2, LSL #3): ";
            getline(cin, input);

            cout << assemble(input) << endl;
    } else if (choice == 2){
        cin.ignore();
        string input;
        cout << "\nEnter ARM branch instruction (e.g., BEQ -6, BL 100, B here): ";
        getline(cin, input);
        
        cout << assembleBranch(input) << endl;
    }else{
        cout << "\nInvalid choice. Please enter 1 or 2." << endl;
    }
    return 0;
}
