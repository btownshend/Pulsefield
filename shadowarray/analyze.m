% Analyze - analyze results of a simul run
if ~exist('rays')
  % Create rays (image with each pixel indicating the LED number that the ray is directed to)
  rays=zeros(size(v,1),p.isize,p.isize);
  % Create raylines{c,l}(i,2) - coords of points along ray from camera c to led l
  raylines={};
  for c=1:size(v,1)
    fprintf('Computing rays from camera %d\n',c);
    for l=1:size(v,2)
      raylines{c,l}=zeros(0,2);
      cp=cpos(c,:);
      lp=lpos(l,:);
      delta=lp-cp;
      maxd=max(abs(delta));
      delta=delta/maxd;
      pv=cp;
      while (max(pv)>p.isize || min(pv)<0) && maxd>0
        pv=pv+delta;
        maxd=maxd-1;
      end
      while max(pv)<=p.isize && min(pv)>=0 && maxd>0
        fp=floor(pv)+1;
        rays(c,fp(1),fp(2))=l;
        raylines{c,l}(end+1,:)=fp;
        pv=pv+delta;
        maxd=maxd-1;
      end
    end
  end
else
  fprintf('Using existing rays(%d,%d,%d)\n',size(rays));
end
im=zeros(p.isize+1,p.isize+1);
% Can't be anyone along borders
im(1,:)=1;im(:,1)=1;im(end,:)=1;im(:,end)=1;
for c=1:size(v,1)
  im=im | ismember(squeeze(rays(c,:,:)),find(v(c,:)==1));
end


figure(2);
clf;
imshow(im'==0);
hold on;
viscircles(tpos,tgtdiam/2,'LineWidth',0.5);
% Dilate image using disk
ib=im==0;	% 1 where a person could be
% structure element slightly smaller than a person
se=strel('disk',floor(mintgtdiam/2),0);
ib=imerode(im==0,se);	% Erode image (1==person could be present)
ibd=imdilate(ib,se);

% Find rays that pass through only one region of possible target
ntgt=[];
ibcertain=0.5*ones(size(ib));   % In ibcertain, 1==person definitely present, 0=not present, 0.5=maybe
for c=1:size(raylines,1)
  for l=1:size(raylines,2)
    r=raylines{c,l};
    lind=sub2ind(size(ib),r(:,1),r(:,2));
    ntgt(c,l)=sum(diff(ib(lind))==1);
    if ntgt(c,l)==1
      % Only 1 target on this line, mark it
      sel=ibd(lind)==1;  % Points that were already marked
      ibcertain(lind(sel))=1;
      ibcertain(lind(~sel))=0;
%      fprintf('Marked %d points on line from camera %d to led %d as certain.\n', sum(sel), c, l);
    elseif ntgt(c,l)==0
      ibcertain(lind)=0;
    end
  end
end
figure(4);
clf;
imshow(ibcertain');

figure(3);
clf;
imshow((1-ib/2)');
hold on;
viscircles(tpos,tgtdiam/2,'LineWidth',0.5);
% Figure out centers of each possible target
[cx,cy]=ind2sub(size(ib),find(bwmorph(ib,'shrink',inf)));
centers=[cx,cy];
% Check all the centers pair-wise to see which ones are unique to at least one camera
distinct=zeros(1,size(centers,1));
for c=1:size(cpos,1)
  for i=1:size(centers,1)
    vdir=centers(i,:)-cpos(c,:);
    angle(i)=atan2(vdir(2),vdir(1));
    dist(i)=norm(vdir);
  end
  for i=1:size(centers,1)
    if abs(angle(i)-atan2(cdir(c,2),cdir(c,1)))>p.cam.fov/2
      % Not visible to this camera
      continue;
    end
    aliased=0;
    for j=1:size(centers,1)
      anglefuzz=maxtgtdiam/dist(j);
      % Check if angles are close and if the other person is in a position that could commpletely block out person i
      if i~=j && abs(angle(j)-angle(i))<anglefuzz && dist(j)<2*dist(i)
%        fprintf('Targets %d (a=%.2f) and %d (a=%.2f) are aliased from camera %d (%.2f<%.2f)\n', i,angle(i)*180/pi,j,angle(j)*180/pi,c,abs(angle(j)-angle(i))*180/pi,anglefuzz*180/pi);
        aliased=1;
      end
    end
%    fprintf('Center %d, Target %d: aliased=%d\n', c, i, aliased);
    if aliased==0
      fprintf('Center %d is distinct from camera %d\n', i, c);
      distinct(i)=distinct(i)+1;
    end
  end
end
sel=distinct>0;
for i=1:size(centers,1)
  text(centers(i,1),centers(i,2),sprintf('%d',i));
end
for i=1:size(cpos,1)
  text(cpos(i,1),cpos(i,2),sprintf('%d',i));
end

plot(centers(sel,1),centers(sel,2),'xg');  
plot(centers(~sel,1),centers(~sel,2),'xb');  
      

