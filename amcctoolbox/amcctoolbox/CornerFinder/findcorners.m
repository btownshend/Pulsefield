function imgcout=findcorners(img,debug)
% FINDCORNERS is the main function called by the calibration GUI.
%
% FINDCORNERS takes a grayscale image return a structure variable imgcout 
% containing the extracted chessboard grid.
% 
% USAGE:
%     imgcout=findcorners(img);
% 
% INPUTS:
%     img: grayscale image (any class)
%     debug: for debugging
% 
% OUTPUTS:
%     imgcout: structure containing the following elements:
% 
%         grid: NxDx2 array containing the x and y coordinates of the
%         chessboard corners detected
%         
%         win: the minimum distance between chessboard corners in pixels
%         
%         suspicious: flag indicating wether the output requires user
%         confirmation
%         
%         fail: flag indicationg the failure of detection

%
% Input Checking
%

% check image
if size(img,3)>1
    error('Input image is required to be grayscale');
end

if min(size(img))<50
    error('Image too small');
end

% check debug flag
if ~exist('debug','var') || isempty(debug)
    debug=0;
end

% change image into double
img=double(img);
img=gscale(img,'minmax');


%
% Parameters
%

hwin=3; % Harris window size
th=0.5; % Parameter to adjust adaptive thresholding

%
% Make Pyramid
%
% Each level in pyramid consists of:
% im: image
% imadj: adjusted image
% ime: sobel edge image
% ix: x component of edge image
% iy: y component of edge image

nolevels=3; % no of levels
pyr=cell(1,3); % initialise pyr

for cntr=1:nolevels
    % downsize image
    im=imresize(img,1/(2^(cntr-1)));
    pyr{cntr}.im=im;
    
    % adaptively adjust
    pyr{cntr}.imadj=adaptimadj(im,[]);
    
    % get sobel edge image
    [imgedge,ix,iy]=getedges(im);
    pyr{cntr}.ime=imgedge;
    pyr{cntr}.ix=ix;
    pyr{cntr}.iy=iy;
end

% set nocrnrs and level
nocrnrs=-1;
level=1;

for cntr=1:length(pyr)
    imgh=harristm(pyr{cntr}.imadj,hwin);

    [mimg,stdv]=adaptstats(imgh);
    ctcrnrpts=getcrnrpts(imgh,mimg,stdv,th);
    
    % check
    if isempty(ctcrnrpts)
        continue;
    end
    
    [ctcrnrs,ctnocrnrs,ctpeaklocs]=chesscornerfilter(pyr{cntr}.im,pyr{cntr}.ime,ctcrnrpts,debug);
    if ctnocrnrs>nocrnrs
        nocrnrs=ctnocrnrs;
        crnrs=ctcrnrs;
        crnrpts=ctcrnrpts;
        peaklocs=ctpeaklocs;
        level=cntr;
    end
    
    % debugging
    if debug
        close all;
        figure;imshow(pyr{cntr}.imadj);
        hold on;plot(ctcrnrpts(2,:),ctcrnrpts(1,:),'+');
        if ~isempty(ctcrnrs)
            plot(ctcrnrs(2,:),ctcrnrs(1,:),'square','color','r');
        end
        pause;
    end
end

%   Check for enough no of corners
if nocrnrs<10
    imgcout=failmode;
    return;
end

% debugging
if debug
    close all;
    figure;imshow(pyr{level}.im);
    hold on;plot(crnrpts(2,:),crnrpts(1,:),'+');
    if ~isempty(crnrs)
        plot(crnrs(2,:),crnrs(1,:),'square','color','r');
        pause;
    end
end


%
% Extract Grid
%

crnrsgrid=getgrid(crnrs,crnrpts,peaklocs,pyr{level}.ix,pyr{level}.iy,debug);

% adjust grid back to full scale
crnrsgrid=crnrsgrid*(2^(level-1));
crnrpts=crnrpts*(2^(level-1));

crnrsgridfil=filtergrid(crnrsgrid);

% check grid size
if min(size(crnrsgridfil(:,:,1)))<3
    imgcout=failmode;
    return;
end

% adjust grid direction
crnrsgridfil=adjgriddir(crnrsgridfil);

% get missing corners
[gridfullrect,nointerpolations]=getmisscrnrs(crnrsgridfil,size(img));
if isempty(gridfullrect)
    imgcout=failmode;
    return;
end

% adjust origin position
gridfullrect=adjgridorigin(gridfullrect);

[grid,win,nobadpts]=getsubpixcrnrs(img,crnrpts,gridfullrect);

if min(size(grid(:,:,1)))<3
    imgcout=failmode;
    return;
end

susth=4;

suspicious=0;
if nobadpts>susth || nointerpolations>susth
    suspicious=1;
end

if debug
    close all;
    figure;
    imshow(pyr{level}.im);
    hold on;   
    plot(crnrsgrid(:,:,2),crnrsgrid(:,:,1),'+');
    figure;
    imshow(pyr{level}.im);
    hold on;
    plot(crnrsgridfil(:,:,2),crnrsgridfil(:,:,1),'o');
    figure;
    imshow(img);
    hold on;
    plot(gridfullrect(:,:,2),gridfullrect(:,:,1),'square');
    figure;imshow(img);hold on;
    plot(grid(:,:,2),grid(:,:,1),'.','MarkerSize',5);
    imgcptrd=getframe(gcf);
    imgcout.img=imgcptrd.cdata;
end

%Output Results
imgcout.grid=grid;
imgcout.suspicious=suspicious;
imgcout.win=win;
imgcout.fail=false;


%fail mode
function imgcout=failmode
imgcout.grid=[];
imgcout.suspicious=[];
imgcout.win=[];
imgcout.fail=true;