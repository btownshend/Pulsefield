function gridout=adjgridorigin(grid)
% ADJGRIDORIGIN adjusts the input grid's origin to ensure consistency among images.
% 
% ADJGRIDORIGIN takes as input the grid from GETMISSCRNRS and returns a the
% grid with the origin adjusted if necessary. If no adjustment is needed 
% the same grid is returned.
% 
% INPUTS:
%     grid: output of GETMISSCRNRS
% 
% OUTPUTS:
%     gridout: adjusted grid

if norm(squeeze(grid(1,1,:)))>norm(squeeze(grid(end,end,:)))
    gridtemp(:,:,1)=rot90(grid(:,:,1),2);
    gridtemp(:,:,2)=rot90(grid(:,:,2),2);
    gridout=gridtemp;
else
    gridout=grid;
end