#define _CRT_SECURE_NO_DEPRECATE
#pragma once

#include <vector>
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <cstring>

#include "util.h"
#include "ops.h"

#ifdef _WIN32
#define strdup _strdup
#endif

// mapping between function names and opcodes
static std::unordered_map<std::string, Op> opcodes = {
	{ "sphere", OP_SPHERE },
	{ "box", OP_BOX },
	{ "heart", OP_HEART }
};

struct Arg {
	uint32_t val;

	inline Arg(uint32_t x) : val(x)
	{
	}
};

struct Args {
	std::vector<Arg*> args;

	inline Args()
	{
	}

	inline ~Args()
	{
		for (int i = 0; i < size(); ++i) {
			delete args[i];
		}
	}

	inline Args(const Args &other)
	{
		for (int i = 0; i < other.size(); ++i) {
			add(new Arg(other[i]));
		}
	}

	inline Args &operator=(Args other)
	{
		args.swap(other.args);
		return *this;
	}

	template <typename S>
	int operator[](S i) const
	{
		return args[i]->val;
	}

	inline void add(Arg *arg)
	{
		args.push_back(arg);
	}

	inline size_t size() const
	{
		return args.size();
	}
};

/**
* This class represents a leaf in the syntax tree
*/
struct Fn {
	char *name; // Name of the function
	Args *args; // Arguments given to this function

	inline Fn(char *_name, Args *_args) : name(_name), args(_args)
	{
	}

	inline ~Fn()
	{
		free(name);
		delete args;
	}

	inline Fn(const Fn &other)
	{
		name = strdup(other.name);
		args = new Args(*other.args);
	}

	inline Fn &operator=(Fn other)
	{
		std::swap(name, other.name);
		std::swap(args, other.args);
		return *this;
	}

	inline friend std::ostream &operator<<(std::ostream &os, const Fn &expr);

	inline bool check(uint32_t &dim)
	{
		// Task 3 - add your code here
		std::unordered_map<std::string, Op>::const_iterator opit = opcodes.find(name);
		if (opit == opcodes.end()) {
			// invalid name
			std::cout << "Invalid name " << name << std::endl;
			return false;
		}

		bool valid = false;
		switch (opit->second) {
		case OP_SPHERE:
		{
			uint32_t maxx = (*args)[0] + (*args)[3];
			uint32_t maxy = (*args)[1] + (*args)[3];
			uint32_t maxz = (*args)[2] + (*args)[3];
			dim = dim > maxx ? dim : maxx;
			dim = dim > maxy ? dim : maxy;
			dim = dim > maxz ? dim : maxz;
			valid = args->size() == 5;
			break;
		}
		case OP_BOX:
		{
			uint32_t maxx = (*args)[0] + (*args)[3];
			uint32_t maxy = (*args)[1] + (*args)[4];
			uint32_t maxz = (*args)[2] + (*args)[5];
			dim = dim > maxx ? dim : maxx;
			dim = dim > maxy ? dim : maxy;
			dim = dim > maxz ? dim : maxz;
			valid = args->size() == 7;
			break;
		}
		case OP_HEART:
		{
			uint32_t maxx = (*args)[0] + (*args)[3];
			uint32_t maxy = (*args)[1] + (*args)[3];
			uint32_t maxz = (*args)[2] + (*args)[3];
			dim = dim > maxx ? dim : maxx;
			dim = dim > maxy ? dim : maxy;
			dim = dim > maxz ? dim : maxz;
			valid = args->size() == 5;
			break;
		}
		}

		if (!valid)
			std::cout << "Invalid argument count: Function " << name << std::endl;
		return valid;
	}

	inline void code(uint32_t reg, Writer &writer)
	{
		// Task 5 - add your code here
		writer << opcodes[name] << reg;
		for (int i = 0; i < args->size(); ++i) {
			writer << (*args)[i];
		}
	}
};


inline std::ostream &operator<<(std::ostream &os, const Fn &fn)
{
	os << fn.name << "(";
	for (int i = 0; i < fn.args->size(); ++i) {
		if (i) os << ", ";
		os << (*fn.args)[i];
	}
	return os << ")";
}

/**
 * This class represents a node in the syntax tree.
 */
