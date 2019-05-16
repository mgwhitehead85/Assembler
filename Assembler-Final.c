// Matt Whitehead
// Project Assembler - Part 8
// May 14, 2019
// Input Files: Part8Matt.asm, Assembler-Final.c
// Description: Write a program that will put a number into an array, print the array, and sort
// the array in ascending order.

#define _CRT_SECURE_NO_WARNINGS  // Lets us use depricated code

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char ASM_FILE_NAME[] = "Part8Matt.asm";

#define MAX 200			// strlen of simulators memory
#define COL 7			// Number of columns for output
#define LINE_SIZE 40	// For c-strings

// REGISTERS
#define AXREG 0
#define BXREG 1
#define CXREG 2
#define DXREG 3

// COMMANDS
#define HALT 5			// halt command
#define MOVREG 192      // Separate mov command
#define ADD 160         // Separate add command
#define MOVMEM 224		// Separate movmem command
#define PUT 7			// Separate put command
#define CMP 96			// Separate compare command
#define JMP 8			// Separate jump command
#define GET 6           // Separate get command
#define FUN 4			// Separate function command
#define MIDCHECK 24		// Check middle of command code
#define BRK 3			// Separate break command

// BOOLEAN
#define TRUE 1
#define FALSE 0

// OPERAND
#define CONSTANT 7			// Separate constant command
#define ADDRESS 6			// Separate address command
#define ADDRESSCONSTANT 5	// Separate BX + constant command
#define ADDRESSVALUE 4		// Separate value of BX command


enum paramType { reg, mem, constant, arrayBx, arrayBxPlus, none };

typedef short int Memory;

struct Registers
{
	int AX;
	int BX;
	int CX;
	int DX;
	int instrAddr;
	int flag;
}regis;

Memory memory[MAX] = { 0 };
int address;
int stackPointer;

void runCode();             // Executes the machine code
void splitCommand(char line[], char command[], char oper1[], char oper2[]);     // Splits line of asm into it's three parts
void convertToMachineCode(FILE *fin);   // Converts a single line of ASM to machine code
void fillMemory();          // Converts the entire ASM file and stores it in memory
void printMemoryDump();     // Prints memeory with commands represented as integes

// Helper functions
void convertToNumber(char line[], int *start, int *value);  // Converts a sub-string to an int
int whichReg(char regLetter);			// Returns the number of the letter registar
void changeToLowerCase(char line[]);	// Changes each character to lower case
void printMemoryDumpHex();				// Prints memory in hexedecimal
void putInRegister(Memory instruction, Memory oper);        // Puts value into register
void addToRegister(Memory instruction, Memory oper);        // Adds value to register
Memory whatsInRegister(Memory oper);                        // Returns value in register
void setFlag(int oper1, int oper2);		// Sets the value of the flag
void removeBrackets(char oper[]);		// Remove brackets from oper
void splitFunCommand(char line[], char command[], char oper1[], char oper2[], char str[]);		// Splits command function
void popStack();			// Placing info from stack back into registers, flag, & return address
void pushStack();			// Puts info on the stack once a function is called
void splitFunParameters(char str[], int count); // Splits up the parameters of a function
void splitAddressPlusConstant(char oper2[]);	// Splits [BX + xxxx] command
void removeSemiColon(char line[]);				// Remove semicolon and all text after

int main()
{
	printMemoryDump();
	fillMemory();
	runCode();
	printMemoryDump();
	printf("\n");
	system("pause");
	return 0;
}


/********************   runCode   ***********************
Executes the machine code
-----------------------------------------------------------*/

