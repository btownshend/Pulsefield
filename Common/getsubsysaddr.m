function [host,port]=getsubsysaddr(id,varargin)
defaults=struct('reload',true,'configfile',[pfroot(),'/src/urlconfig.txt'],'debug',false);
args=processargs(defaults,varargin);
global subsystems;

host=[];
port=nan;

if isempty(subsystems) || args.reload
  if args.debug
    fprintf('Loading URL configurations from %s\n', args.configfile);
  end
  fd=fopen(args.configfile,'r');
  if fd<0
    error('Unable to open url config file: %s\n', args.configfile);
  end
  c=textscan(fd,'%s%s%d','Delimiter',',');
  fclose(fd);
  ss=struct('id',c{1},'host',c{2},'port',num2cell(c{3}));
  if ~isempty(subsystems)
    % Compare them
    changed=false;
    if length(subsystems)~=length(ss)
      changed=true;
    else
      for i=1:length(ss)
        if ~strcmp(ss(i).id,subsystems(i).id) || ~strcmp(ss(i).host,subsystems(i).host) || ss(i).port~=subsystems(i).port
          changed=true;
        end
      end
    end
    if changed
      fprintf('Cached version of subsystems differ from file -- closing existing connections\n');
      oscclose();
      arduino_close();
    end
  end
  subsystems=ss;
end

host=[];
port=[];
for i=1:length(subsystems)
  if strcmp(subsystems(i).id,id)
    host=subsystems(i).host;
    port=subsystems(i).port;
    return;
  end
end
fprintf('Error: unable to locate %s in URL config file\n', id);
