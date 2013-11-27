function [crnrs,nocrnrs,peaklocs]=chesscornerfilter(img,imgedge,crnrpts,debug)
% CHESSCORNERFILTER filters Harris corners for chessboard corners.
% 
% CHESSCORNERFILTER takes as input the image, the sobel edge image and
% the Harris corner points and outputs of the corner points that passed
% the filter the number of those points and the peak directions for each 
% of the points. The peak direction is used by the grid extraction (refer 
% to Bachelor Thesis by Abdallah Kassir 2009).
%   
% The details of the filter are applied in the function VALIDCORNER
% 
% USAGE:
%     [crnrs,nocrnrs,peaklocs]=chesscornerfilter(img,imgedge,crnrpts,debug)
% 
% INPUTS:
%     img: original grayscale image
% 
%     imgedge: Sobel edge image
% 
%     crnrpts: output of GETCRNRPTS
% 
% OUTPUTS:
%     crnrs: 2xN array of corners that passed the filter
% 
%     nocrnrs: number of corners found
% 
%     peaklocs: required by GETGRID

if ~exist('debug','var') || isempty(debug)
    debug=0;
end

% comment the line to allow debugging
debug=0;

% get sweepmatrices, precalculation of these matrices allows for much
% faster program execution
[sweepmatx,sweepmaty]=sweepmatrix(img);

% set output values
nocrnrs=0;
crnrs=[];
peaklocs=[];

i=0;

% loop over all points
for indx=1:size(crnrpts,2)
    x=crnrpts(1,indx);
    y=crnrpts(2,indx);
    
    % extract appropriate window size
    win=getwin(img,[x;y],crnrpts);
    
    % check window size
    if win<3
        continue;
    end
    imgcrop=img(x-win:x+win,y-win:y+win);
    imgedgecrop=imgedge(x-win:x+win,y-win:y+win);
    sweepmatxcrop=sweepmatx(1:round(1.3*win),:);
    sweepmatycrop=sweepmaty(1:round(1.3*win),:);
    
    % apply filter
    [valid,plocs]=validcorner(imgcrop,imgedgecrop,sweepmatxcrop,sweepmatycrop);
    if valid
        i=i+1;
        crnrs(1,i)=x;
        crnrs(2,i)=y;
        nocrnrs=nocrnrs+1;
        peaklocs(:,i)=plocs;
%     elseif x==129  
%         close all;
%         imshow(imgcrop,[]);
%         keyboard;
    end
end

% debugging
if debug
    close all;
    figure;imshow(img);hold on;
    plot(crnrpts(2,:),crnrpts(1,:),'+');
    plot(crnrs(2,:),crnrs(1,:),'o','color','red');
    pause;
end