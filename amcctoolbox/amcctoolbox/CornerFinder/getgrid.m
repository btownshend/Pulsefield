function crnrsgridout=getgrid(crnrs,pixs,peaklocs,ix,iy,debug)
% GETGRID arranges the points that pass the chessboard filter into a grid.
% 
% GETGRID returns the arranged grid of the set of candidate chessboard
% corners. 
% 
% USAGE:
%     crnrsgridout=getgrid(crnrs,pixs,peaklocs,ix,iy);
% 
% INPUTS:
%     crnrs: 2xN matrix of the N candidate corners.
% 
%     pixs: 2xM matrix of the M Harris corners
% 
%     peaklocs: edge peak locations at each point (necesary for
%     arrangement)
% 
%     ix: the x component of the gradient image
% 
%     iy: the y component of the gradient image
% 
% OUTPUTS:
%     crnrsgridout: MxNx2 dimensional matrix contianing arranged points



% check input
if ~exist('debug','var') || isempty(debug)
    debug=0;
end

%adjust array direction
if size(crnrs,1)>2
    crnrs=crnrs';
end

nocrnrs=size(crnrs,2);

% get mean point
centerpt=mean(crnrs,2);
[currentpt,ctindx]=findnearest(centerpt,crnrs,1,0);

% parameters
angth=deg2rad(15);

% always store first point
valid=1;

% inititalise matrices
crnrsgrid=zeros(nocrnrs,nocrnrs);
crnrsgriddone=zeros(nocrnrs,nocrnrs);

% place first point in the middle of the grid
xg=round(nocrnrs/2);
yg=round(nocrnrs/2);

% enumerate different directions
right=1;
top=2;
left=3;
bottom=4;

% setup position matrix
posmat=[0,top,0;left,0,right;0,bottom,0];

% set while loop flag
notdone=1;

% set loop counter
% just to ensure safe performance, prevent loop from going on forever
loopcntr=0;
looplimit=1e6;

