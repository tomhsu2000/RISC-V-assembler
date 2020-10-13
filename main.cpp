//實作 RISC-V 組譯器
//輸入: 一段RISC-V組語的code
//輸出: 對應的machine code
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <string.h>

using namespace std;

// all instructions
const char insts[37][6]{ "lui","auipc","jal","jalr","beq","bne",
"blt","bge","bltu","bgeu","ld","lh","lw","lbu","lhu","sd","sh",
"sw","addi","slti","sltiu","xori","ori","andi","slli","srli",
"srai","add","sub","sll","slt","sltu","xor","srl","sra","or",
"and" };
// type of instruction
// U-type : 0     UJ-type : 1
// I-type : 2,6   SB-type : 3
// S-type : 4     R-type : 5
const int inst_type[37]{ 0,0,1,2,3,3,3,3,3,3,6,6,6,6,6,4,4,4,2,2,
2,2,2,2,5,5,5,5,5,5,5,5,5,5,5,5,5 };
// instruction opcode & function code
const char inst_code[37][33]{
	"22222222222222222222222220110111","22222222222222222222222220010111",
	"22222222222222222222222221101111","22222222222222222000222221100111",
	"22222222222222222000222221100011","22222222222222222001222221100011",
	"22222222222222222100222221100011","22222222222222222101222221100011",
	"22222222222222222110222221100011","22222222222222222111222221100011",
	"22222222222222222000222220000011","22222222222222222001222220000011",
	"22222222222222222010222220000011","22222222222222222100222220000011",
	"22222222222222222101222220000011","22222222222222222000222220100011",
	"22222222222222222001222220100011","22222222222222222010222220100011",
	"22222222222222222000222220010011","22222222222222222010222220010011",
	"22222222222222222011222220010011","22222222222222222100222220010011",
	"22222222222222222110222220010011","22222222222222222111222220010011",
	"00000002222222222001222220010011","00000002222222222101222220010011",
	"01000002222222222101222220010011","00000002222222222000222220110011",
	"01000002222222222000222220110011","00000002222222222001222220110011",
	"00000002222222222010222220110011","00000002222222222011222220110011",
	"00000002222222222100222220110011","00000002222222222101222220110011",
	"01000002222222222101222220110011","00000002222222222110222220110011",
	"00000002222222222111222220110011",
};
map<string, int> lable_map;

void typeU(const char inst[], char mcode[], int index);

void typeUJ(const char inst[], char mcode[], int index);

void typeI(const char inst[], char mcode[], int index, int type);

void typeSB(const char inst[], char mcode[], int index, int number);

void typeS(const char inst[], char mcode[], int index);

void typeR(const char inst[], char mcode[], int index);

int distin(const char inst[]);

void dec_bin_trans(int decimal, char bin[], int bin_num);

void lable(char inst[], int number);

void error_and_die(const char message[]);

int main()
{
	char inst[20], mcode[33];
	vector<string> inst_vec;
	while (1) {
		cin.getline(inst, 20);
		if (strlen(inst) == 0)
			break;
		lable(inst, inst_vec.size());
		inst_vec.push_back(inst + 4);
	}
	for (int i = 0; i < inst_vec.size(); i++) {
		if (inst_vec[i].empty())
			continue;
		int index = distin(inst_vec[i].c_str());
		switch (inst_type[index])
		{
		case 0:
			typeU(inst_vec[i].c_str(), mcode, index);
			break;
		case 1:
			typeUJ(inst_vec[i].c_str(), mcode, index);
			break;
		case 2:
			typeI(inst_vec[i].c_str(), mcode, index, 0);
			break;
		case 6:
			typeI(inst_vec[i].c_str(), mcode, index, 1);
			break;
		case 3:
			typeSB(inst_vec[i].c_str(), mcode, index, i);
			break;
		case 4:
			typeS(inst_vec[i].c_str(), mcode, index);
			break;
		case 5:
			typeR(inst_vec[i].c_str(), mcode, index);
			break;
		default:
			error_and_die("Type do not define");
			break;
		}
		cout << mcode << endl;
	}

}

void typeU(const char inst[], char mcode[], int index)
{
	int i, decimal;
	strncpy(mcode, 33, inst_code[index], 33);
	for (i = 0; inst[i] != ' '; i++);
	decimal = (int)atoi(inst + i + 2);
	dec_bin_trans(decimal, mcode + 20, 5);//rd
	for (i += 2; inst[i] != ','; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, mcode, 20);//imm
}

void typeUJ(const char inst[], char mcode[], int index)
{
	int i, decimal;
	char temp[21];
	strncpy(mcode, 33, inst_code[index], 33);
	for (i = 0; inst[i] != ' '; i++);
	decimal = (int)atoi(inst + i + 2);//rd
	dec_bin_trans(decimal, mcode + 20, 5);
	for (i += 2; inst[i] != ','; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, temp, 21);//offset
	for (i = 0; i < 20; i++) {
		if (1 <= i && i <= 8)
			mcode[i + 11] = temp[i];
		else if (10 <= i && i <= 19)
			mcode[i - 9] = temp[i];
		else if (i == 9)
			mcode[i + 2] = temp[i];
		else
			mcode[i] = temp[i];
	}
}

