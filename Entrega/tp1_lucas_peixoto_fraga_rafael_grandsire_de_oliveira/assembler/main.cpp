#include <iostream>
#include <sstream>

#include "instructions.h"

void begin(ofstream &fout){
	fout << "DEPTH = 256;" << endl
		 << "WIDTH = 8;" << endl
		 << "ADDRESS_RADIX = HEX;" << endl
		 << "DATA_RADIX = BIN;" << endl
		 << "CONTENT" << endl
		 << "BEGIN" << endl << endl;
}

void end(ofstream &fout){
	fout << "END;";
}

int main(int argc, char *argv[]){
	char RAM[256] = {0};
	int pc=0;
	int opTypeMap[28] = {0, 1, 1, 2, 2, 2, 2, 4, 1, 1, 2, 2, 2, 1, 1, 4,
							   			3, 4, 1, 1, 0, 4, 4, 1, 3, 3, 1, 5};
	map<string, int> labelMap, opCodeMap, regMap;
	map<string, mem> memMap;
	mem IO = {255, 2, 0};
	memMap["IO"] = IO;

	fillOpCodeMap(opCodeMap);
	fillRegMap(regMap);

	string s, saux;
	stringstream ss, ssaux;
	ifstream fin(argv[1]);
	for (; getline(fin, saux); ss.clear(), ssaux.clear()){
		ssaux.str(saux);
		getline(ssaux, s, ';');
		ss.str(s);
		string label;
		ss >> label;
		if (label.size() > 0)
			if (label[label.size() - 1] == ':'){
				label.erase(label.size() - 1, 1);
				string op;
				ss >> op;
				if (op == ".data"){
					int numBytes;
                    long long int value;
					ss >> numBytes >> value;
                    mem mem_ = {pc + 1, numBytes, value};
					memMap[label] = mem_;
					pc += numBytes;
				}
				else{
					labelMap[label] = pc + 1;
					pc += 2;
				}
			}
			else pc += 2;
	}
	saux.clear();

	fin.clear();
	fin.seekg(0, fin.beg);

	ofstream fout(argv[2]);
	for (begin(fout), pc = 0; getline(fin, saux); ss.clear(), ssaux.clear()){
		ssaux.str(saux);
		getline(ssaux, s, ';');	
		ss.str(s);
		string op;
		ss >> op;
		if (op.size() > 0){
			if (op[op.size() - 1] == ':')
				ss >> op;		
			switch (opTypeMap[opCodeMap[op]]){
				case 0:{
					newInstruction(pc, (opCodeMap[op] << 3), RAM);
					newInstruction(pc, 0, RAM);
				} break;
				case 1:{
					string op1, op2;
					ss >> op1 >> op2;
					newInstruction(pc, (opCodeMap[op] << 3) + regMap[op1], RAM);
					if (memMap[op2].pc != 0)
						newInstruction(pc, memMap[op2].pc - 1, RAM);
					else if (labelMap[op2] != 0)
						newInstruction(pc, labelMap[op2] - 1, RAM);
					else{
						stringstream ss_(op2);
						int n = 0;
						ss_ >> n;
						newInstruction(pc, n, RAM);
					}
				} break;
				case 2:{
					string op1, op2;
					ss >> op1 >> op2;
					newInstruction(pc, (opCodeMap[op] << 3) + regMap[op1], RAM);
					newInstruction(pc, (regMap[op2] << 5), RAM);
				} break;
				case 3:{
					string op1, op2, op3;
					ss >> op1 >> op2 >> op3;
					newInstruction(pc, (opCodeMap[op] << 3) + regMap[op1], RAM);
					newInstruction(pc, (regMap[op2] << 5) + (regMap[op3] << 2), RAM);
				} break;
				case 4:{
					string op1;
					ss >> op1;
					newInstruction(pc, (opCodeMap[op] << 3), RAM);
					if (memMap[op1].pc != 0)
						newInstruction(pc, memMap[op1].pc - 1, RAM);
					else if (labelMap[op1] != 0)
						newInstruction(pc, labelMap[op1] - 1, RAM);
					else{
						stringstream ss_(op1);
						int n;
						ss_ >> n;
						newInstruction(pc, n, RAM);
					}
				} break;
				case 5:{
					int numBytes;
                    long long int value;
					ss >> numBytes >> value;
					while(numBytes--)
						newInstruction(pc, value >> (8 * numBytes), RAM);
				}
			}
		}
	}
	printRAM(pc, RAM, fout, memMap);
	saux.clear();
	end(fout);
	fin.clear();
	fin.close();
	fout.clear();
	fout.close();
	return 0;
}
