p=struct('proj',[],'cameraview',[],'pose',[],'world2screen',[],'screen2world',[]);
maps=cell(3,3);
% Flush queue
while ~isempty(oscmsgin('VD',0,true))
end

while (true)
  msg=oscmsgin('VD',1);
  if ~isempty(msg)
    msg.path
    if strncmp(msg.path,'/cal/',5)
      unit=msg.data{1}+1;
      data=cell2mat(msg.data(2:end))
      if strcmp(msg.path,'/cal/pose')
        p(unit).pose=data;
      elseif strcmp(msg.path,'/cal/projection')
        p(unit).proj=[data(1:3);data(4:6)];
      elseif strcmp(msg.path,'/cal/cameraview')
        p(unit).cameraview=reshape(data,4,3)';
      elseif strcmp(msg.path,'/cal/world2screen')
        p(unit).world2screen=reshape(data,3,3)';
      elseif strcmp(msg.path,'/cal/screen2world')
        p(unit).screen2world=reshape(data,3,3)';
      elseif strcmp(msg.path,'/cal/mapping')
        unit1=msg.data{1}+1;
        unit2=msg.data{2}+1;
        pt=msg.data{3}+1;
        npts=msg.data{4};
        p1=[msg.data{5},msg.data{6}];
        p2=[msg.data{7},msg.data{8}];
        mapmat=maps{unit1,unit2};
        if length(mapmat)>npts
          mapmat=mapmat(1:npts);
        end
        mapmat.p1(pt,:)=p1;
        mapmat.p2(pt,:)=p2;
        maps{unit1,unit2}=mapmat;
      end
    end
  end
end
