function ok=sync(s1,maxwait)
if nargin<2
  maxwait=1;   % Wait 1 second
end
counter=syncsend(s1);
ok=syncwait(s1,counter,maxwait);
