#pragma once

#include <cstdint>
#include <memory>

namespace jiffle {
	namespace data {



		// types --------------------------------------------------------------------

		typedef uint8_t byte;
		typedef bool bool_t;
		typedef int64_t integer_t;
		typedef long double real_t;

		enum type {
			Void,
			Bool,
			Integer,
			Real,
			String,
			Error,

			Address,
			Instruction,
		};

		struct type_info {
			size_t type : 3;
			size_t bytelen : 29;
		};
		
		// data values --------------------------------------------------------

		struct value {
			type_info _info;
		};

		struct boolean : value {
			bool value;
		};

		struct integer : value {
			integer_t value;
		};

		struct real : value {
			real_t value;
		};

		// instructions -------------------------------------------------------
		
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

		struct instruction : value {
			opcode op;
		};


	}
}