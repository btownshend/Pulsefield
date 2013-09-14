% Simulate using a rolling ball as a calibration device
cpos=[-1 -1
      0 -1
      1 -1
      -1 1
      0 1
      1 1];
ncam=size(cpos,1);
nball=10;
bpos=rand(nball,2)*2-1;
d=nan(nball+ncam,nball+ncam);
for i=1:nball
  for j=1:ncam;
    dist=norm(cpos(j,:)-bpos(i,:));
    dist=dist*(1+randn(1)*dist/60);
    d(i,j+nball)=dist;
    d(j+nball,i)=dist;
  end
end
for i=1:nball+ncam
  d(i,i)=0;
end
opts=statset('Display','iter','TolFun',1e-10,'TolX',1e-10);
[y,stress]=mdscale(d,2,'Start','random','Criterion','metricstress','Options',opts);
tform=maketform('projective',cpos([1,2,5,6],:),y(nball+[1,2,5,6],:));
yt=tforminv(tform,y);
e=mean(yt)-mean([cpos;bpos]);
yt(:,1)=yt(:,1)-e(1);
yt(:,2)=yt(:,2)-e(2);
setfig('Setup'); clf;
plot(cpos(:,1),cpos(:,2),'o');
hold on;
plot(bpos(:,1),bpos(:,2),'x');
axis equal;

setfig('Solution'); clf;
plot(y(1:nball,1),y(1:nball,2),'x');
hold on;
plot(y(nball+1:end,1),y(nball+1:end,2),'o');
axis equal;

setfig('Solution (aligned)'); clf;
plot(yt(1:nball,1),yt(1:nball,2),'xr');
hold on;
plot(bpos(:,1),bpos(:,2),'xg');
berr=sqrt(mean(mean((yt(1:nball,:)-bpos).^2)));
for i=1:nball
  plot([yt(i,1),bpos(i,1)],[yt(i,2),bpos(i,2)],'-');
end
plot(yt(nball+1:end,1),yt(nball+1:end,2),'or');
plot(cpos(:,1),cpos(:,2),'og');
cerr=sqrt(mean(mean((yt(nball+(1:ncam),:)-cpos).^2)));
for i=1:ncam
  plot([yt(nball+i,1),cpos(i,1)],[yt(nball+i,2),cpos(i,2)],'-');
end
axis equal;

fprintf('Ball RMSE=%.3f\n',berr);
fprintf('Camera RMSE=%.3f\n',cerr);



