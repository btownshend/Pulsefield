% Reduce a set of pos and neg images to determine the active regions
% Usage: reduceimages(im)
%   - im{2}{nrep}
function map=reduceimages(im)
nrep=size(im,2);
pos=double(cell2mat(reshape(im(1,:),1,1,[])));
neg=double(cell2mat(reshape(im(2,:),1,1,[])));
diff=median(pos,3)-median(neg,3);
stdpos=std(pos,[],3)/sqrt(size(pos,3));
stdneg=std(neg,[],3)/sqrt(size(neg,3));
noise=sqrt((stdpos.^2+stdneg.^2));
thresh1=3*median(noise(:));
thresh2=3*median(noise(abs(diff(:))>thresh1));
map=zeros(size(diff));
map(diff>thresh2)=1;
map(diff<-thresh2)=-1;
%setfig('reduceimages');clf;
%imshow(map/2+0.5);

