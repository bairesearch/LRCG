/* convertMeshToLdraw.cpp : Defines the entry point for the console application.
/
/ inputFile = col 1 rows 1-100, col2 rows 1-100 etc
/
/takes in a 100 x 100 mesh, all in a single column, with a new x value occuring every ~100 lines
/ie NUMBER_OF_X_HEIGHT_COORDIANTES_IN_A_ROW_IN_THE_INPUT_FILE
*/

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include "convertMeshToLdraw.h"
#include <time.h>

using namespace std;

#define SIZE_OF_INT 32767.0
	//this is the same as RAND_MAX C++ constant

#define BASE_PLATE_OFFSET 0.0 //default = 0.0 for Matlab terrain gen software 1bXx, default = -0.5 for Matlab terrain gen files 1aXx  //takes into account the 2x2 50x50 baseplates = 100x100 units , and the 101x101 terrain file.

#define CREATING_WATER true			//default = true
#define CREATING_BEACH true			//default = true
#define CREATING_BASEPLATES true	//default = true
#define CREATING_TERRAIN_FILES_WITH_ABSOLUTE_POSITION_COORDINATES true		//default = false
#define CREATING_HILLS true			//default = true
#define PLANTING_TREES true			//default = true

#define MAXIMUM_HEIGHT_OF_MAP 18					//default = 18
#define NUM_COORD_PER_ROW 100						//default = 100 for Matlab terrain gen software 1bXx, default = 101 for Matlab terrain gen files 1aXx , 
#define BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE 8	//default = 8			//aka base plate level height negative (ie the greater the value, the less the height)

#define BEACH_HEIGHT_UNDER_SEA_LEVEL 0.3			//default = 0.3
#define BEACH_HEIGHT_ABOVE_SEA_LEVEL 0.5			//default = 0.5
#define INVERSE_PROBABILITY_OF_PLANTING 0.99		//default = 0.99		//was 0.95

#define SAFE_RADIUS_FOR_PLANTING_TREE_BASE 1		//default = 1
#define SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES 3	//default = 3
#define PROBABILITY_OF_LARGEPINE 0.25				//default = 0.25	- species variation
#define PROBABILITY_OF_FLOWER 0.8					//default = 0.8
#define PROBABILITY_OF_STEM 0.5						//default = 0.5


//for tree planting
int heightArray[NUM_COORD_PER_ROW][NUM_COORD_PER_ROW];
bool restrictedAreaArray[NUM_COORD_PER_ROW][NUM_COORD_PER_ROW];
int sceneXPosition;
int sceneYPosition;
	//true = safe for tree planting / false = unsafe for tree planting

bool creatingWater = CREATING_WATER;
bool creatingBeach = CREATING_BEACH;
bool creatingBasePlates = CREATING_BASEPLATES;
bool creatingTerrainFilesWithAbsolutePositionCoordinates = CREATING_TERRAIN_FILES_WITH_ABSOLUTE_POSITION_COORDINATES;
bool creatingHills = CREATING_HILLS;
bool plantingTrees = PLANTING_TREES;

int main()
{
	//int maxSizeOfArray = (NUM_COORD_PER_ROW) * (NUM_COORD_PER_ROW)
	//short* heightArray[maxSizeOfArray];
	//bool* restrictedAreaArray[maxSizeOfArray];

	//heightArray = (short *)malloc(NUM_COORD_PER_ROW * NUM_COORD_PER_ROW * sizeof(short));
	//restrictedAreaArray = (bool *)malloc(NUM_COORD_PER_ROW * NUM_COORD_PER_ROW * sizeof(bool));


	//for parsing
	char xFileNameString[2] = "";
	char yFileNameString[2] = "";
	char fileNameTypeSection[10] = "";
	char inputFileName[5000] = "";
	char outputFileName[5000] = "";


	for(int x = 0; x < 10; x++)
	{
		for(int y = 0; y < 10; y++)
		{
			xFileNameString[0] = '\0';
			yFileNameString[0] = '\0';
			fileNameTypeSection[0] = '\0';
			inputFileName[0] = '\0';
			outputFileName[0] = '\0';

			_itoa(x, xFileNameString, 10);
			_itoa(y, yFileNameString, 10);


			strcat(fileNameTypeSection, "x");
			strcat(fileNameTypeSection, xFileNameString);
			strcat(fileNameTypeSection, "y");
			strcat(fileNameTypeSection, yFileNameString);


			strcat(inputFileName,  "interpolatedData");
			strcat(inputFileName, fileNameTypeSection );
			strcat(inputFileName, ".txt");


			strcat(outputFileName,  "terrain");
			strcat(outputFileName, fileNameTypeSection);
			strcat(outputFileName, ".dat");

			cout << inputFileName << endl;
			cout << outputFileName << endl;

			sceneXPosition = x;
			sceneYPosition = y;

			parseTextFile(inputFileName, outputFileName);
		}
	}
}




