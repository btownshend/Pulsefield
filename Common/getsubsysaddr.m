function [host,port]=getsubsysaddr(id,varargin)
defaults=struct('reload',false,'configfile','/Users/bst/DropBox/PeopleSensor/config/urlconfig.txt');
args=processargs(defaults,varargin);
global subsystems;
if isempty(subsystems) || args.reload
  fprintf('Loading URL configurations from %s\n', args.configfile);
  fd=fopen(args.configfile,'r');
  if fd<0
    error('Unable to open url config file: %s\n', args.configfile);
  end
  c=textscan(fd,'%s%s%d','Delimiter',',');
  fclose(fd);
  subsystems=struct('id',c{1},'host',c{2},'port',num2cell(c{3}));
end
if ~isempty(id)
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
end