void runCode()
{
	address = 0;
    stackPointer = 0;
	Memory instruction;
	Memory oper1;
	Memory oper2;


	while (memory[address] != HALT)
	{
		instruction = memory[address];
		
		if (instruction == 0) {
			address++;
		}

		else if ((instruction & MOVMEM) == MOVREG)       // MOVE command instructions
		{
			oper2 = (instruction & CONSTANT);

			if ((instruction & CONSTANT) == CONSTANT)       // If value is constant then place in register
			{
				oper1 = (instruction - (MOVREG + CONSTANT)) / 8;
				address++;
				instruction = memory[address];
				putInRegister(instruction, oper1);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESS)	// Get value from memory location and put in register
			{
				oper1 = (instruction - (MOVREG + ADDRESS)) / 8;
				address++;
				instruction = memory[address];
				instruction = memory[instruction];
				putInRegister(instruction, oper1);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESSCONSTANT)	// Get value of [BX+xxxx] and put into register
			{
				oper1 = (instruction - (MOVREG + ADDRESSCONSTANT)) / 8;
				address++;
				instruction = memory[address];
				instruction = whatsInRegister(1) + instruction;
				instruction = memory[instruction];
				putInRegister(instruction, oper1);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESSVALUE)	// Get value of [BX] and put in register
			{
				oper1 = (instruction - (MOVREG + ADDRESSVALUE)) / 8;
				oper2 = memory[whatsInRegister(1)];
				putInRegister(oper2, oper1);
				address++;
			}

			else        // Get value from register and put it in another register
			{
				oper1 = (instruction - (MOVREG + oper2)) / 8;
				instruction = whatsInRegister(oper2);
				putInRegister(instruction, oper1);
				address++;
			}
		}

        else if (((instruction & MOVMEM) == 0) && ((instruction & MIDCHECK) == 0) && ((instruction & CONSTANT) == BRK))		// Break command
        {
			popStack();		// Put values from stack back into registers, flags, and return address
        }
        
		else if (((instruction & MOVMEM) == 0) && ((instruction & MIDCHECK) == 0) && ((instruction & CONSTANT) == FUN))		// Function command
		{
			pushStack();	// Put values onto stack
			address++;
			instruction = memory[address];
			address++;
			memory[instruction - 1] = address;
            memory[MAX - stackPointer] = (address + memory[address] + 1);
			address = instruction;
		}

		else if (((instruction & MOVMEM) == 0) && ((instruction & MIDCHECK) == JMP))		// JUMP command instructions
		{
			oper2 = (instruction & CONSTANT);

			if (oper2 == 0)				// Jump If Equal command (je)
			{
				if (regis.flag == 0)
				{
					address++;
					address = memory[address];
				}
				else
				{
					address = address + 2;
				}
			}

			else if (oper2 == 1)		// Jump If Not Equal command (jne)
			{
				if (regis.flag != 0)
				{
					address++;
					address = memory[address];
				}
				else
				{
					address = address + 2;
				}
			}

			else if (oper2 == 2)		// Jump If Below command (jb)
			{
				if (regis.flag == -1)
				{
					address++;
					address = memory[address];
				}
				else
				{
					address = address + 2;
				}
			}

			else if (oper2 == 3)		// Jump If Below Or Equal command (jbe)
			{
				if (regis.flag == -1 || regis.flag == 0)
				{
					address++;
					address = memory[address];
				}
				else
				{
					address = address + 2;
				}
			}

			else if (oper2 == 4)		// Jump If Above command (ja)
			{
				if (regis.flag == 1)
				{
					address++;
					address = memory[address];
				}
				else
				{
					address = address + 2;
				}
			}

			else if (oper2 == 5)		// Jump If Above Or Equal command (jae)
			{
				if (regis.flag == 1 || regis.flag == 0)
				{
					address++;
					address = memory[address];
				}
				else
				{
					address = address + 2;
				}
			}

			else if (oper2 == 6)		// Always Jump command (jmp)
			{
				address++;
				address = memory[address];
			}

			else
			{
				address = address + 2;
			}
		}

		else if ((instruction & MOVMEM) == CMP)		// COMPARE command instructions
		{
			oper2 = (instruction & CONSTANT);

			if ((instruction & CONSTANT) == CONSTANT)		// Compare values of register and constant and set flag
			{
				oper1 = whatsInRegister((instruction - (CMP + CONSTANT)) / 8);
				address++;
				oper2 = memory[address];
				setFlag(oper1, oper2);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESS)	// Compare values of register and memory location and set flag
			{
				oper1 = whatsInRegister((instruction - (CMP + ADDRESS)) / 8);
				address++;
				instruction = memory[address];
				oper2 = memory[instruction];
				setFlag(oper1, oper2);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESSCONSTANT)	// Compares register and value of [BX+xxxx] and set flag
			{
				oper1 = whatsInRegister((instruction - (CMP + ADDRESSCONSTANT)) / 8);
				address++;
				instruction = memory[address];
				instruction = whatsInRegister(1) + instruction;
				instruction = memory[instruction];
				setFlag(oper1, instruction);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESSVALUE)		// Compares register and value of [BX] and set flag
			{
				oper1 = whatsInRegister((instruction - (CMP + ADDRESSVALUE)) / 8);
				oper2 = memory[whatsInRegister(1)];
				setFlag(oper1, oper2);
				address++;
			}

			else        // Compare values of two registers and set flag
			{
				oper1 = whatsInRegister((instruction - (CMP + oper2)) / 8);
				oper2 = whatsInRegister(oper2);
				setFlag(oper1, oper2);
				address++;
			}
		}

		else if (instruction == PUT)		// PUT command instructions
		{
			printf("Output: %d\t", regis.AX);		// Outputs register AX
			printf("\n");
			printf("\n");
			address++;
		}

		else if (instruction == GET)        // GET command instructions
		{
			printf("Enter a number: ");     // Asks user to input number
			scanf("%d", &regis.AX);         // Stores in register AX
			address++;
		}

		else if ((instruction & MOVMEM) == MOVMEM)      // MOVEMEM command instructions
		{
			if ((instruction & CONSTANT) == ADDRESS)
			{
				oper1 = (instruction - (MOVMEM + ADDRESS)) / 8;     // Get value from register and put it in a memory location
				address++;
				instruction = memory[address];
				memory[instruction] = (whatsInRegister(oper1));
				address++;
			}

			else if (((instruction & CONSTANT) == ADDRESSCONSTANT))
			{
				oper1 = whatsInRegister((instruction - (MOVMEM + ADDRESSCONSTANT)) / 8); // Register value into memory location [BX+xxx]
				address++;
				instruction = memory[address];
				instruction = whatsInRegister(1) + instruction;
				memory[instruction] = whatsInRegister(oper1);
				address++;
			}

			else
			{
				oper1 = (instruction - (MOVMEM + ADDRESSVALUE)) / 8;		// Register value into memory location [BX]
				oper2 = whatsInRegister(1);
				memory[oper2] = whatsInRegister(oper1);
				address++;
			}
		}

		else if ((instruction & MOVMEM) == ADD)     // ADD command instructions
		{
			oper2 = memory[address] & CONSTANT;

			if ((instruction & CONSTANT) == CONSTANT)       // If value is constant then add to register
			{
				oper1 = (instruction - (ADD + CONSTANT)) / 8;
				address++;
				instruction = memory[address];
				addToRegister(instruction, oper1);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESS)   // If address then get value from memory location & add to register
			{
				oper1 = (instruction - (ADD + ADDRESS)) / 8;
				address++;
				instruction = memory[address];
				instruction = memory[instruction];
				addToRegister(instruction, oper1);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESSCONSTANT)	// Adds register and value of [BX+xxxx]
			{
				oper1 = (instruction - (ADD + ADDRESSCONSTANT)) / 8;
				address++;
				instruction = memory[address];
				instruction = memory[instruction] + instruction;
				instruction = memory[instruction];
				addToRegister(instruction, oper1);
				address++;
			}

			else if ((instruction & CONSTANT) == ADDRESSVALUE)		// Adds register and value of [BX]
			{
				oper1 = (instruction - (ADD + ADDRESSVALUE)) / 8;
				oper2 = memory[whatsInRegister(1)];
				addToRegister(oper2, oper1);
				address++;
			}

			else        // Return value from a register and add it to another
			{
				oper1 = (instruction - (ADD + oper2)) / 8;
				addToRegister((whatsInRegister(oper2)), oper1);
				address++;
			}
		}

		else
		{
			address++;
		}
	}
	regis.instrAddr = address;
}


