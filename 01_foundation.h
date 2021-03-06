

#define maxChildren 8

typedef uint16_t nodeIndex;
#define maxNodeIndex  0xFFFF

typedef float   number;
typedef uint8_t byte;

typedef struct {
	number  *data;
	size_t   dataSize; //in bytes
	uint64_t dimenX;   //in elements
	uint32_t dimenY;
	uint32_t dimenZ;
} numArray;
typedef struct {
	byte    *data;
	size_t   dataSize;
	uint64_t dimenX;
	uint32_t dimenY;
	uint32_t dimenZ;
} byteArray;

typedef union {
	number      n;
	number      nt[8];
	numArray    N;
	byte        b;
	byte        bt[8];
	uint32_t    i; //just for byte quartets
	byteArray   B;
	nodeIndex   f;
} outType;
#define _evalargs_  nodeIndex self, int16_t taskPiece, outType *fnCallArgs
typedef outType (*evaluator) (_evalargs_);
typedef struct {
	nodeIndex  children[maxChildren];
	int8_t     childCount;  //the number of "subnodes"
	int8_t     argRefIndex; //for argument calls
	nodeIndex  def;         //for variable/state/fn calls
	outType    cache;
	evaluator  evaluate;
} node;

//for non-performance critical info not needed beyond parsing (ideally)
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
char *nextFrameformName = "next N1";
char *videoOutName      = "videoOut B4D2";
char *audioOutName      = "audioOut N1'";


node      *nodes;
nodeIndex *gRootNodes;
frameform *frameforms;
int        curNode      = -1;
int        gCurRootNode = -1;
int        curFrameform = -1;


typedef struct {
	char     *name;
	int8_t    arity;
	evaluator evaluate;
} stdNode;

const stdNode *stdNodeTable[];



//global outputs
double frameRate    = -1;
int    videoWidth   = 0;
int    videoHeight  = 0;
int    windowWidth  = 0;
int    windowHeight = 0;
nodeIndex frameRateRoot    = maxNodeIndex;
nodeIndex videoWidthRoot   = maxNodeIndex;
nodeIndex videoHeightRoot  = maxNodeIndex;
nodeIndex windowWidthRoot  = maxNodeIndex;
nodeIndex windowHeightRoot = maxNodeIndex;
char *frameRateName    = "frameRate N1";
char *videoWidthName   = "videoWidth N1";
char *videoHeightName  = "videoHeight N1";
char *windowWidthName  = "windowWidth N1";
char *windowHeightName = "windowHeight N1";


#define output(self, taskPiece, fnCallArgs)\
	nodes[self].evaluate(self, taskPiece, fnCallArgs);


outType eval_varDef(_evalargs_) {
	return nodes[self].cache;
}
outType eval_varCall(_evalargs_) {
	return nodes[ nodes[self].def ].cache;
}

outType eval_fnDef(_evalargs_) {
	return output(self+1, taskPiece, fnCallArgs);
}
outType eval_fnDefWExargs(_evalargs_) {
	return output(self+1, taskPiece, fnCallArgs);
}
outType eval_fnDefNullary(_evalargs_) {
	return output(self+1, taskPiece, fnCallArgs);
}

outType eval_fnCall(_evalargs_) {
	outType newFnCallArgs[maxChildren];
	for (int i = 0; i < nodes[self].childCount; i++) {
		nodeIndex arg = nodes[self].children[i];
		newFnCallArgs[i] = nodes[arg].evaluate(arg, -1, fnCallArgs);
	}
	return output(nodes[self].def+1, taskPiece, newFnCallArgs);
}
outType eval_fnCallNullary(_evalargs_) {
	return output(nodes[self].def+1, taskPiece, fnCallArgs);
}


