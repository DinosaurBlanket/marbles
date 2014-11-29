
outType nullFnCallArgs[maxChildren] = {};

void initializeNodes(void) {
	int initCount = 0;
	//first assign variables defined literally while incrementing initCount
	for (int i = 0; i <= curNode; i++) {
		if (nodes[i].evaluate == eval_varDef || nodes[i].evaluate == eval_gOutDef) {
			if (nodes[i+1].evaluate == eval_numLit)
				nodes[i].cache = nodes[i+1].cache;
			else
				initCount++;
		}
	}
	
	//then evaluate the other variables
	outType *hotDef = malloc(sizeof(outType) * initCount);
	initCount = 0;
	for (int i = 0; i <= curNode; i++) {
		if (
			(nodes[i].evaluate == eval_varDef || nodes[i].evaluate == eval_gOutDef)
			&& nodes[i+1].evaluate != eval_numLit
		) {
			hotDef[initCount] = _output_(i+1, nullFnCallArgs);
			initCount++;
		}
	}
	//then assign everything else
	initCount = 0;
	for (int i = 0; i <= curNode; i++) {
		if (
			(nodes[i].evaluate == eval_varDef || nodes[i].evaluate == eval_gOutDef)
			&& nodes[i+1].evaluate != eval_numLit
		) {
			nodes[i].cache = hotDef[initCount];
			initCount++;
		}
	}
	free(hotDef);
	
	//set global outputs
	if (frameRateRoot <= curNode)
		frameRate = nodes[frameRateRoot].cache.n;
	if (videoWidthRoot <= curNode)
		videoWidth = nodes[videoWidthRoot].cache.n;
	if (videoHeightRoot <= curNode)
		videoHeight = nodes[videoHeightRoot].cache.n;
	if (windowWidthRoot <= curNode)
		windowWidth = nodes[windowWidthRoot].cache.n;
	if (windowHeightRoot <= curNode)
		windowHeight = nodes[windowHeightRoot].cache.n;
	
}



SDL_Window   *window   = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture  *texture  = NULL;

void initializeVideo() {
	SDL_Init(SDL_INIT_VIDEO);
	
	
	
	window = SDL_CreateWindow(
		"marbles",                 // window title
		SDL_WINDOWPOS_UNDEFINED,   // initial x position
		SDL_WINDOWPOS_UNDEFINED,   // initial y position
		windowWidth,               // width, in pixels
		windowHeight,              // height, in pixels
		SDL_WINDOW_OPENGL          // flags
	);
	
	if (window == NULL) {
		errorCount++;
		printf("Could not create window: %s\n", SDL_GetError());
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ABGR8888,
		SDL_TEXTUREACCESS_STREAMING,
		videoWidth, 
		videoHeight
	);
	
	SDL_RenderSetLogicalSize(renderer, videoWidth, videoHeight);
}