/**********************   Register Commands   *************************
Commands to put/add values to registers and return values in registers
---------------------------------------------------------------------*/

// Returns value in register
Memory whatsInRegister(oper)
{
	switch (oper)
	{
	case 0:
		return regis.AX;

	case 1:
		return regis.BX;

	case 2:
		return regis.CX;

	case 3:
		return regis.DX;
	}
}

// Adds value to the register
void addToRegister(instruction, oper)
{
	switch (oper)
	{
	case 0:
		regis.AX += instruction;
		break;

	case 1:
		regis.BX += instruction;
		break;

	case 2:
		regis.CX += instruction;
		break;

	case 3:
		regis.DX += instruction;
		break;
	}
}

// Places value into the register
void putInRegister(instruction, oper)
{
	switch (oper)
	{
	case 0:
		regis.AX = instruction;
		break;

	case 1:
		regis.BX = instruction;
		break;

	case 2:
		regis.CX = instruction;
		break;

	case 3:
		regis.DX = instruction;
		break;
	}
}


/**********************   Flag Commands   *************************
Command to set the flag value
---------------------------------------------------------------------*/

// Compares two values and sets the flag
void setFlag(oper1, oper2)
{
	if (oper1 < oper2)
	{
		regis.flag = -1;
	}

	else if (oper1 == oper2)
	{
		regis.flag = 0;
	}

	else
	{
		regis.flag = 1;
	}
}