void parseTextFile(char* inputFileName, char* outputFileName)
{
	ofstream outputTerrainFile(outputFileName);
	initiateFileOutput(&outputTerrainFile);

	if(creatingBasePlates)
	{
		placeBasePlate(&outputTerrainFile);
	}

	if(creatingHills)
	{
		ifstream inputTerrainFile(inputFileName);
		fileConversion(&outputTerrainFile, &inputTerrainFile);
		inputTerrainFile.close();
	}
	else
	{
		//Set defaults:
		for(int x=0;x<NUM_COORD_PER_ROW;x++)
		{
			for(int y=0;y<NUM_COORD_PER_ROW;y++)
			{
				heightArray[x][y] = (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE-BEACH_HEIGHT_ABOVE_SEA_LEVEL)*3+2.0;	//takes into account the defined height coordinate of the baseplate
				restrictedAreaArray[x][y] = true;	//all areas are OK for planting trees
			}
		}
	}

	if(plantingTrees)
	{
		plantTrees(&outputTerrainFile);
	}

	finaliseFileOutput(&outputTerrainFile);
	outputTerrainFile.close();
}

void placeBasePlate(ofstream * outputTerrainFile)
{
	char* startOfRow = "1 2 ";

	if(creatingWater)
	{
		startOfRow = "1 1 ";	//blue water
	}
	else
	{
		startOfRow = "1 2 ";	//green grass
	}

	char* endOfRow = " 1 0 0 0 1 0 0 0 1 782.DAT\n";

	double temp = (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL);

	double temp2 = temp*3;
	int zInt = temp2; //this rounds the height value to the nearest Ldraw height value (every 8 units)
	int zLdraw = zInt*8; //this takes the rounded height value and converts it back into a proper Ldraw height value (24 units per level)
	char zCoordString[10];
	_itoa(zLdraw, zCoordString, 10);


	char* spaceString = " ";

	int xLdrawBase = int(double((NUM_COORD_PER_ROW-2)*20.0/4.0));		//default with matlab terrain gen software v1aXX: int xLdrawBase = int(double((NUM_COORD_PER_ROW+BASE_PLATE_OFFSET*6.0)*20.0/4.0));
	if(creatingTerrainFilesWithAbsolutePositionCoordinates)
	{
		xLdrawBase =  xLdrawBase + sceneXPosition*NUM_COORD_PER_ROW*20;
	}

	int yLdrawBase = int(double((NUM_COORD_PER_ROW-2)*20.0/4.0));
	if (creatingTerrainFilesWithAbsolutePositionCoordinates)
	{
		yLdrawBase =  yLdrawBase + sceneYPosition*NUM_COORD_PER_ROW*20;
	}

	int xLdraw = xLdrawBase;
	for(int x=1;x<3;x++)				//default with matlab terrain gen software v1aXX: for(int y=1;y<3;y++)
	{
		char xCoordString[10];
		_itoa(xLdraw, xCoordString, 10);

		int yLdraw = yLdrawBase;
		for(int y=1;y<3;y++)			//default with matlab terrain gen software v1aXX: for(int y=1;y<3;y++)
		{
			char yCoordString[10];
			_itoa(yLdraw, yCoordString, 10);

			(*outputTerrainFile).write(startOfRow, strlen(startOfRow));
			(*outputTerrainFile).write(xCoordString, strlen(xCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(zCoordString, strlen(zCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(yCoordString, strlen(yCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(endOfRow, strlen(endOfRow));

			yLdraw = yLdraw+y*(NUM_COORD_PER_ROW+(BASE_PLATE_OFFSET*2.0))*20/2.0;
		}
		xLdraw = xLdraw+x*(NUM_COORD_PER_ROW+(BASE_PLATE_OFFSET*2.0))*20/2.0;
	}
}

void plantTrees(ofstream * outputTerrainFile)
{
	plantTreeAlgorithm(outputTerrainFile);
}

void plantTreeAlgorithm(ofstream * outputTerrainFile)
{
	//seeds the random operator to achieve different random values every time the algorithm is run
	srand( (unsigned)time( NULL ) );

	for(int xInt=SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES; xInt< (NUM_COORD_PER_ROW - SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES);xInt++)
	{
		for(int yInt=SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES; yInt< (NUM_COORD_PER_ROW - SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES);yInt++)
		{
			double probabilityOfPlanting = SIZE_OF_INT*INVERSE_PROBABILITY_OF_PLANTING;
			if(rand() > probabilityOfPlanting)
			{
				bool passedChecksForPlanting = true;
				bool passedChecksForPlantingTree = true;

				int currentHeight = heightArray[xInt][yInt];

				//checks neighbors to see if it is suitable to plant a tree here

				for(int x=(xInt-SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES); x<(xInt+SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES); x++)
				{
					for(int y=(yInt-SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES); y<(yInt+SAFE_RADIUS_FOR_PLANTING_ADJACENT_TREES); y++)
					{
						if(restrictedAreaArray[x][y] == false)
						{
							passedChecksForPlanting = false;
							passedChecksForPlantingTree = false;
						}
					}
				}

				if((passedChecksForPlanting == true) && (passedChecksForPlantingTree = true))
				{
					for(int x=(xInt-SAFE_RADIUS_FOR_PLANTING_TREE_BASE);x<(xInt+SAFE_RADIUS_FOR_PLANTING_TREE_BASE);x++)
					{
						for(int y=(yInt-SAFE_RADIUS_FOR_PLANTING_TREE_BASE);y<(yInt+SAFE_RADIUS_FOR_PLANTING_TREE_BASE);y++)
						{
							if(heightArray[x][y] != currentHeight)
							{
								passedChecksForPlantingTree = false;
							}
						}
					}
				}

				if(passedChecksForPlantingTree == true)
				{
					restrictedAreaArray[xInt][yInt] = false;
					int typeOfTree;
					int temp = SIZE_OF_INT*PROBABILITY_OF_LARGEPINE;
					if(rand() > temp)
					{
						typeOfTree = 1;
					}
					else //if(temp > SIZE_OF_INT*PROBABILITY_OF_SMALLPINE)
					{
						typeOfTree = 2;
					}
					addRowPlantPart(outputTerrainFile, xInt, yInt, heightArray[xInt][yInt], typeOfTree);
				}
				else if(passedChecksForPlanting)
				{
					int typeOfTree;
					int temp = rand();
					if(temp > SIZE_OF_INT*PROBABILITY_OF_FLOWER)
					{//flower
						restrictedAreaArray[xInt][yInt] = false;
						typeOfTree = 5;
						addRowPlantPart(outputTerrainFile, xInt, yInt, heightArray[xInt][yInt], typeOfTree);
					}
					else if(temp > SIZE_OF_INT*PROBABILITY_OF_STEM)
					{//stem only
						restrictedAreaArray[xInt][yInt] = false;
						typeOfTree = 4;
						addRowPlantPart(outputTerrainFile, xInt, yInt, heightArray[xInt][yInt], typeOfTree);
					}
					else
					{
						//do not plant shrub
					}
				}
				else
				{

				}
			}
			else
			{
				/*
				cout << "rand() = " << rand() << endl;
				cout << "probabilityOfPlanting = " << probabilityOfPlanting << endl;
				cout << "sizeof(int) = " << sizeof(int) << endl;
				exit(0);
				*/
			}
		}
	}

}

void createColourString(int colour, char* colourString)
{
	if(colour == 1)
	{//green
		//colourString = "1";
		colourString[0] = '1';
		colourString[1] = '\0';
	}
	else if(colour == 2)
	{//green
		//colourString = "2";
		colourString[0] = '2';
		colourString[1] = '\0';
	}
	else if(colour == 4)
	{//red
		//colourString = "4";
		colourString[0] = '4';
		colourString[1] = '\0';
	}
	else if(colour == 14)
	{//yellow
		//colourString = "14";
		colourString[0] = '1';
		colourString[1] = '4';
		colourString[2] = '\0';
	}
	else
	{
		//Default colour - colour not specified
		//colourString = "16";
		colourString[0] = '1';
		colourString[1] = '6';
		colourString[2] = '\0';
	}
}

void addRowPlantPart(ofstream * outputTerrainFile, int xInt, int yInt, int zInt, int typeOfTree)
{
	char plantPartString[12] = "";
	int xyModifier;		//takes into account central position of part
	int zModifier;		//takes into account central position of part

	int colour;
	if(typeOfTree == 1)
	{//pine
		strcat(plantPartString, "3471.dat");
		colour = 2;
		xyModifier = -10;
		zModifier = -8;
	}
	else if(typeOfTree == 2)
	{//small pine
		strcat(plantPartString, "2435.dat");
		colour = 2;
		xyModifier = -10;
		zModifier = -8;
	}
	else if(typeOfTree == 4)
	{//stem
		strcat(plantPartString, "3741.dat");
		colour = 2;
		xyModifier = 0;
		zModifier = -8;
	}
	else if(typeOfTree == 5)
	{//flower
		strcat(plantPartString, "FLOWERS.dat");
		int temp = rand()/SIZE_OF_INT*3.0;		//= random number between 0 and sizeOfInt / sizeOfInt * 3
		if(temp == 0)
		{
			colour = 1;
			//blue
		}
		else if(temp == 1)
		{
			colour = 4;
			//red
		}
		else
		{
			colour = 14;
		}
		xyModifier = 0; //-10;
		zModifier = 0;
	}

	char colourString[5];
	createColourString(colour, colourString);

	char* startOfRow = "1 ";
	char* middleOfRow = " 1 0 0 0 1 0 0 0 1 ";
	char* endOfRow = "\n";

	int zLdraw = zInt*8 + zModifier; //this takes the rounded height value and converts it back into a proper Ldraw height value (24 units per level)
	char zCoordString[10];
	_itoa(zLdraw, zCoordString, 10);

	int xLdraw = xInt*20 + xyModifier;
	if(creatingTerrainFilesWithAbsolutePositionCoordinates)
	{
		xLdraw = xLdraw + sceneXPosition*NUM_COORD_PER_ROW*20;
	}
	char xCoordString[10];
	_itoa(xLdraw, xCoordString, 10);

	int yLdraw = yInt*20 + xyModifier;
	if(creatingTerrainFilesWithAbsolutePositionCoordinates)
	{
		yLdraw = yLdraw + sceneYPosition*NUM_COORD_PER_ROW*20;
	}
	char yCoordString[10];
	_itoa(yLdraw, yCoordString, 10);

	char* spaceString = " ";

	(*outputTerrainFile).write(startOfRow, strlen(startOfRow));
	(*outputTerrainFile).write(colourString, strlen(colourString));
	(*outputTerrainFile).write(spaceString, strlen(spaceString));
	(*outputTerrainFile).write(xCoordString, strlen(xCoordString));
	(*outputTerrainFile).write(spaceString, strlen(spaceString));
	(*outputTerrainFile).write(zCoordString, strlen(zCoordString));
	(*outputTerrainFile).write(spaceString, strlen(spaceString));
	(*outputTerrainFile).write(yCoordString, strlen(yCoordString));
	(*outputTerrainFile).write(middleOfRow, strlen(middleOfRow));
	(*outputTerrainFile).write(plantPartString, strlen(plantPartString));
	(*outputTerrainFile).write(endOfRow, strlen(endOfRow));
}




void initiateFileOutput(ofstream * outputTerrainFile)
{
	initiateOutputTerrainFile(outputTerrainFile);
}
void finaliseFileOutput(ofstream * outputTerrainFile)
{
	finaliseOutputTerrainFile(outputTerrainFile);
}

void initiateOutputTerrainFile(ofstream * outputTerrainFile)
{
	ifstream terrainEntryTextFile("terrainEntry.txt");
	while (terrainEntryTextFile.good())
   	{
		(*outputTerrainFile).put((char)terrainEntryTextFile.get());
	}
	(*outputTerrainFile).put('\n');
	terrainEntryTextFile.close();
}

void finaliseOutputTerrainFile(ofstream * outputTerrainFile)
{
	/*
	ifstream terrainExitTextFile("terrainExit.txt");
	while (terrainExitTextFile.good())
   	{
		(*outputTerrainFile).put((char)terrainExitTextFile.get());
	}
	*/
	(*outputTerrainFile).put('0');
}

void fileConversion(ofstream * outputTerrainFile, ifstream * inputTerrainFile)
{
	/*
	NB heightCoordinate definitions used in this code are defined matlab coords

	Matlab to Ldraw Coord
	X -> X
	Y -> Z
	Z -> Y
	*/

	int x = 0;
	int y = 0;

	bool readingHeightCoordinate = true;

	char heightCoordinate[10] = "";

	char c;

	int inputLineNum = 0;

   	while ((*inputTerrainFile).get(c))
	{
		y = inputLineNum/(NUM_COORD_PER_ROW);
		x = inputLineNum%(NUM_COORD_PER_ROW);

		//y = inputLineNum/100.0;
		//x = inputLineNum%100;

		if((readingHeightCoordinate) && (c != '\n'))
		{
			char typeString[2];
			typeString[0] = c;
			typeString[1] = '\0';
			strcat(heightCoordinate, typeString);
		}
		else if((readingHeightCoordinate) && (c == '\n'))
		{//new line
			//cout << "new line";
			//output heightCoordinate value into file:
			addRowGroundPart(outputTerrainFile, x, y, heightCoordinate);

			//reset heightCoordinate string
			heightCoordinate[0] = '\0';
			inputLineNum++;
		}
		else
		{
			//Error in converting File
			cout << "Error in converting File" << endl;
		}
	}
}

void addRowGroundPart(ofstream * outputTerrainFile, int xInt, int yInt, char* heightCoordinate)
{
	char* startOfRow = "1 2 ";
	char* endOfRow = " 1 0 0 0 1 0 0 0 1 3005.DAT\n";

	double temp = atof(heightCoordinate)*MAXIMUM_HEIGHT_OF_MAP;

	if(!creatingBasePlates)
	{
		if(creatingWater)
		{
			if(temp > (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL))
			{//> is used instead of < due to the way the y/height coordinate system is defined in ldraw
				temp = BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL;
				startOfRow = "1 1 ";
				restrictedAreaArray[xInt][yInt] = false;
					//unsafe for tree planting	- water
			}
			else if(temp > (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE-BEACH_HEIGHT_ABOVE_SEA_LEVEL))
			{//Potential beach
				if(creatingBeach)
				{
					startOfRow = "1 14 ";
					restrictedAreaArray[xInt][yInt] = false;
						//unsafe for tree planting	- sand
				}
				else
				{
					startOfRow = "1 2 ";
					restrictedAreaArray[xInt][yInt] = true;
						//safe for tree planting	- grass
				}

			}
			else
			{
				startOfRow = "1 2 ";
				restrictedAreaArray[xInt][yInt] = true;
					//safe for tree planting	- grass
			}
		}
		else
		{
			if(temp > (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL))
			{//> is used instead of < due to the way the y/height coordinate system is defined in ldraw
				temp = BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL;
				startOfRow = "1 2 ";
				restrictedAreaArray[xInt][yInt] = true;
					//safe for tree planting	- grass
			}
			else
			{
				startOfRow = "1 2 ";
				restrictedAreaArray[xInt][yInt] = true;
					//safe for tree planting	- grass
			}
		}

			double temp2 = temp*3;
			int zInt = temp2; //this rounds the height value to the nearest Ldraw height value (every 8 units)
			int zLdraw = zInt*8; //this takes the rounded height value and converts it back into a proper Ldraw height value (24 units per level)
			char zCoordString[10];
			_itoa(zLdraw, zCoordString, 10);

			int xLdraw = xInt*20;
			if(creatingTerrainFilesWithAbsolutePositionCoordinates)
			{
				 xLdraw =  xLdraw + sceneXPosition*NUM_COORD_PER_ROW*20;
			}
			char xCoordString[10];
			_itoa(xLdraw, xCoordString, 10);

			int yLdraw = yInt*20;
			if(creatingTerrainFilesWithAbsolutePositionCoordinates)
			{
				 yLdraw =  yLdraw + sceneYPosition*NUM_COORD_PER_ROW*20;
			}
			char yCoordString[10];
			_itoa(yLdraw, yCoordString, 10);

			heightArray[xInt][yInt] = zInt;

			char* spaceString = " ";

			(*outputTerrainFile).write(startOfRow, strlen(startOfRow));
			(*outputTerrainFile).write(xCoordString, strlen(xCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(zCoordString, strlen(zCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(yCoordString, strlen(yCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(endOfRow, strlen(endOfRow));
	}
	else
	{//creating Baseplates.

		if(temp > (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL))
		{//> is used instead of < due to the way the y/height coordinate system is defined in ldraw
			//temp = BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE + BEACH_HEIGHT_UNDER_SEA_LEVEL;

			if(creatingWater)
			{
				//startOfRow = "1 1 ";
				restrictedAreaArray[xInt][yInt] = false;
			}
			else
			{
				//startOfRow = "1 2 ";
				restrictedAreaArray[xInt][yInt] = true;
			}
			heightArray[xInt][yInt] = (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE-BEACH_HEIGHT_ABOVE_SEA_LEVEL)*3+2.0;
		}
		else
		{
			if(temp > (BASEPLATE_OR_SEA_LEVEL_HEIGHT_NEGATIVE-BEACH_HEIGHT_ABOVE_SEA_LEVEL))
			{//potential beach
				if(creatingWater && creatingBeach)
				{
					startOfRow = "1 14 ";	//sand
					restrictedAreaArray[xInt][yInt] = false;
						//unsafe for tree planting	- sand
				}
				else
				{
					startOfRow = "1 2 ";	//grass
					restrictedAreaArray[xInt][yInt] = true;
				}
			}
			else
			{//dry land
					startOfRow = "1 2 ";	//grass
					restrictedAreaArray[xInt][yInt] = true;
			}

			double temp2 = temp*3;
			int zInt = temp2; //this rounds the height value to the nearest Ldraw height value (every 8 units)
			int zLdraw = zInt*8; //this takes the rounded height value and converts it back into a proper Ldraw height value (24 units per level)
			char zCoordString[10];
			_itoa(zLdraw, zCoordString, 10);

			int xLdraw = xInt*20;
			if(creatingTerrainFilesWithAbsolutePositionCoordinates)
			{
				xLdraw =  xLdraw + sceneXPosition*NUM_COORD_PER_ROW*20;
			}
			char xCoordString[10];
			_itoa(xLdraw, xCoordString, 10);

			int yLdraw = yInt*20;
			if(creatingTerrainFilesWithAbsolutePositionCoordinates)
			{
				yLdraw =  yLdraw + sceneYPosition*NUM_COORD_PER_ROW*20;
			}
			char yCoordString[10];
			_itoa(yLdraw, yCoordString, 10);

			heightArray[xInt][yInt] = zInt;

			char* spaceString = " ";

			(*outputTerrainFile).write(startOfRow, strlen(startOfRow));
			(*outputTerrainFile).write(xCoordString, strlen(xCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(zCoordString, strlen(zCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(yCoordString, strlen(yCoordString));
			(*outputTerrainFile).write(spaceString, strlen(spaceString));
			(*outputTerrainFile).write(endOfRow, strlen(endOfRow));
		}
	}
}


