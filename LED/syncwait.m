function ok=syncwait(s1,counter,maxwait)
if nargin<3
  maxwait=1;   % Wait 1 second
end
nread=0;
ok=false;
timeout=now+maxwait/24/60/60;

while true
  [resp,cnt]=portread(s1,1);
  if cnt==0
    if now>timeout
      fprintf('Timeout reading sync response\n');
      return;
    end
    continue;
  end
 % fprintf('Got 0x%02x\n',resp);
  if nread==0
    if resp=='A'
      nread=nread+1;
    else
      fprintf('sync: Expect "A" (0x%02x), got 0x%02x\n', 'A',resp);
      continue;
    end
  else
    % second byte of response
    if resp==counter
      %      fprintf('SYNC OK\n');
      ok=true;
      return;
    end
    fprintf('Mismatched sync: sent %d, got %d\n', counter, resp);
  end
end
