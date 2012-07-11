% Salvaged code - in case its needed later

% Find rays that pass through only one region of possible target
ntgt=[];
ibcertain=0.5*ones(size(ib));   % In ibcertain, 1==person definitely present, 0=not present, 0.5=maybe
for c=1:size(rays.raylines,1)
  for l=1:size(rays.raylines,2)
    r=rays.raylines{c,l};
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
if doplot
  setfig('analyze.certain');
  clf;
  imshowmapped(rays.imap,ibcertain);
  title('analyze.certain');
end


% Check all the centers pair-wise to see which ones are unique to at least one camera
distinct=zeros(1,size(centers,1));
for c=1:size(layout.cpos,1)
  for i=1:size(centers,1)
    vdir=centers(i,:)-layout.cpos(c,:);
    angle(i)=atan2(vdir(2),vdir(1));
    dist(i)=norm(vdir);
  end
  for i=1:size(centers,1)
    if abs(angle(i)-atan2(layout.cdir(c,2),layout.cdir(c,1)))>p.cam.fov/2
      % Not visible to this camera
      continue;
    end
    aliased=0;
    for j=1:size(centers,1)
      anglefuzz=p.maxtgtdiam/dist(j);
      % Check if angles are close and if the other person is in a position that could commpletely block out person i
      if i~=j && abs(angle(j)-angle(i))<anglefuzz && dist(j)<2*dist(i)
%        fprintf('Targets %d (a=%.2f) and %d (a=%.2f) are aliased from camera %d (%.2f<%.2f)\n', i,angle(i)*180/pi,j,angle(j)*180/pi,c,abs(angle(j)-angle(i))*180/pi,anglefuzz*180/pi);
        aliased=1;
      end
    end
%    fprintf('Center %d, Target %d: aliased=%d\n', c, i, aliased);
    if aliased==0
%      fprintf('Center %d is distinct from camera %d\n', i, c);
      distinct(i)=distinct(i)+1;
    end
  end
end


  sel=distinct>0;
  plot(tgts.tpos(sel,1),tgts.tpos(sel,2),'xg');  
  plot(tgts.tpos(~sel,1),tgts.tpos(~sel,2),'xb');  
