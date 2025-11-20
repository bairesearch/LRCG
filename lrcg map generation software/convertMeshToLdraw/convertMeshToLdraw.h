#include <iostream>
#include <fstream>
using namespace std;

/*public short* heightArray; [NUM_COORD_PER_ROW][NUM_COORD_PER_ROW];
public bool* restrictedAreaArray; [NUM_COORD_PER_ROW][NUM_COORD_PER_ROW];*/

int main();

void createColourString(int colour, char* colourString);

void parseTextFile(char* inputFileName, char* outputFileName);
void addRowGroundPart(ofstream * outputTerrainFile, int xInt, int yInt, char* heightCoordinate);
void fileConversion(ofstream * outputTerrainFile, ifstream * inputTerrainFile);

void initiateFileOutput(ofstream * outputTerrainFile);
void initiateOutputTerrainFile(ofstream * outputTerrainFile);
void finaliseFileOutput(ofstream * outputTerrainFile);
void finaliseOutputTerrainFile(ofstream * outputTerrainFile);

void plantTrees(ofstream * outputTerrainFile);
void plantTreeAlgorithm(ofstream * outputTerrainFile);
void addRowPlantPart(ofstream * outputTerrainFile, int xInt, int yInt, int zInt, int typeOfTree);

void placeBasePlate(ofstream * outputTerrainFile);
