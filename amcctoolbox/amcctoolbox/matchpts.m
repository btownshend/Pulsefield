function [matc,err]=matchpts(pts2,pts1,delta,phi,d)

if ~exist('d','var')
    d=0;
end
% find points in 2 closest to points in 1

pts12=phi*pts1+repmat(delta,1,size(pts1,2));

matc=zeros(1,size(pts1,2));
err=zeros(1,size(pts1,2));
for cntr=1:size(pts12,2)
    dists=sqrt(sum((pts2-repmat(pts12(:,cntr),1,size(pts2,2))).^2));
    [err(cntr),matc(cntr)]=min(dists);
end

% for points reoccurring in 2 choose the ones with the shortest distance

for cntr=unique(matc)
    ind=find(matc==cntr);
    [m,mi]=min(err(ind));
    ind(mi)=[];
    matc(ind)=0;
    err(ind)=0;
end

if d
    lmind=find(matc);
    rmind=matc(lmind);
    C=lines(size(pts1,2));
    
    Cr=zeros(size(pts2,2),3);
    Cl=zeros(size(pts1,2),3);
    
    Cr(rmind,:)=C(lmind,:);
    Cl(lmind,:)=C(lmind,:);

%     clf;
%     hold on;
    scatter3(pts2(1,:),pts2(2,:),pts2(3,:),50,Cr,'.');
    scatter3(pts12(1,:),pts12(2,:),pts12(3,:),50,Cl,'.');
%     axis image;
%     pause;
end
