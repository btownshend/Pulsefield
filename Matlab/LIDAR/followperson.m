% Plot following someone
function followperson(csnap,id,f1,f2)
maxfollow=20;
if f2-f1>=maxfollow
  error('Can only follow for up to %d frames',maxfollow);
end
frames=arrayfun(@(z) z.vis.frame,csnap);
fsel=find(frames>=f1 & frames<=f2);
pos=[];
for i=1:length(fsel)
  t=csnap(fsel(i)).tracker.tracks;
  psel=find([t.id]==id);
  if ~isempty(psel)
    pos(end+1:end+2,:)=t(psel).legs;
  end
end
if size(pos,1)==0
  error('No frames that include ID %d', id);
end
c=[min(pos(:,1)),max(pos(:,1)),min(pos(:,2)),max(pos(:,2))];
c=c+[-1 1 -1 1]*0.4;
for f=f1:f2
  plotsnap(csnap,'frame',f);
  axis(c);
  index=find(frames==f);
  if ~isempty(index) 
    t=csnap(index).tracker.tracks;
    psel=find([t.id]==id);
    if ~isempty(psel)
      setfig(sprintf('Frame %d Like',f));clf;
      t(psel).plotlike(csnap(index).vis,false);
    end
  end
end
