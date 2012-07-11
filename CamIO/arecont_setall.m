function p=arecont_setall(id,p)
params=fieldnames(p);
for i=1:length(params)
  newval=p.(params{i});
  oldval=arecont_get(id,params{i});
  if isnumeric(oldval)
%    fprintf('%s: %d->%d\n', params{i}, oldval, newval);
    if oldval~=newval
      fprintf('Changing %s from %d to %d\n', params{i}, oldval, newval);
      arecont_set(id,params{i},newval);
      reval=arecont_get(id,params{i});
      if reval~=newval
        fprintf('Unable to change %s to %d, value reads as %d\n', params{i}, newval, reval);
        p.(params{i})=reval;
      end
    end
  else
%    fprintf('%s: %s->%s\n', params{i}, oldval, newval);
    if ~strcmp(oldval,newval)
      fprintf('Changing %s from %s to %s\n', params{i}, oldval, newval);
      arecont_set(id,params{i},newval);
      reval=arecont_get(id,params{i});
      if ~strcmp(reval,newval)
        fprintf('Unable to change %s to %s, value reads as %s\n', params{i}, newval, reval);
        p.(params{i})=reval;
      end
    end
  end
end