struct Expr {
	Op op; // operation code
	Expr *lhs; // left hand argument (optional)
	Expr *rhs; // right hand argument (optional)
	Fn *fn; // function (optional)

	// unary
	inline Expr(Op _op, Expr *_lhs) : op(_op), lhs(_lhs), rhs(NULL), fn(NULL)
	{
	}

	// binary
	inline Expr(Op _op, Expr *_lhs, Expr *_rhs) : op(_op), lhs(_lhs), rhs(_rhs), fn(NULL)
	{
	}

	// fn
	inline Expr(Op _op, Fn *_lhs) : op(_op), lhs(NULL), rhs(NULL), fn(_lhs)
	{
	}

	inline ~Expr()
	{
		delete lhs;
		delete rhs;
		delete fn;
	}

	inline Expr(const Expr &other)
	{
		op = other.op;
		if (other.lhs)
			lhs = new Expr(*other.lhs);
		if (other.rhs)
			rhs = new Expr(*other.rhs);
		if (other.fn)
			fn = new Fn(*other.fn);
	}

	inline Expr &operator=(Expr other)
	{
		std::swap(op, other.op);
		std::swap(lhs, other.lhs);
		std::swap(rhs, other.rhs);
		std::swap(fn, other.fn);
		return *this;
	}

	inline friend std::ostream &operator<<(std::ostream &os, const Expr &expr);

	inline bool check(uint32_t &dim)
	{
		// Task 3 - add your code here
		switch (op) {
		case OP_FN:
			return fn->check(dim);
		case OP_CONJ:
			return lhs->check(dim) && rhs->check(dim);
		case OP_DISJ:
			return lhs->check(dim) && rhs->check(dim);
		case OP_NEG:
			return lhs->check(dim);
		}
		return false;
	}

	inline uint32_t code(uint32_t reg, Writer &writer)
	{
		// Task 5 - add your code here
		uint32_t a, b;
		switch (op) {
		case OP_FN:
			fn->code(reg, writer);
			break;
		case OP_CONJ:
			a = reg = lhs->code(reg, writer);
			b = reg = rhs->code(++reg, writer);
			writer << OP_CONJ << ++reg << a << b;
			break;
		case OP_DISJ:
			a = reg = lhs->code(reg, writer);
			b = reg = rhs->code(++reg, writer);
			writer << OP_DISJ << ++reg << a << b;
			break;
		case OP_NEG:
			a = reg = lhs->code(reg, writer);
			writer << OP_NEG << ++reg << a;
			break;
		}
		return reg;
	}
};


inline std::ostream &operator<<(std::ostream &os, const Expr &expr)
{
	switch (expr.op) {
	case OP_FN:
		return os << *expr.fn;
	case OP_NEG:
		return os << "(!" << *expr.lhs << ")";
	case OP_CONJ:
		return os << "(" << *expr.lhs << " | " << *expr.rhs << ")";
	case OP_DISJ:
		return os << "(" << *expr.lhs << " & " << *expr.rhs << ")";
	default:
		return os;
	}
}

// Abstract Syntax Tree
struct Ast {
	Expr *root;

	Ast(Expr *_root) : root(_root)
	{
	}

	inline ~Ast()
	{
		delete root;
	}

	inline Ast(const Ast &other)
	{
		root = new Expr(*root);
	}

	inline Ast &operator=(Ast other)
	{
		std::swap(root, other.root);
		return *this;
	}

	inline friend std::ostream &operator<<(std::ostream &os, const Ast &ast);

	inline bool check(uint32_t &dim)
	{
		// Task 3 - modify if you can't solve the optional task, see note below.
		// NOTE: dim is used to determine the dimensions of the loop. We can use a constant here (e.g. 128) or the maximum of the x, y and z values of each function.
		return root->check(dim);
	}

	inline void code(uint32_t dim, Writer &writer)
	{
		// Task 5
		writer << OP_LOOP << static_cast<uint32_t>(-1) << 0 << 0 << 0 << dim << dim << dim;
		uint32_t reg = root->code(0, writer);
		writer << OP_DRAW << reg;
		writer << OP_JUMP << 0;
	}
};

inline std::ostream &operator<<(std::ostream &os, const Ast &ast)
{
	return os << *ast.root;
}

Ast parse(FILE *fp);