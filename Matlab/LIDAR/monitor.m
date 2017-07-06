% 6/3/17
% Monitor OSC messages and store in MATLAB struct
% Haven't really used since dump .mat files from frontend works well
function d=monitor() 
  fprintf('Flushing...');
  while ~isempty(oscmsgin('VD',0.0))
  end
  fprintf('ready\n');
  ignored=containers.Map();
  d=struct('body',[],'leg',[]);
  while true
    m=oscmsgin('VD',1.0);
    if isempty(m) 
      if size(d.body,1)>0
        break;
      end
      continue;
    end
    if strcmp(m.path,'/pf/frame')
      %frame=m.data{1};
    elseif strcmp(m.path,'/pf/body')
      d.body(end+1,:)=cell2mat(m.data);
    elseif strcmp(m.path,'/pf/leg')
      d.leg(end+1,:)=cell2mat(m.data);
    elseif ~isKey(ignored,m.path)
      fprintf('Ignoring %s\n', m.path);
      ignored(m.path)=true;
    end
  end
  fprintf('\n');
end

