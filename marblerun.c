//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "foundation.h"
#include "arithmetic.h"
#include "stdNodeTable.h"
#include "parse.h"



//void freeArgs(node *toBeFreed) {
//	if (toBeFreed->arity) {
//		uint i = 0;
//		for (; i < toBeFreed->arity; i++)
//			freeArgs(&toBeFreed->arguments[i]);
//		free(toBeFreed->arguments);
//	}
//}
//
//void cleanUp(void) {
//	uint i = 0;
//	for (; i<currentRootNode; i++)
//		freeArgs(&rootNodes[i]);
//}


int main(int argc, char **argv) {
	char *fileName = argv[1];
	if (fileName == NULL) {
		puts("usage: marblerun <file name>");
		return 1;
	}
	fileStream = fopen(fileName, "r");
	if (fileStream == NULL) {
		printf("error: could not open '%s'\n\n", fileName);
		return 2;
	}
	
	buildStdNodeTable();
	
	while (1) {
		
		rootNodes[currentRootNode] = currentNode;
		
		getNode();
		if (!noErrors || reachedEOF)
			break;
		
		//if the current rootNode is still "empty", then ignore it,
		//otherwise, we count the root node to be evaluated
		if (strcmp( nodes[rootNodes[currentRootNode]].name, "empty" )) {
			currentRootNode++;
			// reallocate the rootNodes to a larger array if necessary
		}
		
	}
	
	fclose(fileStream);
	
	if (noErrors) {
		puts("results:");
		uint i = 0;
		for (; i<currentRootNode; i++) {
			nodes[rootNodes[i]].evaluate(rootNodes[i]);
			printf("  %2d: %9.3f\n", i, nodes[rootNodes[i]].output.n);
		}
	}
	
	//cleanUp();
	return 0;
}



