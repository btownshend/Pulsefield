% Dispatch OSC commands
% Usage: oscdispatch(dispatchtable)
% dispatchtable - struct array of {path,fn}
% executes each matching path fn in order.   Continues if return val is 1, finishes otherwise
function oscdispatch(msgs,dispatchtable)
for i=1:length(msgs)
  m=msgs{i};
  for j=1:2:length(dispatchtable)
    dpath=dispatchtable{j};
    dfn=dispatchtable{j+1};
    if strcmp(m.path,dpath)
      retval=dfn(m);
      fprintf('Dispatch: %d matched %s, retval=%d\n', (j+1)/2, m.path, retval);
      if retval==0
        return;
      end
    end
  end
  fprintf('Dispatch: no match to %s\n', m.path);
end
