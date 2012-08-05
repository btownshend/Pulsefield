% Process args
% Usage:
%  defaults=struct(name,value,name,value...)
%  args=processargs(defaults,varargin);
function args=processargs(args,va)
i=1;
while i<=length(va)
  if ~isfield(args,va{i})
    error('Unknown option: "%s"\n',va{i});
  end
  args.(['SET',va{i}])=true;   % Flag that it was set explicitly
  if islogical(args.(va{i})) && (i==length(va) || ischar(va{i+1}))
    % Special case of option string without value for boolean, assume true
    args.(va{i})=true;
  else
    args.(va{i})=va{i+1};
    i=i+1;
  end
  i=i+1;
end
