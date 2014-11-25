

#define maxChildren 8

typedef uint16_t nodeIndex;
#define  maxNodeIndex  0xFFFF

typedef double  number;
typedef uint8_t byte;

#define _arrayType_(name, elemType)\
typedef struct {\
	elemType  *data;\
	uint64_t   dataSpace;\
	uint64_t   dimenX;\
	uint32_t   dimenY;\
	uint32_t   dimenZ;\
} name;
_arrayType_(numArray,  number)
_arrayType_(byteArray, byte)
_arrayType_(nodeArray, nodeIndex)
typedef union {
	number      n;
	number      n2[2];
	number      n3[3];
	number      n4[4];
	numArray    N;
	byte        b;
	byte        b2[2];
	byte        b4[4];
	byte        b8[8];
	byte        b16[16];
	byteArray   B;
	nodeIndex   f;
	nodeArray   F;
} outType;
#define _evalargs_ nodeIndex self, outType fnCallArgs[maxChildren]
typedef outType (*evaluator) (_evalargs_);
typedef struct {
	nodeIndex  definition;//for variable/state/fn calls
	int8_t     argRefIndex;//for argument calls
	int8_t     childCount;//the number of "subnodes", defNodes have 1
	nodeIndex  children[maxChildren];
	evaluator  evaluate;
	outType    cache;
} node;

//for non-performance critical info not needed beyond parsing
typedef struct {
	char    *name; //includes type information and parameter names
	uint32_t line; //this node was found on this line in the source file
	uint8_t  level;//elevation+fold
	int16_t  frameform;//-1 if outside not in frameform
	uint8_t  arity;//for fnDef only
} nodeInfo;
nodeInfo *nodesInfo;


#define maxTokenLength 64
#define maxLineLength 256

typedef struct {
	char       name[maxLineLength];
	uint32_t   line;
	int16_t    curStateNode;
	int16_t    stateNodeSpace;
	nodeIndex *stateNodes;
	outType   *hotState;
	int16_t    curRootNode;
	int16_t    rootNodeSpace;
	nodeIndex *rootNodes;
	//per-frameform outputs
	nodeIndex nextFrameform;
	nodeIndex videoOut;
	nodeIndex audioOut;
} frameform;
char *nextFrameformName = "next num";
char *videoOutName      = "videoOut byte4''";
char *audioOutName      = "audioOut num'";


node      *nodes;
nodeIndex *gRootNodes;
frameform *frameforms;
int        curNode      = -1;
int        gCurRootNode = -1;
int        curFrameform = -1;


typedef struct {
	char     *name;
	uint8_t   arity;
	evaluator evaluate;
} stdNode;

#define stdNodeTableLength  30
const stdNode *stdNodeTable[stdNodeTableLength];



//global outputs
double frameRate    = defaultFrameRate;
int    windowWidth  = defaultWindowWidth;
int    windowHeight = defaultWindowHeight;
nodeIndex frameRateRoot    = maxNodeIndex;
nodeIndex windowWidthRoot  = maxNodeIndex;
nodeIndex windowHeightRoot = maxNodeIndex;
char *frameRateName    = "frameRate num";
char *windowWidthName  = "windowWidth num";
char *windowHeightName = "windowHeight num";



#define _output_(toBeEvaluated, fnCallArgs)\
	nodes[toBeEvaluated].evaluate(toBeEvaluated, fnCallArgs);

outType eval_varDef(_evalargs_) {
	return nodes[self].cache;
}
outType eval_varCall(_evalargs_) {
	return nodes[ nodes[self].definition ].cache;
}

outType eval_fnDef(_evalargs_) {
	return _output_(self+1, fnCallArgs)
}
outType eval_fnDefN(_evalargs_) {
	return _output_(self+1, fnCallArgs)
}

outType eval_fnCall(_evalargs_) {
	outType newFnCallArgs[maxChildren];
	for (int i = 0; i < nodes[self].childCount; i++) {
		nodeIndex arg = nodes[self].children[i];
		newFnCallArgs[i] = nodes[arg].evaluate(arg, fnCallArgs);
	}
	nodeIndex fnBody = nodes[self].definition + 1;
	return _output_(fnBody, newFnCallArgs)
}
outType eval_fnCallN(_evalargs_) {
	nodeIndex fnBody = nodes[self].definition + 1;
	return _output_(fnBody, fnCallArgs)
}

outType eval_argCall(_evalargs_) {
	return fnCallArgs[ nodes[self].argRefIndex ];
}
outType eval_fnArgCall(_evalargs_) {
	nodeIndex nodePassed = fnCallArgs[ nodes[self].argRefIndex ].f;
	
	//std fn
	if (nodePassed > curNode)
		return stdNodeTable[nodePassed-curNode-1]->evaluate(self, fnCallArgs);
	
	//user-defined fn
	outType newFnCallArgs[maxChildren];
	for (int i = 0; i < nodes[self].childCount; i++) {
		nodeIndex arg = nodes[self].children[i];
		newFnCallArgs[i] = nodes[arg].evaluate(arg, fnCallArgs);
	}
	return _output_(nodePassed+1, newFnCallArgs)
}
outType eval_fnPass(_evalargs_) {
	return nodes[self].cache;
}


outType eval_stateDef(_evalargs_) {
	return nodes[self].cache;
}
outType eval_shareDef(_evalargs_) {
	return nodes[self].cache;
}
outType eval_stateCall(_evalargs_) {
	return nodes[ nodes[self].definition ].cache;
}
outType eval_shareCall(_evalargs_) {
	return nodes[ nodes[self].definition ].cache;
}


outType eval_outDef(_evalargs_) {
	return nodes[self].cache;
}

char *name_frameformRef = "frameformRef num";
outType eval_frameformRef(_evalargs_) {
	return nodes[self].cache;
}

char *name_numLit = "numLit num";
outType eval_numLit(_evalargs_) {
	return nodes[self].cache;
}







#define readOnlyFlag 0x80000000

void setToReadOnly(outType *a) {
	a->B.dimenZ |= readOnlyFlag;
}

void setToWriteable(outType *a) {
	a->B.dimenZ &= ~readOnlyFlag;
}

bool isReadOnly(outType a) {
	return a.B.dimenZ & readOnlyFlag;
}

uint32_t dimenZ(outType a) {
	return a.B.dimenZ & ~readOnlyFlag;
}









