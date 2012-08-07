% Initialize OSC struct
% Start a server on port which can later be polled for connections
function oscinit(port)
if nargin<1
  [~,port]=getsubsysaddr('MPO');
end

global oscsetup

if ~isempty(oscsetup)  && isfield(oscsetup,'server')
  % Already have a setup struct
  if isfield(oscsetup,'port') && oscsetup.port==port
    % All OK, just return
    fprintf('Skipping oscinit -- already have port setup\n');
    return
  end
  % Need to close existing server and start again
  fprintf('Closing existing OSC server port\n');
  osc_free_server(oscsetup.server);
end
server=osc_new_server(port);
fprintf('Started new OSC server at port %d\n', port);
oscsetup=struct('port',port,'server',server,'clients',[]);