void typeI(const char inst[], char mcode[], int index, int type)
{
	int i, decimal;
	strncpy(mcode, 33, inst_code[index], 33);
	for (i = 0; inst[i] != ' '; i++);
	decimal = (int)atoi(inst + i + 2);
	dec_bin_trans(decimal, mcode + 20, 5);//rd
	if (type == 0) {
		for (i += 2; inst[i] != 'x'; i++);
		decimal = (int)atoi(inst + i + 1);
		dec_bin_trans(decimal, mcode + 12, 5);//rs1
		for (i++; inst[i] != ','; i++);
		decimal = (int)atoi(inst + i + 1);//imm
		dec_bin_trans(decimal, mcode, 12);
	}
	else {// load instruction
		for (i += 2; inst[i] != ','; i++);
		decimal = (int)atoi(inst + i + 1);
		dec_bin_trans(decimal, mcode, 12);//offset
		for (i++; inst[i] != 'x'; i++);
		decimal = (int)atoi(inst + i + 1);
		dec_bin_trans(decimal, mcode + 12, 5);//rs1
	}
}

void typeSB(const char inst[], char mcode[], int index, int number)
{
	int i, j, decimal;
	char lable[3], temp[13];
	strncpy(mcode, 33, inst_code[index], 33);
	for (i = 0; inst[i] != ' '; i++);
	decimal = (int)atoi(inst + i + 2);
	dec_bin_trans(decimal, mcode + 12, 5);//rs1
	for (i += 2; inst[i] != 'x'; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, mcode + 7, 5);//rs2
	for (i++; inst[i] != ','; i++);
	for (i++, j = 0; inst[i] != '\0'; i++, j++)
		lable[j] = inst[i];
	lable[j] = '\0';
	dec_bin_trans((lable_map[lable] - number) * 4, temp, 13);//offset
	for (i = 0; i < 12; i++) {
		if (2 <= i && i <= 7)
			mcode[i - 1] = temp[i];
		else if (8 <= i && i <= 11)
			mcode[i + 12] = temp[i];
		else if (i == 1)
			mcode[i + 23] = temp[i];
		else
			mcode[i] = temp[i];
	}
}

void typeS(const char inst[], char mcode[], int index)
{
	int i, decimal;
	char temp[12];
	strncpy(mcode, 33, inst_code[index], 33);
	for (i = 0; inst[i] != ' '; i++);
	decimal = (int)atoi(inst + i + 2);
	dec_bin_trans(decimal, mcode + 7, 5);//rs2
	for (i += 2; inst[i] != ','; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, temp, 12);//offset
	for (int j = 0; j < 12; j++) {
		if (j >= 7)
			mcode[j + 13] = temp[j];
		else
			mcode[j] = temp[j];
	}
	for (i++; inst[i] != 'x'; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, mcode + 12, 5);//rs1
}

void typeR(const char inst[], char mcode[], int index)
{
	int i, decimal;
	strncpy(mcode, 33, inst_code[index], 33);
	for (i = 0; inst[i] != ' '; i++);
	decimal = (int)atoi(inst + i + 2);
	dec_bin_trans(decimal, mcode + 20, 5);//rd
	for (i += 2; inst[i] != 'x'; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, mcode + 12, 5);//rs1
	for (i++; inst[i] != 'x'; i++);
	decimal = (int)atoi(inst + i + 1);
	dec_bin_trans(decimal, mcode + 7, 5);//rs2
}

// distin instruction's index in array
int distin(const char inst[])
{
	char inst_code[6];
	int j, i;
	for (i = 0, j = 0; inst[i] != ' '; i++, j++)
		inst_code[j] = inst[i];
	inst_code[j] = '\0';
	for (i = 0; i < 37; i++)
		if (strcmp(insts[i], inst_code) == 0)
			break;
	return i;
}

// transform decimal to 2's complement binary
void dec_bin_trans(int decimal, char bin[], int bin_num)
{
	int pos = bin_num;
	bool negative = false;
	if (decimal < 0) {
		negative = true;
		decimal *= -1;
		decimal--;
	}
	do {
		bin[--pos] = (char)(decimal % 2 + '0');
	} while (decimal /= 2);
	while (pos != 0)
		bin[--pos] = '0';
	if (negative) {
		while (pos != bin_num) {
			if (bin[pos] == '0')
				bin[pos++] = '1';
			else
				bin[pos++] = '0';
		}
	}
}

//store where lable appear
void lable(char inst[], int number)
{
	int i;
	char lable[3];
	if (inst[0] == ' ')
		return;
	for (i = 0; inst[i] != ':'; i++)
		lable[i] = inst[i];
	lable[i] = '\0';
	lable_map[lable] = number;
}

void error_and_die(const char message[])
{
	cout << message << endl;
	exit(1);
}
