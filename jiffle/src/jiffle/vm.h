#pragma once

#include "data.h"
#include "expr.h"

namespace jiffle {
	namespace vm {
				
		// opcodes ------------------------------------------------------------
		enum opcode : unsigned char {
			// Memory
			SET,	// Assigns a constant into a register
			LOAD,	// Loads an addressed memory value to a register
			STORE,	// Stores the value of a register to addressed memory
			FENCE,	// memory barrier between loads and stores
			CAS,	// compare and swap atomic operator

			// Control flow
			JUMP,	// Unconditional jump to address in register 
			IFZ,	// Perform next instruction if value in register == 0
			IFNZ,	// Perform next instruction if value in register != 0
			IFL,	// Perform next instruction if value in register < 0
			IFLE,	// Perform next instruction if value in register <= 0
			IFG,	// Perform next instruction if value in register > 0
			IFGE,	// Perform next instruction if value in register >= 0
			
			// Bitwise arithmetics
			RSHIFT,	// bitwise shift right
			LSHIFT,	// bitwise shift left
			AND,	// bitwise and
			OR,		// bitwise or
			NOT,	// bitwise not
			XOR,	// bitwise xor

			// Integer Arithmetics
			ADD,	// addition
			SUB,	// subtraction
			DIV,	// division
			MOD,	// modulus
			POW,	// exponentiation
			MINUS,	// unary minus

			// Floating Point Arithmetics
			FADD,	// floating point addition
			FSUB,	// floating point subtraction
			FDIV,	// floating point division
			FMOD,	// floating point modulus
			FPOW,	// floating point exponentiation
			FMINUS,	// floating point unary minus
		};

		// memory model -------------------------------------------------------

		// Program is composed of tables of code and data
		// each table has an unique symbol that identifies it.
		
		// All memory has an owner.
		// Memory can be:
		//	- constant (constant values, baked into the program)
		//	- variable (initialized on owner's start code)
		// Memory is released when the owner is no longer accessed.


		// Memory addressing is by owner symbol path 
		// and index into his memory buffer
		struct address {
			std::string symbol;
			size_t index;
		};

		// Instructions are executed when a table is loaded and when released
		struct instruction {
			opcode opcode;
			// ...
		};
		
		struct table {
			std::string symbol;					// link access reference
			std::vector<data::byte> memory;		// owned (released after cleanup)

			std::vector<instruction> start;		// executed code on jump to symbol
			std::vector<instruction> end;		// executed code on cleanup (when symbol never used again)
		};

		// TODO: input parameters / external borrowed memory referenced by symbol path and index 


		// functions ----------------------------------------------------------

		// first table is the module root
		std::vector<table> generate(const expr::node& ast);

		// tests --------------------------------------------------------------
		
		void generate_test();

	}
}