/********************   splitCommands   ***********************
Splits line of asm into it's three parts
-----------------------------------------------------------*/

// Splits normal line commands
void splitCommand(char line[], char command[], char oper1[], char oper2[])
{
	if (line[0] != 'h')
	{
		if ((line[0] == 'p') || (line[0] == 'g') || (line[0] == 10) || (line[0] == 13) || (isdigit(line[0])) || (line[0] == 'f') || (line[0] == 'b')
			|| (line[0] == ';') || ((line[0] == ' ') && (line[1] == ';')))
		{
			strcpy(command, line);
			return;
		}

		else if (line[0] == 'j')		// Split line if JUMP command
		{
			char *spacePtr = strchr(line, ' ');
			int index = (int)(spacePtr - line);
			printf("\nIndex is: %d", index);
			strncpy(command, line, index);
			command[index] = '\0';

			char *endPtr = strchr(spacePtr + 1, '\n');
			index = (int)(endPtr - (spacePtr + 1));

			strncpy(oper1, spacePtr + 1, index);
			oper1[index] = '\0';
			printf("\nOperand 1 = %s", oper1);
			return;
		}

		char *spacePtr = strchr(line, ' ');  // The pointer where the first space occurs
		int index = (int)(spacePtr - line);  // The differnce between the start point and the space
		printf("\nIndex is: %d", index);

		if (index < 1 || index > 3)
		{
			printf("\a\a\tnumber not in the range\n");
			system("pause");
			exit(1);	// This is temporary. You must find a way to deal with index out of bounds.
		}

		strncpy(command, line, index);	    // Move the command into the command variable
		command[index] = '\0';				// Add the string stopper

		printf("\nCommand = %s", command);

		char *secondSpacePtr = strchr(spacePtr + 1, ' ');
		index = (int)(secondSpacePtr - (spacePtr + 1));

		strncpy(oper1, spacePtr + 1, index);        // Move oper1 into the oper1 variable
		oper1[index] = '\0';                        // Add the string stopper

		char *endPtr = strchr(secondSpacePtr + 1, '\n');
		index = (int)(endPtr - (secondSpacePtr + 1));

        strncpy(oper2, secondSpacePtr + 1, index);	// Move oper2 into the oper2 variable
        oper2[index] = '\0';						// Add the string stopper

        printf("\nOperand 1 = %s", oper1);
        printf("\nOperand 2 = %s", oper2);
		}
	}

	else
	{
		strcpy(command, line);
		return;
	}
}

// Splits FUN command
void splitFunCommand(char line[], char command[], char oper1[], char oper2[], char str[])
{
	if (line[0] == 'f')		// Split line if FUN command
	{
		char *firstPtr = strchr(line, ' ');  // The pointer where the first space occurs
		int index = (int)(firstPtr - line);  // The differnce between the start point and the space
		printf("\nIndex is: %d", index);

		strncpy(command, line, index);	    // Move the command into the command variable
		command[index] = '\0';				// Add the string stopper

		printf("\nCommand = %s", command);

		char *secondSpacePtr = strchr(firstPtr + 1, ' ');
		index = (int)(secondSpacePtr - (firstPtr + 1));

		strncpy(oper1, firstPtr + 1, index);        // Move oper1 into the oper1 variable
		oper1[index] = '\0';                        // Add the string stopper

		char *thirdPtr = strchr(secondSpacePtr + 1, ' ');
		index = (int)(thirdPtr - (secondSpacePtr + 1));

		strncpy(oper2, secondSpacePtr + 1, index);	// Move oper2 into the oper2 variable
		oper2[index] = '\0';

		char *endPtr = strchr(thirdPtr + 1, '\n');
		index = (int)(endPtr - (thirdPtr + 1));

		strncpy(str, thirdPtr + 1, index);			// Return function parameters
		str[index] = '\0';

		printf("\nOperand 1 = %s", oper1);
		printf("\nOperand 2 = %s", oper2);
		printf("\nParameters = %s", str);
	}

	else
	{
		strcpy(command, line);
		return;
	}
}

