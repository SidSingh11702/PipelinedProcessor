/*******************

* Authors : Akshat Meena and Siddharth Singh

* Purpose : Pipelined Processor

*******************/

#include <bits/stdc++.h>
using namespace std;

/* Definition of class Program begins */

class Program {

private:
	int* RF;				// Register file
	string* ICache;			// Instruction cache
	string* DCache;			// Data cache
	int PC;					// Program Counter
	string IR;				// Instruction register
	char opcode;			// opcode
	int operand[3];			// operands
	int LMD;				// memory destination		
	int ALUOutput;			// output of execute stage
	queue<char> opq;		// queue for opcode buffer
	queue<int> R0q;			// queue for source register buffer
	queue<int> outputq;		// queue for buffer of output of EX stage
	bool RFport[16];		// bool to keep track of open ports in register file
	bool stage[5];			// bool to keep track of stage to be executed

public:
	int AI,LI,DI,CI,HI;							// Counts of various instructions
	int cycle;									// No of cycles
	int DS, CS;									// No. of Data stalls and Control stalls resp
	Program();									// Constructor
	~Program();									// Destructor
	void RegisterFile(string filepath);			// Read and store the values of Registers from given file
	void InputICache(string filepath);			// Read and store the values of ICache from given file
	void InputDCache(string filepath);			// Read and store the values of DCache from given file
	void IFetch();								// Feteches the instruction from ICache		
	void IDecode();								// Decodes the fetched instruction
	void Execution();							// Exectues the instruction
	void MemoryAccess();						// Performs the memory access
	void WriteBack();							// Writes back the data onto the register
	void Pipeline();							// Handles pipelining
	void Data();								// Handles Data stalls
	void Branch();								// Handles Control stalls
	void Halt();								// Controls the Halt instructions
	void Output();								// Output the Dcache and Stats of program in resp. files
	int HexToDec(string x);						// Converts hexadecimal number to its decimal form
	string DecToHex(int x);						// Converts decimal number to its hexadecimal form
};

/* Definition of class Program ends */

// Constructor
Program::Program(){

	PC = AI = LI = DI = CI = HI = cycle = DS = CS = 0;			// initialising the variables to 0
	RF = new int[16];											// assign memory to the variables
	ICache = new string[256];
	DCache = new string[256];

	for(int i=0; i<16;i++)										// set all ports to be closed
		RFport[i] = false;

	for(int i=0; i<5;i++)										// set the stage bools
		stage[i]=false;
}

// Destructor
Program::~Program(){
	delete[] RF;												// deallocate the memory
	delete[] ICache;
	delete[] DCache;
}

// Read and store the values of Registers from given file
void Program::RegisterFile(string filepath){	

	ifstream RFile(filepath);		
	string s;
	RFile >> s;

	for(int i = 0; i < 16; i++){	
		RF[i] = HexToDec(s);
		RFile >> s;
	}

	RFile.close();

}

// Read and store the values of ICache from given file
void Program::InputICache(string filepath){		

	ifstream File(filepath);

	for(int i = 0; i < 256; i++)
		File >> ICache[i];

	File.close();
}

// Read and store the values of DCache from given file
void Program::InputDCache(string filepath){	

	ifstream File(filepath);

	for(int i = 0; i < 256; i++)
		File >> DCache[i];

	File.close();
}

// Handles pipelining
void Program::Pipeline(){

	cycle++;

	for(int i=0; i<5; i++){								// set the next stage to be true
		if(stage[i] == false){
			stage[i] = true;
			break;
		}
	}

	if(stage[4])										// perform WB stage
		WriteBack();

	if(stage[3])										// perform MEM stage
		MemoryAccess();

	if(stage[2]){										// perform EX stage
		Execution();
		outputq.push(ALUOutput);
	}

	if(stage[1]){										// perform ID stage
		IDecode();

		if(opcode == 'f'){								// if the opcode is for Halt instruction
			HI++;										// jump to the respective function
			Halt();
			return;
		}
		
		if(RFport[operand[0]] || RFport[operand[1]] || RFport[operand[2]]){		// check if there is data dependency
			DS += 2;
			Data();																// jump to the respective function for stall
			return;
		}

		if(opcode == 'a' || opcode == 'b'){										// check for control instruction
			CS += 2;
			Branch();															// jump to the respective function for stall
			return;
		}

		if(opcode >= '0' && opcode <= '8')										// Set the port as open
			RFport[operand[0]] = true;

		opq.push(opcode);
		R0q.push(operand[0]);
	}

	if(stage[0])										//perform IF stage
		IFetch();

	if(PC < 256)
		Pipeline();

}

