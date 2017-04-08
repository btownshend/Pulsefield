function comparesnaps(s1,s2)
f1=arrayfun(@(z) z.vis.frame, s1);
f2=arrayfun(@(z) z.vis.frame, s2);
frame=intersect(f1,f2);
for i=1:length(frame)
  f=frame(i);
  c1=s1(f1==f).vis.class;
  c2=s2(f2==f).vis.class;
  if any(c1~=c2)
    fprintf('Frame %d (%d,%d) has %d class differences: ', f, find(f1==f), find(f2==f), sum(c1~=c2));
    for i=1:length(c1)
      if c1(i)~=c2(i)
        fprintf('%d:%d,%d ', i, c1(i), c2(i));
      end
    end
    fprintf('\n');
  end
  b1=s1(f1==f).vis.shadowed;
  b2=s2(f2==f).vis.shadowed;
  if any(b1(:)~=b2(:))
    fprintf('Frame %d (%d,%d) has %d shadowing differences: ', f, find(f1==f), find(f2==f), sum(b1(:)~=b2(:)));
    for i=1:size(b1,1)
      if b1(i,1)~=b2(i,1) || b1(i,2)~=b2(i,2)
        fprintf('%d:[%d,%d],[%d,%d] ', i, b1(i,:), b2(i,:));
      end
    end
    fprintf('\n');
  end
end