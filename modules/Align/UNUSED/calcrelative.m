% Calculate relative orientation of projectors
% Build list of correspondences
corr=[];
step=0.025;
for x=0:step:1
  for y=0:step:1
    [cx,cy]=proj2cam(pmap{1},x,y);
    [px,py]=cam2proj(pmap{2},cx,cy);
    corr=[corr;x,y,px,py];
    [cx,cy]=proj2cam(pmap{2},x,y);
    [px,py]=cam2proj(pmap{1},cx,cy);
    corr=[corr;px,py,x,y];
  end
end
sel=all(isfinite(corr)');
corr=corr(sel,:);

x=eye(3);

for iter=1:20
% Find mapping from p1 to p2 as a 3x3 matrix 
options=optimset('display','final','MaxIter',1e4,'MaxFunEvals',10000,'TolX',1e-10,'TolFun',1e-10);
x=fminsearch(@(z) correrror(corr,z), x(:),options);
x1=reshape(x,3,3);
x1(3,3)=1;

setfig('reproj');clf;
c1=corr(:,[1,2]);
c1(:,3)=1;
c2=corr(:,[3,4]);
c1x=c1*x1;
c1x(:,1)=c1x(:,1)./c1x(:,3);
c1x(:,2)=c1x(:,2)./c1x(:,3);
for i=1:size(c2,1)
  plot([c1x(i,1),c2(i,1)],[c1x(i,2),c2(i,2)],'-k');
  hold on;
end
err=c1x(:,1:2)-c2(:,1:2);
e2=sqrt(err(:,1).^2+err(:,2).^2);
xlabel('P1 reprojected');
ylabel('P2');
fprintf('Mean error=%f, 95th-pctlile=%f\n', mean(e2),prctile(e2,95));
sel=e2>mean(e2)+3*std(e2);
if sum(sel)==0
  break;
end
fprintf('Dropping %d/%d outliers\n', sum(sel), length(sel));
corr=corr(~sel,:);
end
