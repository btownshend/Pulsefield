% Find minimum distance from a set of points to the entry
% Entry is defined by entryline, if present, entry point otherwise
function dist=disttoentry(layout,p);

if isfield(layout,'entryline')
  origin=layout.entryline(1,:);
  endpt=layout.entryline(2,:);
  dir=layout.entryline(2,:)-layout.entryline(1,:);  
  len=norm(dir);
  dir=dir/norm(dir);
  dist=1e99;
  for i=1:size(p,1)
    pvec=p(i,:)-origin;
    pvec2=p(i,:)-endpt;
    if dot(pvec,dir)<0
      % Before initial point
      d=norm(p(i,:)-origin);
    elseif dot(pvec2,-dir)<0
      % After endpoint
      d=norm(p(i,:)-endpt);
    else
      % inside
      v=pvec-(dot(pvec,dir)*dir);
      d=norm(v);
    end
    dist=min(d,dist);
  end
else
  pixdist=sqrt((p(:,1)-layout.entry(1)).^2+(p(:,2)-layout.entry(2)).^2);
  dist=min(pixdist);
end