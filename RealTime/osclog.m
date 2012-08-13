function osclog(varargin)
defaults=struct('client',[],'msg',[],'path',[],'data',[],'server',[],'close',false);
args=processargs(defaults,varargin);

global osclogfd
if args.close
  if ~isempty(osclogfd)
    fprintf('Closing OSC log file\n');
    try 
      fclose(osclogfd);
    catch me
      fprintf('Error closing file: %s\n', me.message);
    end
    osclogfd=[];
  end
  return;
end
  
if isempty(osclogfd)
  logfname=sprintf('/tmp/osclog-%s.txt',datestr(now,'mmddHHMMSS'));
  osclogfd=fopen(logfname,'w');
  fprintf('Logging OSC commands to %s\n', logfname);
  fprintf(osclogfd,'Time\tSrc\tDest\tCmd\tData\n');
end

ts=datestr(now,'mm/dd HH:MM:SS.FFF');
if ~isempty(args.msg)
  desc=formatmsg(args.msg.path,args.msg.data,9);
elseif ~isempty(args.path)
  desc=formatmsg(args.path, args.data,9);
else
  desc='??';
end
if ~isempty(args.client)
  % Message to client
  if isempty(args.client.ident)
    dest=args.client.url;
  else
    dest=[args.client.ident,'@',args.client.url];
  end
  fprintf(osclogfd,'%s\t.\t%s\t%s\n',ts,dest,desc);
end
if ~isempty(args.server)
  % Message to server (me)
  if ~isempty(args.msg)
    src=args.msg.src;
  else
    src='??';
  end
  fprintf(osclogfd,'%s\t%s\t.\t%s\n',ts,src,desc);
end  
