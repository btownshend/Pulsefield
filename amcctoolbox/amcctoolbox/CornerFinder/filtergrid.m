function gridout=filtergrid(grid)
% FILTERGRID removes spur rows and columns existing after the grid arrangement process.
% 
% FITLERGRID processes the input grid and iteratively removes rows and
% columns until a rectangular grid is obtained.
% 
% INPUTS:
%     grid: MxNx2 matrix output by GETGRID
% 
% OUTPUTS:
%     gridout: VxWx2 matrix containing the filtered grid

gridout=grid;
while 1
    row1count=0;
    row2count=0;
    col1count=0;
    col2count=0;
    
    rowthresh=size(gridout,1)/2;
    colthresh=size(gridout,2)/2;
    
    for y=1:size(gridout,2)
        if gridout(1,y)
            row1count=row1count+1;
        end
        if gridout(end,y)
            row2count=row2count+1;
        end
    end
    for x=1:size(gridout,1)
        if gridout(x,1)
            col1count=col1count+1;
        end
        if gridout(x,end)
            col2count=col2count+1;
        end
    end
    
    row1count=row1count-rowthresh;
    row2count=row2count-rowthresh;
    col1count=col1count-colthresh;
    col2count=col2count-colthresh;
    
    % remove row or column with the least number of points
    [mincount,indx]=min([row1count,row2count,col1count,col2count]);
    if mincount<0
        switch indx
            case 1
                gridout(1,:,:)=[];
            case 2
                gridout(end,:,:)=[];
            case 3                
                gridout(:,1,:)=[];
            case 4
                gridout(:,end,:)=[];
        end
    else
        break;
    end
end