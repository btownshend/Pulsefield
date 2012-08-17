% Send a ping to dest and look for replies at src
function ok=oscping(destIdent,replyIdent)
  ok=true;
  pingnum=randi(100000);
  % Flush incoming messages
  while true
    m=oscmsgin(replyIdent);
    if isempty(m)
      break;
    end
  end
  oscmsgout(destIdent,'/ping',{int32(pingnum)});
  fprintf('%s->%s ping(%d) ',replyIdent, destIdent, pingnum);
  while true
    m=oscmsgin(replyIdent,1.0);
    if ~isempty(m)
      fprintf('Got %s\n', formatmsg(m.path,m.data));
    end
    if isempty(m)
      ok=false;
      break;
    elseif strcmp(m.path,'/ack')
      if m.data{1}~=pingnum
        fprintf('ACK mismatch: sent %d, got %d -- continuing to wait\n', pingnum, m.data{1});
      else
        break;
      end
    else
      fprintf('Flushed %s on %s while looking for /ack\n',m.path,replyIdent);
    end
  end
end