//Removes semicolon and all text after
void removeSemiColon(char line[])
{
	int semicolon = 0;

	for (int i = 0; i < strlen(line); i++)
	{
		if (line[i] == ';')
		{
			semicolon = 1;
		}
	}

	if (semicolon == 1)
	{
		char *semicolonPtr = strchr(line, ';');
		int index = (int)(semicolonPtr - line);
		strcpy(line, line, index);
		line[index] = '\n';
		return;
	}

	else {
		return;
	}
}

// Removes brackets from operands
void removeBrackets(char oper[]) {
	char *addressNum = strchr(oper, ']');
	int index = (int)(addressNum - oper);
	strncpy(oper, oper + 1, index - 1);
	oper[index - 1] = '\0';
	return;
}

// Splits FUN from parameters
void splitFunParameters(char str[], int count) {
	int numParameters = 0;
	char *pch;
	pch = strtok(str, " ,.-[]");
	while (pch != NULL)
	{
		memory[address] = atoi(pch);
		numParameters++;
		pch = strtok(NULL, " ,.-[]");
		address++;
	}

	if (numParameters == count)
	{
		return;
	}

	else
	{
		memory[address - (numParameters + 3)] = HALT;
		return;
	}
}

// Splits BX + Constant command
void splitAddressPlusConstant(char oper2[]) {
	Memory constantValue;
	char *pch;
	pch = strtok(oper2, " ,.-[]+");
	pch = strtok(NULL, " ,.-[]+");
	constantValue = atoi(pch);
	memory[address] = constantValue;
	return;
}


/********************   stackCommands   ***********************
Pushes and takes values off the stack
---------------------------------------------------------------------*/

// Puts values onto the stack
void pushStack()
{
	stackPointer++;
	memory[MAX - stackPointer] = whatsInRegister(0);
	stackPointer++;
	memory[MAX - stackPointer] = whatsInRegister(1);
	stackPointer++;
	memory[MAX - stackPointer] = whatsInRegister(2);
	stackPointer++;
	memory[MAX - stackPointer] = whatsInRegister(3);
	stackPointer++;
	memory[MAX - stackPointer] = regis.flag;
	stackPointer++;
}

// Returns values off the stack
void popStack()
{
	Memory returnValue;
	int returnAddress = memory[MAX - stackPointer];
	memory[returnAddress] = whatsInRegister(0);
	address = returnAddress + 1;
	stackPointer--;
	regis.flag = memory[MAX - stackPointer];
	stackPointer--;
	returnValue = memory[MAX - stackPointer];
	putInRegister(returnValue, 3);
	stackPointer--;
	returnValue = memory[MAX - stackPointer];
	putInRegister(returnValue, 2);
	stackPointer--;
	returnValue = memory[MAX - stackPointer];
	putInRegister(returnValue, 1);
	stackPointer--;
	returnValue = memory[MAX - stackPointer];
	putInRegister(returnValue, 0);
	stackPointer--;
}


/********************   convertToMachineCode   ***********************
Converts a single line of ASM to machine code
---------------------------------------------------------------------*/

