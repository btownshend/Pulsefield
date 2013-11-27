function [gridout,nointerpolations]=getmisscrnrs(grid,imsize,debug)
% GETMISSCRNRS fills in the gaps in the grid by linear interpolation.
% 
% GETMISSCRNRS successively inerpolates missing points by fitting a least
% square line of the row and column the missing point belongs to and then
% storing the point as the intersection of the two lines.
% 
% USAGE:
%     [gridout,nointerpolations]=getmisscrnrs(grid);
% 
% INPUTS:
%     grid: output of FILTERGRID
% 
% OUTPUTS:
%     gridout: grid with the missing corners interpolated
% 
%     nointerpolations: the number of interpolations is used to detect the
%     quality of the chessboard detection by FINDCORNERS

%Function that interpolates the positions of missing corners
if ~exist('debug','var') || isempty(debug)
    debug=0;
end

% comment the line to allow debugging
debug=0;

gridout=grid;
nointerpolations=0;

% list of interpolations that lie outside the image
outgrid=zeros(size(gridout,1),size(gridout,2));

for x=1:size(gridout,1)
    for y=1:size(gridout,2)
        if gridout(x,y,1)==0
            nzerrowx=gridout(x,:,1);
            nzerrowx=nzerrowx(nzerrowx>0);
            nzerrowy=gridout(x,:,2);
            nzerrowy=nzerrowy(nzerrowy>0);
            nzercolx=gridout(:,y,1);
            nzercolx=nzercolx(nzercolx>0);
            nzercoly=gridout(:,y,2);
            nzercoly=nzercoly(nzercoly>0);
            if length(nzerrowx)<2 || length(nzercolx)<2
                % cannot process grid
                gridout=[];
                return;
            end
            nointerpolations=nointerpolations+1;
            P1=polyfit(nzerrowy,nzerrowx,1);
            P2=polyfit(nzercolx,nzercoly,1);
            eqnsmat=[1,-P1(1);-P2(1),1];
            bmat=[P1(2);P2(2)];
            gridout(x,y,:)=eqnsmat\bmat;
            if gridout(x,y,1)<1 || gridout(x,y,1)>imsize(1) || gridout(x,y,2)<1 ||gridout(x,y,2)>imsize(2)
                % add to list for later processing
                outgrid(x,y)=1;
            end
            if debug
                close all;
                figure;
                hold on;
                plot(nzerrowy,polyval(P1,nzerrowy));
                plot(polyval(P2,nzercolx),nzercolx);
                plot(gridout(:,:,2),gridout(:,:,1),'+');
            end
        end
    end
end


% process outlist

while ~isempty(find(outgrid,1))
    if outgrid(1,1)
        r=1;
        c=1;
    elseif outgrid(1,end)
        r=1;
        c=size(outgrid,2);
    elseif outgrid(end,1)
        r=size(outgrid,1);
        c=1;
    elseif outgrid(end,end)
        r=size(outgrid,1);
        c=size(outgrid,2);
    else
        % error
        gridout=[];
        return;
    end

    % remove row or column
    if size(outgrid,2)>size(outgrid,1)
        % remove column
        outgrid(:,c)=[];
        gridout(:,c,:)=[];
    else
        % remove row
        outgrid(r,:)=[];
        gridout(r,:,:)=[];
    end
end

