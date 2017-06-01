#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <list>
#include <vector>

namespace jiffle {

	// categories -------------------------------------------------------------

	enum category {
		Primitive,
		Error,
		Particle,
		Structure,
		Instruction,
	};

	enum primitive {
		Null,
		False,
		True,
		Number,
		String,
		Symbol,
		Comment,
		Address, // for code generation, text is the path
	};

	enum error {
		UserError,
		SyntaxError,
	};

	enum particle {
		// flags
		OPERATOR_BIT = 0x80000000,

		// symbol evaluation
		Whitespace = 0,						// " \v\f\r\t"
		
		// builtin operators: arithmetics
		OpMultiply = OPERATOR_BIT | 1,		// "*"
		OpDivide = OPERATOR_BIT | 2,		// "/" 
		OpModulus = OPERATOR_BIT | 3,		// "%"
		OpAddition = OPERATOR_BIT | 4,		// "+"
		OpSubtraction = OPERATOR_BIT | 5,	// "-"

		// structure declaration
		Separator = ',',				// explicit sequence separator
		SeparatorImplicit = '\n',		// implicit sequence separator
		SequenceStart = '(',			// start of subsequence 
		SequenceEnd = ')',				// end of subsequence
		DefinitionAssign = '=',			// definition evaluation, can only be used after object
		DefinitionStart = '{',			// start of definition sequence, almost equivalent to '=('
		DefinitionEnd = '}',			// end of definition sequence, also ends evaluation
		ParameterStart = '[',			// start of argument group for definitions
		ParameterEnd = ']',				// end of arguments group for definitions
			
		// -- symbol paths --
		// . private to current and children frames (at start of symbol only)
		// . composition, accessing a nested symbol
		// :: absolute symbol path (at start of symbol only)

		//constexpr static char* KeywordFork = "<-";
		//constexpr static char* KeywordRange = "..";
		//constexpr static char* KeywordFlow = "->";

		// TODO: parse
		//Specification = ':',				// specification
		//Variance = '|',						// variance (polymorphism)
		//Reference = '&',					// reference a symbol without evaluating
		//Statefull = '$',					// updateable state
		//Escape = '\\',						// escape newline separator
		//Index = '@',						// get index in sequence
		//Fork = 7,							// '<-' forked abstraction (for each in sequence, original order is maintained)
		//Range = 8,							// '..' range (variance for numbers and characters) (sequence for list indices)
		//Flow = 9,							// '->' flow mapping (if lhs is evaluated, then compute rhs)
		//Negation = '~',						// negation (variation inversion)
		//Extern = '%',						// abstract (extern)
	
	};
		
	enum structure {
		Evaluation,
		Object,
		Definition,			// '='
		Sequence,			// '()' default
		DefinitionSequence, // '{}'
		Parameter,			// '[]'
		Module,				// per file

		// flags
		FLAGS_MASK = 0x80000000,
		ExplicitFlag = 0x80000000,
	};
		
	enum instruction {
		// -- Store --
		// @brief	Stores a value to the addressed memory
		// @item0	Primitive values: null,false,true,number,string
		//			Error values: any
		// @item1	Address
		STORE,

		//////////////
		
		SET,	// Assigns a constant into a register
		LOAD,	// Loads an addressed memory value to a register
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

		// Arithmetics
		ADD,	// addition
		SUB,	// subtraction
		DIV,	// division
		MOD,	// modulus
		POW,	// exponentiation
		MINUS,	// unary minus

		// Bitwise arithmetics
		RSHIFT,	// bitwise shift right
		LSHIFT,	// bitwise shift left
		AND,	// bitwise and
		OR,		// bitwise or
		NOT,	// bitwise not
		XOR,	// bitwise xor

	};


	// data types -------------------------------------------------------------

	struct pos {
		size_t ch;
		size_t len;
		size_t ln;
		size_t col;
	};

	struct data {
		category category;
		union {
			primitive primitive;
			error error;
			particle particle;
			structure structure;
			instruction instruction;
		};
		pos pos;
		std::string text;			// for primitives, errors, and named structures
		std::vector<data> items;	// for structures and instructions
	};

	// memory model -----------------------------------------------------------

	// Program is composed of frames of code and data
	// each frames has an unique symbol that identifies it.

	// Memory is addressed by frame name and/or index
	// . is current frame
	// :: at start to force the use of an absolute path
	// else paths are relative first, abs if rel not found
	// the last symbol can also be an index, f.ex:
	// "::0" means item on location 0 on program global memory (output)
	typedef std::string address;

	// All memory has an owner.
	// Memory can be:
	//	- constant (constant values, baked into the program)
	//	- variable (initialized on owner's start code)
	// Memory is released when the owner is no longer accessed.
	
	struct frame {
		std::string name;			// link access reference
		std::vector<data> memory;	// owned (released after cleanup)
		std::vector<data> start;	// executed instructions when loading symbol (jump to symbol)
		std::vector<data> exit;		// executed instructions on cleanup (when symbol will never be used again)
	};


	// functions --------------------------------------------------------------

	// -- Tokenize --
	// @brief	Convert an input string to tokens
	// @return	data values, categories:
	//				- primitive (all)
	//				- error (syntax and user)
	//				- particle (all)
	std::vector<data> tokenize(const std::string & code);

	// -- Structurize -- 
	// @brief	Converts particle separated tokens
	//			to a structured data tree.
	// @return	structured data values, categories:
	//				- primitive (all, leaves)
	//				- error (syntax and user, leaves)
	//				- structure (all, branches and root)
	data structurize(const std::string& moduleName, const std::vector<data>& tokens);

	//// -- Simplify --
	//// @brief	remove redundant structures
	//// @return	structured data values, same as above
	//std::vector<data> simplify(const std::vector<data>& modules);

	// -- Generate --
	// @brief	produces a set of frames with instructions
	// @note	first module provided is the entry point
	// @return	memory and instructions, 
	//			first frame is the entry point
	std::vector<frame> generate(const std::vector<data>& modules);
	
	// -- Evaluate --
	// @brief	executes the given frames to produce output memory
	// @return	the output memory produced by the program
	// @note	first frame is the entry point.
	// @note	the memory returned is equivalent to a frame memory
	//			as if the caller had owned memory.
	std::vector<data> evaluate(const std::vector<frame>& frames);

	// tests ------------------------------------------------------------------
	
	void tokenize_test();
	void structurize_test();
	void generate_test();
	void evaluate_test();

}