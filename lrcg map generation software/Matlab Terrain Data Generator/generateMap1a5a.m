%Matlab needs recursion for more than 1 interpolation level...
%Map generation using a random grid and interpolation between that grid
%has 1 level of interpolation due to matlab student edition failure to allow large arrays

MapXRange = 10;
MapYRange = 10;
MapDivisionMax = 0.1;

M = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
   0.0, 0.0, rand, rand, rand, rand, rand, rand, 0.0, 0.0;
   0.0, 0.0, rand, rand, rand, rand, rand, rand, 0.0, 0.0;
   0.0, 0.0, rand, rand, rand, rand, rand, rand, 0.0, 0.0;
   0.0, 0.0, rand, rand, rand, rand, rand, rand, 0.0, 0.0;
   0.0, 0.0, rand, rand, rand, rand, rand, rand, 0.0, 0.0;
   0.0, 0.0, rand, rand, rand, rand, rand, rand, 0.0, 0.0;
   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0];

%original 2005 version matrix: 
%M = [rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand;
%   rand, rand, rand, rand, rand, rand, rand, rand, rand, rand];
   
numInterpolationGrids= 10;
numFineInterpolationGrids=10;

currentInterpolationDivision = MapDivisionMax;
currentXInterpGrid = MapXRange;
currentYInterpGrid = MapYRange;

[xi,yi] = meshgrid(0:MapDivisionMax:MapXRange, 0:MapDivisionMax:MapYRange);
zi = interp2(M,xi,yi, 'cubic');			%original 2005 version interpolation method: zi = interp2(M,xi,yi, 'spline');
mesh(xi,yi,zi);
colormap(cool);

fileNameString = strcat('interpolatedData.txt');	
fid = fopen(fileNameString, 'w');
fprintf(fid, '%1.4f\n', zi);
fclose(fid);   
      
xFileNameCount = 0;
for xInterpGrid = 0:(MapXRange-1)
   
   yFileNameCount = 0;
   for yInterpGrid = 0:(MapYRange-1)
         
      currentFineInterpolationDivision = currentInterpolationDivision / numFineInterpolationGrids;

		%[xiFine,yiFine] = meshgrid(xInterpGrid:currentFineInterpolationDivision:xInterpGrid+currentFineInterpolationDivision*100, yInterpGrid:currentFineInterpolationDivision:yInterpGrid+currentFineInterpolationDivision*100); 
      [xiFine,yiFine] = meshgrid(0:MapDivisionMax:(MapXRange), 0:MapDivisionMax:(MapYRange));
      ziCropped = zi((xInterpGrid*numInterpolationGrids+1):(xInterpGrid*numInterpolationGrids + currentInterpolationDivision*100), (yInterpGrid*numInterpolationGrids+1):(yInterpGrid*numInterpolationGrids+currentInterpolationDivision*100));

      ziFine = interp2(ziCropped, xiFine, yiFine, 'spline');	
  		mesh(xiFine,yiFine,ziFine);
		colormap(cool);
      
      fileNameString = strcat('interpolatedData', 'x', num2str(xFileNameCount), 'y', num2str(yFileNameCount), '.txt');	
		fid = fopen(fileNameString, 'w');
		fprintf(fid, '%1.4f\n', ziFine);
		fclose(fid);
      
      
		yFileNameCount = yFileNameCount + 1;
	end
	xFileNameCount = xFileNameCount + 1;
end
   
   
