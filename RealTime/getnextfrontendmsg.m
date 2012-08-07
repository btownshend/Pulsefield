function msg=getnextfrontendmsg(timeout)
global visserver;

msg=[];
if isempty(visserver.msgin)
  % Get some more messages, timeout after given time
  visserver.msgin=osc_recv(visserver.addr,timeout);
end

if isempty(visserver.msgin)
  fprintf('No messages available after timeout of %.2f seconds\n', timeout);
else
  if iscell(visserver.msgin)
    msg=visserver.msgin{1};
    visserver.msgin=visserver.msgin(2:end);
  else
    msg=visserver.msgin;
    visserver.msgin=[];
  end
  %  fprintf('Have %d messages, next is %s\n', length(visserver.msgin)+1, msg.path);
end
