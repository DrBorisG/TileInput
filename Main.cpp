//A Goal: Draw a rectangle and put a sprite on it
//Stretch - think about how this would be done variably. 
//Heros are a special and complex case who have many more frames than 
//NPCs or enemies.

/* [02:30] <@DrBorisG> the job order looks something like
[02:31] <@DrBorisG> 1) load and convert a sprite sheet from a surface to a texture
[02:31] <@DrBorisG> 2) set up accompanying clip data
[02:32] <@DrBorisG> 3) render a big black background rectangle
[02:32] <@DrBorisG> 4) render sprite with rendercopy + clip data
[02:33] <@DrBorisG> +chroma key transperency too I guess might as well set it all up at once
[02:33] <@DrBorisG> 5) push all of that to the visible screen with renderclear / renderupdate */


////////////////////////////////////////////////////// NO CODE ABOVE THIS LINE /////////////////////////////////////////////////

#include <stdio.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>

class map //The game is designed around maps, 16x16 tile collections. I felt a good thing to do might be to load the one the user is currently on AND adjacent rooms for fast swapping.
{			//Thus the map function is generically set up to let you load any map, and hopefully it either natively does this or I can write a function to quickly assign one instance of map to another.
public:			//everything is public, this class is for cleaner code and convenience, not others using it.
	uint16_t x; //Store the x and y size right in the map so the main loop doesn't have to keep track after the first time.
	uint16_t y;
	uint16_t mapNum; //the particular map's number (may be useful for auto-calculating adjacent maps? What about non-adjacent map teleports?)
	uint16_t * visArray; //arrays for the four tile layers. 
	uint16_t * behArray;
	uint16_t * bakArray;
	uint16_t * forArray; 

	void populateMap(uint16_t inNum, uint16_t inX, uint16_t inY)
	{
		//declarations
		char fileName[21]; //map####.csv
		FILE * mapFile; //only used for doing the intial dump to arrays, but has to be here and 'cheap' to hold on to really.
		char * buffer; //buffer in lines from the map csv file.
		char * tokbuffer; //a pointer for tokenizing the map buffer
		uint32_t xCurrent=0; // keeping track of tiles when breaking out the CSVs.
		uint16_t yCurrent=0; //how many rows have actively been gone through in the process. Initialized to 0
		int walkArray = 0;


		//code
		mapNum = inNum; //computationally cheap to make sure this number just sticks with the instance of the class from now on.
		x = inX; //I can see needing to know the map size a lot, letting the class hold it is process-cheap and useful for me as the programmer.
		y = inY; //in an ideal world we could somehow unpack map x and y from mapnum too, rather than passing from main loop, but I'll figure that out later once I have it working.
		uint16_t holder = x * 4; //assumeing the max case of 4 character per map tile.
		buffer = (char*) malloc(holder); //if I did this right, should be change buffer to be a pointer pointing at holder #bytes of memory.
		holder = x * y ; //map width * height
		visArray = (uint16_t*) malloc(holder); //does malloc correctly cast to uint16? If not I can always add a *2 to that holder assignment above
		behArray = (uint16_t*) malloc(holder);
		bakArray = (uint16_t*) malloc(holder);
		forArray = (uint16_t*) malloc(holder);
		sprintf(fileName, "resources/map%04d.csv", mapNum); //make a single string out of the map number, prepends zeroes if the number isn't four digits.
		mapFile = fopen(fileName, "rb"); //open the file

		
		while (yCurrent < y) //until we get through building the visible layer array, basically
		{
			fgets(buffer, x*4, mapFile); //buffer should be a string of format: ###,###,###,...,###\n     //note that this includes the possibility that it's #,#,#,...,#\n
			tokbuffer = strtok(buffer, ","); //break the buffer down by commas
			while (xCurrent < x) //while on a row
			{
				visArray[walkArray] = atoi(tokbuffer); //turn one parsed string token into an int, store it on the visible layer array
				walkArray++; xCurrent++; //step array position and current x position
				tokbuffer = strtok(NULL, ","); //step to the next token
			}
			yCurrent++; //step a vertical row
			xCurrent = 0;
		}
		walkArray = 0; //reset array position tracking for the next array

		while (yCurrent < y*2) //building the behavior array
		{
			fgets(buffer, x * 4, mapFile); //buffer should be a string of format: ###,###,###,...,###\n     //note that this includes the possibility that it's #,#,#,...,#\n
			tokbuffer = strtok(buffer, ","); //break the buffer down by commas
			while (xCurrent < x) //while on a row
			{
				behArray[walkArray] = atoi(tokbuffer); //turn one parsed string token into an int, store it on the behavior layer array
				walkArray++; xCurrent++; //step array position and current x position
				tokbuffer = strtok(NULL, ","); //step to the next token
			}
			yCurrent++; //step a vertical row
			xCurrent = 0;
		}
		walkArray = 0; //reset array position tracking for the next array

		while (yCurrent < y*3) //background
		{
			fgets(buffer, x * 4, mapFile); //buffer should be a string of format: ###,###,###,...,###\n     //note that this includes the possibility that it's #,#,#,...,#\n
			tokbuffer = strtok(buffer, ","); //break the buffer down by commas
			while (xCurrent < x) //while on a row
			{
				bakArray[walkArray] = atoi(tokbuffer); //turn one parsed string token into an int, store it on the background layer array
				walkArray++; xCurrent++; //step array position and current x position
				tokbuffer = strtok(NULL, ","); //step to the next token
			}
			yCurrent++; //step a vertical row
			xCurrent = 0;
		}
		walkArray = 0; //reset array position tracking for the next array

		while (yCurrent < y*4) //foreground overlay
		{
			fgets(buffer, x * 4, mapFile); //buffer should be a string of format: ###,###,###,...,###\n     //note that this includes the possibility that it's #,#,#,...,#\n
			tokbuffer = strtok(buffer, ","); //break the buffer down by commas
			while (xCurrent < x) //while on a row
			{
				forArray[walkArray] = atoi(tokbuffer); //turn one parsed string token into an int, store it on the foreground layer array
				walkArray++; xCurrent++; //step array position and current x position
				tokbuffer = strtok(NULL, ","); //step to the next token
			}
			yCurrent++; //step a vertical row
			xCurrent = 0;
		}

		free(buffer); //give the memory from the dynamic buffer back
		fclose(mapFile); //politely close the read from the file

		return; 
		
	}

	void destroymap()
	{
		free(visArray);
		free(behArray);
		free(bakArray);
		free(forArray);
		return;
		//how do you destroy the int16s?
	}

};



/*




//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture(std::string path);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
		{
			printf("Something went wrong with nearest pixel mode");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load PNG texture
	gTexture = loadTexture("resources/korbin.png");
	if (gTexture == NULL)
	{
		printf("Failed to load texture image!\n");
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded image
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* loadTexture(std::string path)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}


*/
//commented all the SDL stuff momentarily

int main(int argc, char* args[])

{

	uint16_t xfeed = 50; 
	uint16_t yfeed = 33;
	uint16_t mapnumfeed = 1;
	uint16_t readout = 0;
	map mainMap;

	mainMap.populateMap(mapnumfeed, xfeed, yfeed);
	
	readout = mainMap.visArray[0];
	readout = mainMap.behArray[0];
	readout = mainMap.bakArray[0];
	readout = mainMap.forArray[0];
	readout = mainMap.visArray[0];

	xfeed = 0;

	


/* {
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
				}

				//Clear screen
				SDL_RenderClear(gRenderer);

				//Render texture to screen
				SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

*/
	return 0;
}