void convertToMachineCode(FILE *fin)
{
	char line[LINE_SIZE];		// Full command
	char command[LINE_SIZE];	// The asm commmand
	char oper1[LINE_SIZE], oper2[LINE_SIZE];	// The two operands could be empty
	char str[LINE_SIZE];
	int machineCode = 0;			// One line of converted asm code from the file
	char *plus = "+";

	fgets(line, LINE_SIZE, fin);    // Takes one line from the asm file

	changeToLowerCase(line);

	if (line[0] == ';')			// Skips lines beginning with ';'
	{
		address = address;
	}

	else if ((line[0] == ' ') && (line[1] == ';'))		// Puts a '0' in lines beginning with ' ;'
	{
		memory[address] = 0;
		address++;
	}

	else
	{

		removeSemiColon(line);
		splitCommand(line, command, oper1, oper2);

		if (line[0] == 'f')			// FUN command
		{
			int count = 0;
			splitFunCommand(line, command, oper1, oper2, str);
			memory[address] = FUN;
			address++;

			removeBrackets(oper1);
			memory[address] = atoi(oper1);
			address++;

			count = atoi(oper2);
			memory[address] = atoi(oper2);
			address++;

			splitFunParameters(str, count);
			address++;
		}

		else if (line[0] == 'b')	// BRK command
		{
			memory[address] = BRK;
			address++;
		}

		else if (isdigit(line[0]))		// Store number if digit
		{
			memory[address] = atoi(line);
			address++;
		}

		else if ((line[0] == 10) || (line[0] == 13))		// If asm line is blank then makes location '0'
		{
			memory[address] == 0;
			address++;
		}

		else if (command[0] == 'h')      // HALT command
		{
			memory[address] = HALT;
			address++;
		}

		else if (command[0] == 'g')      // GET command
		{
			memory[address] = GET;
			address++;
		}

		else if (command[0] == 'p')		// PUT command
		{
			memory[address] = PUT;		// Machine code for put constant to register
			address++;
		}

		else if (command[0] == 'j')		// JUMP command
		{
			if (command[1] == 'e')			// Machine code for Jump If Equal
			{
				machineCode = JMP;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if (command[1] == 'n')		// Machine code for Jump If Not Equal	
			{
				machineCode = JMP;
				machineCode += 1;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if (command[1] == 'b' && command[2] == 'e')	// Machine code for Jump If Below Or Equal
			{
				machineCode = JMP;
				machineCode += 3;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if (command[1] == 'b')		// Machine code for Jump If Below
			{
				machineCode = JMP;
				machineCode += 2;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if (command[1] == 'a' && command[2] == 'e')		// Machine code for Jump If Above Or Equal
			{
				machineCode = JMP;
				machineCode += 5;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if (command[1] == 'a')		// Machine code for Jump If Above
			{
				machineCode = JMP;
				machineCode += 4;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if (command[1] == 'm')		// Machine code for Always Jump
			{
				machineCode = JMP;
				machineCode += 6;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else
			{
				machineCode = JMP;
				machineCode += 7;
				memory[address] = machineCode;
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}
		}

		else if (command[0] == 'c')		// COMPARE command
		{
			if (isdigit(oper2[0]))      // Parse if oper2 is constant
			{
				machineCode = CMP;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += CONSTANT;
				memory[address] = machineCode;
				address++;

				memory[address] = atoi(oper2);
				address++;
			}

			else if (oper2[0] == '[' && (strstr(oper2, plus) == NULL))       // Parse if oper2 is a memory location
			{
				if (isdigit(oper2[1])) {		// If memory location is a value
					machineCode = CMP;
					machineCode += (whichReg(oper1[0]) << 3);
					machineCode += ADDRESS;
					memory[address] = machineCode;
					address++;

					removeBrackets(oper2);
					memory[address] = atoi(oper2);
					address++;
				}

				else {							// If memory location is [BX]
					machineCode = CMP;
					machineCode += (whichReg(oper1[0]) << 3);
					machineCode += ADDRESSVALUE;
					memory[address] = machineCode;
					address++;
				}
			}

			else if (oper1[0] != '[' && oper2[0] != '[')		// Parse if oper1 & oper2 are registers
			{
				machineCode = CMP;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += whichReg(oper2[0]);
				memory[address] = machineCode;
				address++;
			}

			else
			{
				machineCode = CMP;							// Parse if oper2 is [BX+xxxx]
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += ADDRESSCONSTANT;
				memory[address] = machineCode;
				address++;

				splitAddressPlusConstant(oper2);
				address++;
			}
		}

		else if (command[0] == 'm')     // MOVE or MOVEMEM command
		{

			if (isdigit(oper2[0]))      // Parse if oper2 is constant
			{
				machineCode = MOVREG;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += CONSTANT;
				memory[address] = machineCode;      // Machine code for move constant to register
				address++;

				memory[address] = atoi(oper2);      // Constant value stored in next memory location
				address++;
			}

			else if ((oper1[0] == '[') && (isdigit(oper1[1])))       // Parse if oper1 is a memory location
			{
				machineCode = MOVMEM;
				machineCode += (whichReg(oper2[0]) << 3);
				machineCode += ADDRESS;
				memory[address] = machineCode;  
				address++;

				removeBrackets(oper1);
				memory[address] = atoi(oper1);
				address++;
			}

			else if ((oper1[0] == '[') && (oper1[1] == 'b') && (strstr(oper1, plus) == NULL))       // Parse if oper1 is [BX]. Movmem.
			{
				machineCode = MOVMEM;
				machineCode += (whichReg(oper2[0]) << 3);
				machineCode += ADDRESSVALUE;
				memory[address] = machineCode;
				address++;
			}

			else if ((oper1[0] == '[') && (oper1[1] == 'b'))       // Parse if oper1 is [BX+xxxx]. Movmem.
			{
				machineCode = MOVMEM;
				machineCode += (whichReg(oper2[0]) << 3);
				machineCode += ADDRESSCONSTANT;
				memory[address] = machineCode;     
				address++;

				splitAddressPlusConstant(oper1);
				address++;
			}

			else if (oper2[0] == '[' && (strstr(oper2, plus) == NULL))  // Parse if oper1 is memory location
			{
				if (isdigit(oper2[1]))
				{									// Parse if oper2 is digit memory location
					machineCode = MOVREG;
					machineCode += (whichReg(oper1[0]) << 3);
					machineCode += ADDRESS;
					memory[address] = machineCode;      
					address++;

					removeBrackets(oper2);
					memory[address] = atoi(oper2);
					address++;
				}

				else
				{									// Parse if oper2 is [BX]
					machineCode = MOVREG;
					machineCode += (whichReg(oper1[0]) << 3);
					machineCode += ADDRESSVALUE;
					memory[address] = machineCode;
					address++;
				}
			}

			else if (oper1[0] != '[' && oper2[0] != '[')       // Parse if oper1 & oper2 are registers
			{
				machineCode = MOVREG;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += whichReg(oper2[0]);
				memory[address] = machineCode;      // Machine code to move register value into another register
				address++;
			}

			else
			{
				machineCode = MOVREG;						// Parse if oper2 is [BX+xxxx]
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += ADDRESSCONSTANT;
				memory[address] = machineCode;
				address++;

				splitAddressPlusConstant(oper2);
				address++;
			}
		}

		else if (command[0] == 'a')         // ADD command
		{
			if (isdigit(oper2[0]))          // Parse if oper2 is constant
			{
				machineCode = ADD;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += CONSTANT;
				memory[address] = machineCode;      // Machine code to add constant to register
				address++;

				memory[address] = atoi(oper2);
				address++;
			}

			else if (oper2[0] == '[' && (strstr(oper2, plus) == NULL))       // Parse if oper2 is a memory location
			{
				if (isdigit(oper2[1])) 
				{									// Parse is oper2 is digit memory location
					machineCode = ADD;
					machineCode += (whichReg(oper1[0]) << 3);
					machineCode += ADDRESS;
					memory[address] = machineCode; 
					address++;

					removeBrackets(oper2);
					memory[address] = atoi(oper2);
					address++;
				}

				else 
				{										// Parse is oper2 is [BX]
					machineCode = ADD;
					machineCode += (whichReg(oper1[0]) << 3);
					machineCode += ADDRESSVALUE;
					memory[address] = machineCode;
					address++;
				}
			}

			else if (oper1[0] != '[' && oper2[0] != '[')		// Parse if oper1 & oper2 are registers
			{
				machineCode = ADD;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += whichReg(oper2[0]);
				memory[address] = machineCode;      // Machine code to get value from register & add it to another register
				address++;
			}

			else
			{											// Parse if oper2 is [BX+xxxx]
				machineCode = ADD;
				machineCode += (whichReg(oper1[0]) << 3);
				machineCode += ADDRESSCONSTANT;
				memory[address] = machineCode;
				address++;

				splitAddressPlusConstant(oper2);
				address++;
			}
		}
	}

	printf("\n");
	printMemoryDump();
}


/********************   fillMemory   ***********************
Changes the assembly code to machine code and places the
commands into the memory.
-----------------------------------------------------------*/

void fillMemory()
{
	address = 0;
	FILE *fin;		// File pointer for reading in the assembly code

					// Recommend changing so you can type in file name
	fopen_s(&fin, ASM_FILE_NAME, "r");
	if (fin == NULL)
	{
		printf("Error, file didn't open\n\nExiting program...\n\n");
		system("pause");
		exit(1);
	}

	for (int i = 0; i < MAX && !feof(fin); i++)
	{
		convertToMachineCode(fin);
	}
}


/****************************   printMemoryDump   ********************************
Prints memory by number
MAX is the amount of elements in the memory array (Vicki used 100)
COL is the number of columns that are to be displayed (Vicki used 7; was originally called COLUMNS)
---------------------------------------------------------------------------------*/

void printMemoryDump()
{
	int numRows = MAX / COL + 1;	// Number of rows that will print
	int carryOver = MAX % COL;		// Number of columns on the bottom row
	int location;                   // The current location being called

	for (int row = 0; row < numRows; row++)
	{
		location = row;

		// Print(setw( 3 ) << location << "." << setw( 5 ) << memory[location];
		for (int column = 0; location < MAX&&column < COL; column++)
		{
			if (!(numRows - 1 == row&&carryOver - 1 < column))
			{
				printf("%5d.%5d", location, memory[location]);
				location += (numRows - (carryOver - 1 < column));
			}
		}
		printf("\n");
	}

	printf("\n");
	printf("AX:%d\t", regis.AX);
	printf("BX:%d\t", regis.BX);
	printf("CX:%d\t", regis.CX);
	printf("DX:%d\t", regis.DX);
	printf("\n\n");
	printf("Instruction: %d\n", regis.instrAddr);
	printf("Flag: %d", regis.flag);

	printf("\n\n");
}


//-----------------------------------------------------------------------------
//**************   Helper functions   *****************************************
//-----------------------------------------------------------------------------


/*****************************   ConvertToNumber   ******************************
Takes in a line (string) and converts the characters that they represent.
*	line - is the string of assembly code to convert
*	start - is the location where the line is being coverted,
It starts at the beginning of number and it passed back at the next location
*	value - is the integer value of the digits in the code
---------------------------------------------------------------------------------*/

void convertToNumber(char line[], int *start, int *value)
{
	char number[16];
	int negative = FALSE;
	int i = 0;

	if (line[*start] == '-')
	{
		++*start;
		negative = TRUE;
	}

	while (i < 16 && strlen(line) > *start && isdigit(line[*start]))
	{
		number[i] = line[*start];
		i++;
		++*start;
	}

	number[i] = '\0';
	*value = atoi(number);

	if (negative == TRUE)
	{
		*value = -*value;// *-1;
	}
}


/*********************   whichReg   *************************
Changes the letter of the registar to a number.
------------------------------------------------------------*/

int whichReg(char regLetter)
{
	if (regLetter == 'a')
	{
		return AXREG;
	}
	else if (regLetter == 'b')
	{
		return BXREG;
	}
	else if (regLetter == 'c')
	{
		return CXREG;
	}
	else if (regLetter == 'd')
	{
		return DXREG;
	}
	return -1;      // Something went wrong if -1 is returned
}


/*********************   changeToLowerCase   ********************
Changes each character to lower case
----------------------------------------------------------------*/

void changeToLowerCase(char line[])
{
	int index = 0;
	while (index < strlen(line))
	{
		line[index] = tolower(line[index]);
		index++;
	}
}


/*********************   changeToLowerCase   ********************
Prints memory in hexedecimal
----------------------------------------------------------------*/

void printMemoryDumpHex()
{
	int numRows = MAX / COL + 1;	// Number of rows that will print
	int carryOver = MAX % COL;		// Number of columns on the bottom row
	int location;                   // The current location being called

	for (int row = 0; row < numRows; row++)
	{
		location = row;

		// Print(setw( 3 ) << location << "." << setw( 5 ) << memory[location];
		for (int column = 0; location < MAX && column < COL; column++)
		{
			if (!(numRows - 1 == row && carryOver - 1 < column))
			{
				printf("%5d.%3x", location, memory[location]);
				location += (numRows - (carryOver - 1 < column));
			}
		}
		printf("\n");
	}

	printf("\n");
	printf("AX:%d\t", regis.AX);
	printf("BX:%d\t", regis.BX);
	printf("CX:%d\t", regis.CX);
	printf("DX:%d\t", regis.DX);
	printf("\n\n");
	printf("Instruction: %d\n", regis.instrAddr);
	printf("Flag: %d", regis.flag);

	printf("\n\n");
}
