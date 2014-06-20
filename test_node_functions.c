//this is a temporary test to help me figure out how to implement 
//user-defined variables and functions without having to worry 
//about parsing


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "foundation.h"
#include "stdNodeTable.h"




int main(void) {
	
	
	//num  foo
	nodes[0].name = "num  foo  34";	
	nodes[0].evaluate = eval_varDef;
	nodes[0].output.n = 34;
	
	
	
	//fn  double
	nodes[1].name = 
		"fn  double\n"
		"\tnum  in\n"
		"\tis  num";
	nodes[1].arity        = 1;
	nodes[1].arguments[0] = 2;
	nodes[1].evaluate     = eval_fnDef;
	nodes[1].output.n     = 0;
	
	//	mul
	nodes[2] = node_mul;
	nodes[2].arguments[0] = 3;
	nodes[2].arguments[1] = 4;
	
	//		2
	nodes[3] = node_numLit;
	nodes[3].output.n = 2;
	
	//		in
	nodes[4].name = 
		"fn  double\n"
		"\tnum  in\n"
		"\tis  num";
	nodes[4].fnDef       = 1; //the nodeIndex of "double"
	nodes[4].argRefIndex = 0; //the argument index of "in"
	nodes[4].arity       = 0;
	nodes[4].evaluate    = eval_argCall;
	
	
	
	//num  out A
	nodes[5].name         = "num  out A";
	nodes[5].arity        = 1;
	nodes[5].arguments[0] = 6;
	nodes[5].evaluate     = eval_state;
	
	//	double
	nodes[6].name         = "double";
	nodes[6].fnDef        = 1; //the nodeIndex of "double"
	nodes[6].arity        = 1;
	nodes[6].arguments[0] = 7;
	nodes[6].evaluate     = eval_fnCall;
	
	//		foo
	nodes[7].name      = "foo";
	nodes[7].fnDef     = 0; //the nodeIndex of "foo"
	nodes[7].arity     = 0;
	nodes[7].evaluate  = eval_varCall;
	
	
	nodes[5].evaluate(5);//evaluate "out A"
	
	printf("should be 68: %f\n", nodes[5].output.n);
	
	
	return 0;
}


