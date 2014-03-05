simul=0;
if ~exist('vis','var')
  if simul
    if ~exist('recvis','var')
      recvis=load('../../Recordings/201309/20130901T020032.mat');
    end
    truth=struct('targets',[]);
    vis=[];
    for i=1:min(200,length(recvis.snap))
      tgt=[];
      for j=1:length(recvis.snap(i).hypo)
        tgt(j,:)=recvis.snap(i).hypo(j).pos;
      end
      if size(tgt,1)>0
        tgt(:,2)=tgt(:,2)-min(recvis.p.layout.pos(:,2));
      end
      truth.targets=tgt;
      vis=[vis,simlidar(truth,'when',recvis.snap(i).when)];
    end
  else
    [~,myport]=getsubsysaddr('MPV');
    fprintf('Instructing frontend to use port %d to send us msgs\n', myport);
    oscmsgout('FE','/vis/dest/add/port',{myport});
    ok=oscping('FE','MPV');

    oscmsgout('FE','/vis/get/reflect',{uint32(2)});
    oscmsgout('FE','/vis/set/echoes',{uint32(5)});

    vis=[];
    for i=1:100
      newvis=sickrcvr('debug',10);
      if isempty(newvis)
        break;
      end
      newvis.range=newvis.range(:,1,:);
      newvis.reflect=newvis.reflect(:,1,:);
      vis=[vis,newvis];
    end
  end
end

bg=[];
for i=1:length(vis)
  bg=updatebg(bg,vis(i));
  vis(i).targets=classify(vis(i),bg);
end

tracker=multiObjectTracking();
im=255*ones(400,600,3,'uint8');
for i=1:length(vis)
  tracker.update(vis(i).targets.pos,vis(i).targets.bbox);
  winbounds=[-4.5,4.5,-0.5,6.5];
  im2=vis2image(vis(i),im,winbounds);
  tracker.displayTrackingResults(im2,winbounds);
  input('?');
end
