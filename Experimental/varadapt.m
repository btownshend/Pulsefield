% Check variance adaptation - for debugging of foreground detector
n=20;
cam=1;
if ~exist('bvis')
  fprintf('Block an area...');
  for i=1:n
    bvis{i}=getvisible(p,'stats',true,'usefrontend',true);
  end
  fprintf('done\n');
end

% Compute variance based on these samples and see if it matches
sx=zeros(size(std));sx2=sx;
vc=p.camera(cam).viscache;
% Build a table of pixels that are 'in use' -- have some blocked LEDs using them.
inuse=zeros(size(bvis{1}.im{1},1),size(bvis{1}.im{1},2));
for i=1:length(bvis)
  x=im2double(bvis{i}.im{cam});
  sx=sx+x;
  sx2=sx2+x.^2;
  med(i)=median(x(:));
  blocked=bvis{i}.v(cam,:)==0;
  inuse1=inuse*0;
  for j=1:length(vc.ledmap)
    if blocked(vc.ledmap(j))
      inuse1(vc.indices(j,:))=1;
    end
  end
  inuse=inuse+inuse1;
end
%figure; imshow(inuse>5); 
inuse(:,:,2)=inuse;
inuse(:,:,3)=inuse(:,:,1);
sel=inuse(:)>5;
if sum(sel)==0
  sel(:)=true;
  fprintf('Using all pixels as "in use"\n');
end
  
setfig('varadapt');
scale=10;
pnum=1;
for i=[1,5,10,15,20]
  subplot(5,1,pnum);pnum=pnum+1;
  std=sqrt(bvis{i}.refim2{cam})*255;
  imshow(std/scale);
  title(sprintf('T=%.1f sec, scale=%.2f, mean(std)=%.3f, inuse=%.3f',(bvis{i}.whenrcvd-bvis{1}.whenrcvd)*24*3600,scale,mean(std(:)),mean(std(sel))));
  set(gca,'Position',get(gca,'OuterPosition'));   % Explode plot to fill space
end

newstd=sqrt(max(0,sx2-sx.*sx/length(bvis))/length(bvis))*255;
setfig('vcompare');clf;
h=cdfplot(sqrt(bvis{1}.refim2{cam}(sel))*255);
hold on;
set(h,'Color',[1 0 0]);
h=cdfplot(sqrt(bvis{end}.refim2{cam}(sel))*255);
set(h,'Color',[0 1 0]);
cdfplot(newstd(sel));
legend('refim2{1}','refim2{end}','recomputed');
axis([0,15,0,1]);
setfig('varadapt-median');
plot(med*255);
xlabel('Sample');
ylabel('Median');
