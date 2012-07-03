% Update current hyothesis of locations 
% Input:
%   hprob(k,isize,isize) - for each individual, k, a map of the prob they are centered at the given coord (1=possibly there, 0=not there)
%   possible(isize,isize) - pixel positions that could be a center point of a person
%   maxchange - max movement of anyone in pixels
% Output:
%   hf.prob - updated hprob
function hf=updatehypo(p,layout,imap,h,possible,maxchange,doplot)
if nargin<5
  doplot=0;
end
pghost=0.5;   % Prob any given pixel is a ghost pixel
% Build a filter to allow movement
fsize=ceil(m2pix(imap,p.simul.speed*p.simul.dt));
filt=fspecial('disk',fsize);
% increase probability of stationary
filt=filt/2;
filt(fsize+1,fsize+1)=filt(fsize+1,fsize+1)+0.5;

hf=struct('prob',zeros(size(h.prob)));

for i=1:size(h.prob,1)
  hf.prob(i,:,:)=imfilter(squeeze(h.prob(i,:,:)),filt).* possible;
  % Renormalize
  hf.prob(i,:,:)=hf.prob(i,:,:)/sum(sum(hf.prob(i,:,:)));
end
% Scale to account for multiple targets being at the same point
% TODO

% Compute expected value of each
indmat=zeros(size(h.prob,2),size(h.prob,3),2);
indmat(:,:,1)=repmat((1:size(h.prob,2))',1,size(h.prob,3));
indmat(:,:,2)=repmat(1:size(h.prob,3),size(h.prob,2),1);
hf.pos=[];
for i=1:size(h.prob,1)
  p=squeeze(hf.prob(i,:,:));
  % Renormalize
  p=p/sum(sum(p));
  % Wtd average
  hf.pos(i,:)=[sum(sum(indmat(:,:,1).*p)),sum(sum(indmat(:,:,2).*p))];
end

hf.pos=pix2m(imap,hf.pos);