outType eval_argCall(_evalargs_) {
	return fnCallArgs[nodes[self].argRefIndex];
}
outType eval_fnArgCall(_evalargs_) {
	nodeIndex nodePassed = fnCallArgs[ nodes[self].argRefIndex ].f;
	
	//std fn
	if (nodePassed > curNode)
		return stdNodeTable[nodePassed-curNode-1]->evaluate(self, -1, fnCallArgs);
	
	//user-defined fn
	outType newFnCallArgs[maxChildren];
	for (int i = 0; i < nodes[self].childCount; i++) {
		nodeIndex arg = nodes[self].children[i];
		newFnCallArgs[i] = nodes[arg].evaluate(arg, -1, fnCallArgs);
	}
	return output(nodePassed+1, taskPiece, newFnCallArgs);
}
outType eval_fnPass(_evalargs_) {
	return nodes[self].cache;
}

outType eval_stateDef(_evalargs_) {
	return nodes[self].cache;
}
outType eval_vistateDef(_evalargs_) {
	return nodes[self].cache;
}
outType eval_stateCall(_evalargs_) {
	return nodes[ nodes[self].def ].cache;
}
outType eval_vistateCall(_evalargs_) {
	return nodes[ nodes[self].def ].cache;
}


outType eval_gOutDef(_evalargs_) {
	return nodes[self].cache;
}

outType eval_outDef(_evalargs_) {
	return nodes[self].cache;
}


char *name_frameformRef = "frameformRef N1";
outType eval_frameformRef(_evalargs_) {
	return nodes[self].cache;
}

char *name_numLit = "numLit N1";
outType eval_numLit(_evalargs_) {
	return nodes[self].cache;
}



typedef struct {
	uint16_t  taken;   // how many threads have started working the task
	uint16_t  finished;// how many threads have finished their piece
	nodeIndex self;
	outType  *fnCallArgs;
} task;

SDL_SpinLock task_lock;
//always lock task_lock before doing anything 
//with anything prefixed with 'task_'

#define taskStackSize 1024
task task_stack[taskStackSize];
int  task_current = -1;

int doATask(void) {
	int       myTask;
	int       taskPiece = -1;
	nodeIndex self;
	outType  *fnCallArgs;
	
	SDL_AtomicLock(&task_lock);
		myTask = task_current;
		for (;; myTask--) {
			if (myTask == -1) {
				SDL_AtomicUnlock(&task_lock);
				return -1;
			}
			if (task_stack[myTask].finished == threadCount)
				task_current--;
			else if (task_stack[myTask].taken < threadCount) {
				taskPiece  = task_stack[myTask].taken;
				self       = task_stack[myTask].self;
				fnCallArgs = task_stack[myTask].fnCallArgs;
				task_stack[myTask].taken++;
				break;
			}
		}
	SDL_AtomicUnlock(&task_lock);
	
	output(self, taskPiece, fnCallArgs);
	
	SDL_AtomicLock(&task_lock);
		task_stack[myTask].finished++;
	SDL_AtomicUnlock(&task_lock);
	
	return 0;
}

outType initTask(nodeIndex self, outType *fnCallargsIn) {
	task toBeAdded = {
		.taken      = 1,
		.finished   = 0,
		.self       = self,
		.fnCallArgs = fnCallargsIn
	};
	int myTask;
	SDL_AtomicLock(&task_lock);
		task_current++;
		myTask = task_current;
		task_stack[task_current] = toBeAdded;
	SDL_AtomicUnlock(&task_lock);
	
	outType toBeReturned = output(self, 0, fnCallargsIn);
	
	SDL_AtomicLock(&task_lock);
		task_stack[myTask].finished++;
	SDL_AtomicUnlock(&task_lock);
	
	while (true) {
		SDL_AtomicLock(&task_lock);
			if (task_stack[myTask].finished == threadCount) {
				SDL_AtomicUnlock(&task_lock);
				break;
			}
		SDL_AtomicUnlock(&task_lock);
		if (doATask()) {
			_threadWait_
		}
	}
	
	return toBeReturned;
}



bool isReadOnly(nodeIndex n) {
	return 
		nodes[n].evaluate == eval_varCall   ||
		nodes[n].evaluate == eval_stateCall ||
		nodes[n].evaluate == eval_vistateCall
	;
}



