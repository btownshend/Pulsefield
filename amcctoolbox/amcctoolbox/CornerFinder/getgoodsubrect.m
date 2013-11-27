function gridout=getgoodsubrect(grid,badptsx,badptsy)
%GETGOODSUBRECT extracts a grid subset of the input grid such that the output grid does not contain any bad points.
% 
% GETGOODSUBRECT takes as input a MxNx2 matrix acting a chessboard grid.
% It also takes two vectors containing the x and y coordinates of the bad
% points. The two vectors must be the same size.
% 
% The result returned is suboptimal. Optimal algorithms introduce
% complexity beyond our basic requirements.
% 
% USAGE:
%     gridout=getgoodsubrect(grid,badptsx,badptsy);
% 
% INPUTS:
%     grid: MxNx2 chessboard grid
% 
%     badptsx: x coordinates of the bad points
% 
%     badptsy: y coordinates of the bad points
% 
% OUTPUTS:
%     gridout: a subset of the input grid without any bad points


xmin=1;
xmax=size(grid,1);
ymin=1;
ymax=size(grid,2);

for i=1:length(badptsx)
    dist=[badptsx(i)-1,badptsy(i)-1,size(grid,1)-badptsx(i),size(grid,2)-badptsy(i)];
    [minval,minindx]=min(dist);
    switch minindx
        case 1
            if badptsx(i)>=xmin
                xmin=badptsx(i)+1;
            end
        case 2
            if badptsy(i)>=ymin
                ymin=badptsy(i)+1;
            end
        case 3
            if badptsx(i)<=xmax
                xmax=badptsx(i)-1;
            end
        case 4
            if badptsy(i)<=ymax
                ymax=badptsy(i)-1;
            end
    end
end

gridout=grid(xmin:xmax,ymin:ymax,:);