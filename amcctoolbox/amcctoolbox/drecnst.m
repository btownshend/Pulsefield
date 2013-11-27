function [ptpos,ptclrs]=drecnst(imleftname,imrightname)

imleft=im2double(imread(imleftname));
imright=im2double(imread(imrightname));

imleft=imresize(imleft,0.25,'nearest');
imright=imresize(imright,0.25,'nearest');


sizex=size(imleft,1);
sizey=size(imleft,2);
% patchx=6;
% patchy=6;
% patchxh=2;
% patchyh=2;

% mptsl=[];
% mptsr=[];
% match points
% corsignal=zeros(1,size(imleft,2)-2*patchyh);
% corsigmat=zeros((2*patchxh+1)*(2*patchyh+1),size(imleft,2)-2*patchyh);

xh=2;
yh=2;
psz=[2*xh+1,2*yh+1];

imleftcols=im2col(imleft,psz);
imrightcols=im2col(imright,psz);


for x=1:sizex-2*patchxh
    bandpatch=imrightcols(:,(x-1)*sizey+(1:sizey));
    for y=1:sizey-2*patchyh
        corsignal=imleftcols(:,(x-1)*sizey+y);
        [pks,pkinds]=sort(corsignal);
        if pks(1)<pks(2)
            mptsl=[mptsl,[x;y]];
            mptsr=[mptsr,[x;pkinds(1)+patchyh]];
        end
%         corsignal=corsignal(patchxh+1,patchyh+1:size(imright,2)-patchyh);
    end
end

% load stereo calibration parameters
% load Calib_Results_stereo_rectified.mat;



