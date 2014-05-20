

FILE *fileStream;
char  fileChar;
#define maxTokenLength 80
char  tokenBuf[maxTokenLength];
uint  expectedIndentation;
uint  currentLine = 1;

int   reachedEOF = 0;
int   noErrors = 1;

uint  currentNode = 0;
#define rootNodePageSize 20
uint  rootNodes[rootNodePageSize];
uint  currentRootNode = 0;

#define commentChar  '/'


int  lookupNode(char *nameIn) {
	uint i=0;
	for (; i<stdNodeTableLength; i++)
		if (!( strcmp(stdNodeTable[i].name, nameIn) ))
			return i;
	noErrors = 0;
	return -1;
}

void  getNode() {
	
	uint tokenCharIndex = 0;
	for (;; tokenCharIndex++) {
		if (tokenCharIndex == maxTokenLength) {
			noErrors = 0;
			printf(
				"error: token at line %d exceeds maxTokenLength\n", 
				currentLine
			);
			return;
		}
		
		fileChar = fgetc(fileStream);
		
		if (!tokenCharIndex  &&  fileChar == ' ') {
			noErrors = 0;
			printf("error: leading space at line %d\n", currentLine);
			return;
		}
		else if (fileChar == EOF) {
			reachedEOF = 1;
			if (expectedIndentation > 0) {
				noErrors = 0;
				printf("error: file ended unexpectadly at line %d\n", currentLine);
			}
			return;
		}
		else if (fileChar == '\t') {
			noErrors = 0;
			printf("error: line %d is over-indented\n", currentLine);
			return;
		}
		else if (fileChar == '\n'  ||  fileChar == commentChar) {
			//terminate the token
			tokenBuf[tokenCharIndex] = '\0';
			
			//remove trailing whitespace, if any
			if (tokenCharIndex > 1) {
				uint backstep = 1;
				while (tokenBuf[tokenCharIndex-backstep] == ' ') {
					tokenBuf[tokenCharIndex-backstep] = '\0';
					backstep++;
				}
			}
			
			//if we hit a comment, read through the rest of the comment
			if (fileChar == commentChar) {
				for (;; tokenCharIndex++) {
					fileChar = fgetc(fileStream);
					if (fileChar == '\n')
						break;
					if (fileChar == EOF) {
						reachedEOF = 1;
						if (expectedIndentation > 0) {
							noErrors = 0;
							printf("error: file ended unexpectadly at line %d\n", currentLine);
						}
						return;
					}
				}
			}
			currentLine++;
			break;
		}
		else tokenBuf[tokenCharIndex] = fileChar;
	}
	
	//if the line was blank or commented...
	if (tokenBuf[0] == '\0') {
		//if it was not at the root level
		if (expectedIndentation > 0) {
			noErrors = 0;
			printf("error: expected an argument at line %d\n", currentLine-1);
		}
		return;
	}
	
	
	if (tokenBuf[0] >= '0'  &&  tokenBuf[0] <= '9') {
		//make sure it's a valid number
		for (
			tokenCharIndex=0; 
			tokenCharIndex < maxTokenLength; 
			tokenCharIndex++
		) {
			if (tokenBuf[tokenCharIndex] == '\0')
				break;
			if (
				(
					tokenBuf[tokenCharIndex] < '0' || 
					tokenBuf[tokenCharIndex] > '9'
				) && tokenBuf[tokenCharIndex] != '.'
			) {
				noErrors = 0;
				printf(
					"error: invalid number literal '%s' at line %d\n",
					tokenBuf, currentLine-1
				);
				return;
			}
		}
		
		nodes[currentNode] = node_numLit;
		sscanf(tokenBuf, "%lf", &nodes[currentNode].output.n);
		currentNode++;
		// reallocate the nodes to a larger array if necessary
		return;
	}
	
	int nodeToGet = lookupNode(tokenBuf);
	if (nodeToGet == -1) {
		noErrors = 0;
		printf(
			"error: '%s' at line %d is not recognized\n", 
			tokenBuf, currentLine
		);
		return;
	}
	
	nodes[currentNode] = stdNodeTable[nodeToGet];
	
	
	// get arguments, if any
	if (nodes[currentNode].arity) {
		uint parentIndex = currentNode;
		currentNode++;
		expectedIndentation++;
		uint indentation = 0;
		uint currentArg = 0;
		for (; currentArg < nodes[parentIndex].arity; currentArg++) {
			indentation = 0;
			while (indentation < expectedIndentation) {
				fileChar = fgetc(fileStream);		
				if (fileChar == '\t') 
					indentation++;
				else if (fileChar == EOF) {
					reachedEOF = 1;
					printf("error: file ended unexpectadly at line %d\n", currentLine);
					return;
				}
				else {
					printf("error: line %d, under-indented\n", currentLine);
					return;
				}
			}
			nodes[parentIndex].arguments[currentArg] = currentNode;
			getNode();
			//check that the node is of the requiredType
		}
		expectedIndentation--;
	}
	else currentNode++;
	
	// reallocate the nodes to a larger array if necessary
}

