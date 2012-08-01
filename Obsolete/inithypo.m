% Initialize hypothesis of locations using fixed positions
% Input:
%   tgts - struct holding positions of targets
%   imap - map from physical dimensions to pixels on bitmaps of space
% Output:
%   h.prob(k,isize,isize) - for each individual, k, a map of the prob they are centered at the given coord (1=possibly there, 0=not there)
function h=inithypo(tgts,imap)
tpos=m2pix(imap,tgts.tpos);
h=struct('prob',zeros(size(tpos,1),imap.isize(1),imap.isize(2)),'pos',tgts.tpos);
% Prob=1 that they are where they are
for i=1:size(h.prob,1)
  h.prob(i,round(tpos(i,1)),round(tpos(i,2)))=1;
end
