function [resp,cnt]=portread(s1,n)
if nargin<2
  n=1;
end
if isfield(s1,'socket')
  % Ethernet
  resp=[];
  np=0;
  for i=1:1000
    resp=[resp,jtcp('read',s1,'MAXNUMBYTES',n-length(resp))];
    cnt=length(resp);
    if cnt>=n
      break;
    end
    np=np+1;
    pause(0.001);
%    fprintf('paused\n');
  end
%  if cnt<n
%    fprintf('Read %d/%d bytes: %s (0x%s)\n',cnt,n-length(resp), sprintf('%c',char(resp)),sprintf('%02x',resp));
%  end
%  if (np>0)
%    fprintf('np=%d\n',np);
%  end
else
  [resp,cnt]=fread(s1,n,'uchar');
end