// Handles Data stalls
void Program::Data(){

	if(stage[0])
		IFetch();

	// Perform the remaining stages in the stalls

	for(int i=2; i>0; i--){
		cycle++;

		for(int i=0; i<5; i++){
			if(stage[i] == false){
				stage[i] = true;
				break;
			}
		}

		if(i == 2){
			if(stage[4])
				WriteBack();
			
			if(stage[3])
				MemoryAccess();
		}
		else
			if(stage[4])
				WriteBack();
	}

	for(int i=0; i<16;i++)							// close all ports
		RFport[i] = false;

	for(int i=0; i<5;i++)							// reset all stages
		stage[i]=false;


	// flush all buffer in the queues

	while(!opq.empty())
		opq.pop();

	while(!R0q.empty())
		R0q.pop();

	while(!outputq.empty())
		outputq.pop();

	opq.push(opcode);
	R0q.push(operand[0]);
	stage[0] = stage[1] = true;

	if(opcode=='a' || opcode == 'b'){				// jump to control stall function
		PC -= 2;
		CS += 2;
		Branch();
	 	return;
	}

	Pipeline();
}

// Handles Control stalls
void Program::Branch(){

	// Perform the remaining stages in the stalls

	for(int i=2; i>0; i--){
		cycle++;

		for(int i=0; i<5; i++){
			if(stage[i] == false){
				stage[i] = true;
				break;
			}
		}

		if(i == 2){
			if(stage[4])
				WriteBack();

			if(stage[3])
				MemoryAccess();

			if(stage[2]){
				Execution();
				PC = ALUOutput;
			}
		}
		else{
			if(stage[4])
				WriteBack();

				if(stage[0])
					IFetch();
		}
	}

	for(int i=0; i<5;i++)			// reset the stages
		stage[i] = false;

	// flush the buffer queues

	while(!opq.empty())
		opq.pop();

	while(!R0q.empty())
		R0q.pop();

	while(!outputq.empty())
		outputq.pop();

	stage[0] = true;

	Pipeline();

}

// Controls the Halt instructions
void Program::Halt(){
	
	//Perform the remaining stages and dont take further input in pipeline

	for(int i=3; i>0; i--){
		cycle++;

		if(i == 2){
			
			if(stage[4])
				WriteBack();

			if(stage[3])
				MemoryAccess(); 

			if(stage[2])
				Execution();
			
		}
		else if(i==2){
			
			if(stage[4])
				WriteBack();
			
			if(stage[3])
				MemoryAccess();
			
		}
		else{
			if(stage[4])
				WriteBack();
			
		}
	}
}

// Fetches the instruction from ICache	
void Program::IFetch(){
	stringstream ss;
	ss << ICache[PC];
	ss << ICache[PC+1];
	IR = ss.str();
	PC += 2;
}

// Decodes the fetched instruction
void Program::IDecode(){
	opcode = IR[0];
	for(int i=0; i<3; i++){
		string s (1, IR[i+1]);
		operand[i] = HexToDec(s);
	}
}

// Executes the instruction
void Program::Execution(){

	if(opcode=='0'){
		AI++;
		ALUOutput = RF[operand[1]] + RF[operand[2]];
	}
	else if(opcode=='1'){
		AI++;
		ALUOutput = RF[operand[1]] - RF[operand[2]];
	}
	else if(opcode=='2'){
		AI++;
		ALUOutput = RF[operand[1]] * RF[operand[2]];
	}
	else if(opcode=='3'){
		AI++;
		ALUOutput = RF[operand[0]] + 1;
	}
	else if(opcode=='4'){
		LI++;
		ALUOutput = RF[operand[1]] & RF[operand[2]];
	}
	else if(opcode=='5'){
		LI++;
		ALUOutput = RF[operand[1]] | RF[operand[2]];
	}
	else if(opcode=='6'){
		LI++;
		ALUOutput = ~RF[operand[1]];
	}
	else if(opcode=='7'){
		LI++;
		ALUOutput = RF[operand[1]] ^ RF[operand[2]];
	}
	else if(opcode=='8'){
		DI++;
		ALUOutput = RF[operand[1]]+operand[2];
	}
	else if(opcode=='9'){
		DI++;
		ALUOutput = RF[operand[1]]+operand[2];
	}
	else if(opcode=='a'){
		CI++;
		if(operand[0]<8){
			ALUOutput = PC + 2*((operand[0]*16)+operand[1]);
		}
		else{
			ALUOutput = PC + 2*((-256) + ((operand[0]*16)+operand[1]));
		}
	}
	else if(opcode=='b'){
		CI++;
		if(RF[operand[0]]==0){
			if(operand[1]<8){
				ALUOutput = PC + 2*((operand[1]*16)+operand[2]);
			}
			else{
				ALUOutput = PC + 2*((-256) + ((operand[1]*16)+operand[2]));
			}
		}
		else
			ALUOutput = PC;
	}

}