while notdone && loopcntr<looplimit
    
    loopcntr=loopcntr+1;
    % get current point coords
    currentpt=crnrs(:,ctindx);
    xc=currentpt(1);
    yc=currentpt(2);
    
    % get surrounding chessboard corner properties (sorted by distance)
    [surcrnrs,surindx]=findnearest(currentpt,crnrs,8);
    vecs(1,:)=surcrnrs(1,:)-xc;
    vecs(2,:)=surcrnrs(2,:)-yc;
    angles=cart2pol(vecs(1,:),vecs(2,:));
    angles(angles<=0)=angles(angles<=0)+2*pi();
    
    % setup the segment vectors to the corners in order to check for 
    % validity of segment between points by summing ix and iy along segment
    
    vecsnrm=zeros(size(vecs)); % normal vectors
    vecslen=zeros(1,size(vecs,2)); % vector lengths
    for veccnr=1:size(vecs,2)
        vecslen(veccnr)=norm(vecs(:,veccnr));
        vecsnrm(:,veccnr)=vecs(:,veccnr)/vecslen(veccnr);
    end
    
    ixvalue=zeros(size(vecslen)); % ix values
    iyvalue=ixvalue; % iy values
    segedgevalue=iyvalue;
    
    for crnrcnr=1:size(vecs,2)
        for evcnr=1:round(vecslen(crnrcnr))
            xev=round(xc+vecsnrm(1,crnrcnr)*evcnr);
            yev=round(yc+vecsnrm(2,crnrcnr)*evcnr);
            ixvalue(crnrcnr)=ixvalue(crnrcnr)+ix(xev,yev);
            iyvalue(crnrcnr)=iyvalue(crnrcnr)+iy(xev,yev);
        end
        segedgevalue(crnrcnr)=norm([ixvalue(crnrcnr),iyvalue(crnrcnr)]/evcnr);
    end
    segedgemean=mean(segedgevalue);
    
    
    % get surrounding Harris point properties (sorted by distance)
    surpixs=findnearest(currentpt,pixs,8);
    vecspixs(1,:)=surpixs(1,:)-currentpt(1);
    vecspixs(2,:)=surpixs(2,:)-currentpt(2);
    anglespixs=cart2pol(vecspixs(1,:),vecspixs(2,:));
    anglespixs(anglespixs<=0)=anglespixs(anglespixs<=0)+2*pi();
    
    theta=pi()/90:pi()/90:2*pi();

    
    locs=peaklocs(:,ctindx);
    
    locs=locs';
    if length(locs)~=4
        error('There should be 4 and only 4 peaks');
    end
    lineangles=theta(locs);

    % get cross corners

    % reset crosspixs
    crosspixs=zeros(1,4);
    
    for pk=1:4
        for crnr=1:length(surindx)
            % check for angle proximity and segment edge projection
            if angprox(angles(crnr),lineangles(pk),angth) && segedgevalue(crnr)>segedgemean
                for pix=1:size(surpixs,2)
                    % check if a Harris corner lies in between
                    if angprox(anglespixs(pix),lineangles(pk),angth)
                        if isequal(surpixs(:,pix),surcrnrs(:,crnr))
                            % store
                            crosspixs(pk)=surindx(crnr);
                            break;
                        else
                            break;
                        end
                    end
                end
            end
        end
    end

    
    %Adjust cross
    for i=1:size(crosspixs,2)
        for u=xg-1:xg+1
            for v=yg-1:yg+1
                if crosspixs(i)==crnrsgrid(u,v) && crnrsgriddone(u,v)>0 % check for valid corner, check for non zero value as well
                    valid=1; % a cross is valid if a match is found
                    k=posmat(u-xg+2,v-yg+2)-i;
                    crosspixs=crosspixs(mod((1:end)-k-1, end)+1 );
                end
            end
        end
    end
    

    
    if valid % if connection found store
    
        %draw cross matrix
        cmat=zeros(3,3);
        cmat(2,2)=ctindx;
        cmat(2,3)=crosspixs(1);
        cmat(1,2)=crosspixs(2);
        cmat(2,1)=crosspixs(3);
        cmat(3,2)=crosspixs(4);

        %store changes
        crnrsgrid(xg-1:xg+1,yg-1:yg+1)=crnrsgrid(xg-1:xg+1,yg-1:yg+1)+cmat.*(~crnrsgrid(xg-1:xg+1,yg-1:yg+1));
        cmatdone=[0,1,0;1,2,1;0,1,0];
        cmatdone=cmatdone.*(cmatdone&cmat)-crnrsgriddone(xg-1:xg+1,yg-1:yg+1);
        cmatdone(cmatdone<0)=0;
        crnrsgriddone(xg-1:xg+1,yg-1:yg+1)=cmatdone+crnrsgriddone(xg-1:xg+1,yg-1:yg+1);
%         if debug
%             close all;
%             figure; imshow(img);
%             hold on; plot(crnrsgrid(:,:,2),crnrsgrid(:,:,1),'o');
%         end
    % reset valid
    valid=0;
    else % ignore and reset
        crnrsgriddone(xg,yg)=0;
    end
    
    %get new point
    [xg,yg]=find(crnrsgriddone==1,1);
    
    % if no new point found end
    if isempty(xg)
        notdone=0;
    else
        ctindx=crnrsgrid(xg,yg);
    end
end

% store x and y coords into matrix
crnrsgridout=zeros([size(crnrsgrid),2]);

for x=1:size(crnrsgrid,1)
    for y=1:size(crnrsgrid,2)
        if crnrsgrid(x,y)
            crnrsgridout(x,y,:)=crnrs(:,crnrsgrid(x,y));
        end
    end
end

function prox=angprox(ang1,ang2,th)
% ANGPROX checks if the two angles are within a certain threshold.
%

prox=abs(ang1-ang2)<th || abs(ang1-ang2)>(2*pi-th);
