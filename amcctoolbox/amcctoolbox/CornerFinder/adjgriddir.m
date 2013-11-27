function gridout=adjgriddir(grid)
% ADJGRIDDIR adjusts the direction of the extracted grid to ensure consistency across all images.
% 
% ADJGRIDDIR examines the direction of the grid rows and columns. It then
% consequently adjusts the grid to a consistent direction.
% 
% USAGE:
%     gridout=adjgriddir(grid);
% 
% INPUTS:
%     grid: MxNx2 array, output of FILTERGRID
%
% OUTPUTS:
%     gridout: adjusted MxNx2 or NxMx2 array

% turn off unwanted warnings
warning('off','MATLAB:polyfit:PolyNotUnique');
warning('off','MATLAB:polyfit:RepeatedPointsOrRescale');

rowslope=zeros(1,size(grid,1));
colslope=zeros(1,size(grid,2));

% get slopes of rows
for rowindx=1:size(grid,1)
    currentrowx=grid(rowindx,:,1);
    currentrowy=grid(rowindx,:,2);
    currentrowx=currentrowx(currentrowx>0);
    currentrowy=currentrowy(currentrowy>0);
    P=polyfit(currentrowx,currentrowy,1);
    rowslope(rowindx)=abs(P(1));
end

% get slopes of columns
for colindx=1:size(grid,2)
    currentcolx=grid(:,colindx,1);
    currentcoly=grid(:,colindx,2);
    currentcolx=currentcolx(currentcolx>0);
    currentcoly=currentcoly(currentcoly>0);
    P=polyfit(currentcolx,currentcoly,1);
    colslope(colindx)=abs(P(1));
end

% reset warnings
warning('on','MATLAB:polyfit:PolyNotUnique');
warning('on','MATLAB:polyfit:RepeatedPointsOrRescale');

rowslope=mean(rowslope);
colslope=mean(colslope);

% adjust for the higher slope
if colslope>rowslope
    gridtemp(:,:,1)=rot90(grid(:,:,1));
    gridtemp(:,:,2)=rot90(grid(:,:,2));
    gridout=gridtemp;
else
    gridout=grid;
end