// Performs the memory access
void Program::MemoryAccess(){
		
	char op = opq.front();
	int R0 = R0q.front();

	if(op=='8'){
		LMD = HexToDec(DCache[ALUOutput]);
		outputq.push(LMD);
	}
	else if(op=='9')
		DCache[ALUOutput] = DecToHex(RF[R0]);

}

// Writes back the data onto the register
void Program::WriteBack(){

	char op = opq.front();
	int R0 = R0q.front();

	if(op == '8'){
		outputq.pop();
		RF[R0] = outputq.front();
	}
	else if(op >= '0' && op <= '7'){
		RF[R0] = outputq.front();
	}

	RFport[R0]==false;

	opq.pop();
	R0q.pop();
	outputq.pop();
}

// Output the Dcache and Stats of program in resp. files
void Program::Output(){

	ofstream File1("output/DCache.txt");

	for(int i = 0; i < 256; i++)
		File1 << DCache[i] << (i != 255 ? "\n" : "");	

	File1.close();

	ofstream File2("output/Output.txt");
	File2<<"Total number of instructions executed: "<< AI+LI+DI+CI+HI <<endl;
	File2<<"Number of instructions in each class"<<endl;
	File2<<"Arithmetic instructions              : "<< AI <<endl;
	File2<<"Logical instructions                 : "<< LI <<endl;
	File2<<"Data instructions                    : "<< DI <<endl;
	File2<<"Control instructions                 : "<< CI <<endl;
	File2<<"Halt instructions                    : "<< HI <<endl;
	float x = cycle;
	float y = (AI+LI+DI+CI+HI);
	float CPI = x/y;
	File2<<"Cycles Per Instruction               : "<< CPI <<endl;
	File2<<"Total number of stalls               : "<< DS+CS <<endl;
	File2<<"Data stalls (RAW)                    : "<< DS <<endl;
	File2<<"Control stalls                       : "<< CS;

	File2.close();
}

// Converts hexadecimal number to its decimal form
int Program::HexToDec(string x){

	int Dec = 0;

	for(int i=0; i<x.length(); i++){
		int multiplier = pow(16,x.length()-1-i);
		int a;
		if(x[i] >= '0' && x[i] <= '9')
			a = x[i]-48;
		else
			a = x[i]-87;

		Dec += a*multiplier;
	}

	if(Dec < 128)
		return Dec;
	else
		return Dec-256;
}

// Converts decimal number to its hexadecimal form
string Program::DecToHex(int x){
	
	stringstream ss;

	if(x < 0)
		x += 256;

	if(x == 0){
		ss << 0;
		ss << 0;
		return ss.str();
	}

	int len = log(x)/log(16);

	for(int i=len; i>=0; i--){
		
		int y = x%16;
		x /= 16;
		char c;
		
		if(y < 10)
			c = y+48;
		else
			c = y+87;

		ss << c;
	}

	if(len == 0)
		ss << 0;

	string Hex = ss.str();

	reverse(Hex.begin(), Hex.end());

	return Hex;
}

/* main function begins*/

int main(){
	
	Program P;

	// take input from the given files

	P.RegisterFile("input/RF.txt");
	P.InputICache("input/ICache.txt");
	P.InputDCache("input/DCache.txt");

	//implement the pipeline
	P.Pipeline();
	
	// print the output
	P.Output();

	return 0;
}

/* main function ends */