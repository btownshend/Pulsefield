% inpoly - check if a point is inside a polygon
function inside=inpoly(poly,pt)
ncross=0;
for j=1:size(poly,1)
  j2=mod(j,size(poly,1))+1;
  delta=poly(j2,:)-poly(j,:);
  v=pt-poly(j,:);
  v2=pt-poly(j2,:);
  if v2(2)*v(2) < 0
    if v(1)< v(2)/delta(2) * delta(1);
      ncross=ncross+1;
      %          fprintf('(%.1f,%.1f)->(inf,%.1f) crosses (%.1f,%.1f)->(%.1f,%.1f)\n', pt, tpos(i,2), poly(j,:), poly(j2,:));
    end
  end
end
inside=mod(ncross,2)==1;

