function [gridout,win,nobadpts]=getsubpixcrnrs(img,crnrpts,grid)
%GETSUBPIXCRNRS retruns the subpixel positions of the chessboard corners.
% 
% GETSUBPIXCRNRS relies on the subpixel corner finder by 
% Jean-Yves Bouguet. The addition it introduces is the adaptive selection
% of the window size.
% 
% GETSUBPIXCRNRS also chooses a subset of the grid if certain corners do 
% converge using Bouget's code.

% reshape into row vector
gridx=grid(:,:,1);
gridy=grid(:,:,2);
indcs=find(gridx)';

gridpts=gridx(indcs);
gridpts=[gridpts;gridy(indcs)];

gridout=zeros(size(grid));
bad=false(size(grid,1),size(grid,2));

win=Inf;

% get smallest window size first
for cntr=1:size(gridpts,2)
    ctwin=round(getwin(img,gridpts(:,cntr),crnrpts)/2);
    % get smallest win size
    if ctwin<win && ctwin>2
        win=ctwin;
    end
end


for x=1:size(grid,1)
    for y=1:size(grid,2)
        currentpt=squeeze(grid(x,y,:));
        [subpxpt,goodpt,badpt]=subpixcrnr(currentpt,img,win,win); % adjust for x and y
        gridout(x,y,:)=subpxpt;
        bad(x,y)=badpt;
    end
end


[badptsx,badptsy]=find(bad);
nobadpts=length(badptsx);
gridout=getgoodsubrect(gridout,badptsx,badptsy);

