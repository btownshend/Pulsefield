function comparesnaps(s1,s2)
f1=arrayfun(@(z) z.vis.frame, s1);
f2=arrayfun(@(z) z.vis.frame, s2);
frame=intersect(f1,f2);
for i=1:length(frame)
  f=frame(i);
  c1=s1(f1==f).vis.class;
  c2=s2(f2==f).vis.class;
  if any(c1~=c2)
    fprintf('Frame %d (%d,%d) has %d differences: ', f, find(f1==f), find(f2==f), sum(c1~=c2));
    for i=1:length(c1)
      if c1(i)~=c2(i)
        fprintf('%d:%d,%d ', i, c1(i), c2(i));
      end
    end
    fprintf('\n');
  else
    fprintf('Frame %d (%d,%d) identical\n', f, find(f1==f), find(f2==f));
  end
end