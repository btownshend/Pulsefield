% Find optimal velocity damping predictor
function optimalveldamping(csnap)
fps=50;
v=nan(0,2,2);
p=nan(0,2,2);
for i=1:length(csnap)
  if length(csnap(i).tracker.tracks) >= 1 &&all(isfinite(csnap(i).tracker.tracks(1).legvelocity(:)))
    v(end+1,:,:)=csnap(i).tracker.tracks(1).legvelocity/fps;
    p(end+1,:,:)=csnap(i).tracker.tracks(1).legs;
  end
end
% Use instananeous velociy
v(2:end,:,:)=diff(p,1);
%v(1,:,:)=nan;
steps=1:5
dimlbl='xy';
alldpred=[];
allfit=[];
alldelta=[];
for step=steps
  fprintf('Step=%d\n', step);
  for dim=1:2
    pos=p(:,:,dim);
    delta=(pos(1+step:end,:)-pos(1:end-step,:));%/step;
    vel=v(1:end-step,:,dim);
    pred=[];
    for leg=1:3
      if leg==3
        fit=[vel;vel(:,[2,1])]\[delta(:,1);delta(:,2)];
        else
          fit=vel\delta(:,leg);
      end
      if leg==3
        dpred=vel*fit;
        pred=pos(1:end-step,1)+dpred;
        dpred(:,2)=vel*fit([2,1]);
        pred(:,2)=pos(1:end-step,2)+dpred(:,2);
      else
        dpred=pos(1:end-step,leg)+vel*fit;
        pred(step:step+length(dpred)-1,leg)=pos(1:end-step,leg)+dpred;
      end
      fprintf('  d%c(%d)=%.2f*v(1) %.2f*v(2)\n', dimlbl(dim),leg,fit);
    end
  end
  alldpred(step,1:size(pred,1),:)=dpred;    % (step,frame,leg)
  alldelta(step,1:size(delta,1),:)=delta;   % (step,frame,leg)
  allfit(step,:)=fit;      % (step,[same,other])
  alldeltastd(step,:)=std(delta(:));
  allpredstd(step,:)=std(delta(:)-dpred(:));
  gain(step)=std(delta(:))/std(delta(:)-dpred(:));
  fprintf('std(delta)=%.2f, std(delta-dpred)=%.2f, gain=%.2f\n', std(delta(:)),std(delta(:)-dpred(:)),gain(step));
end
A=[1,0.1];
modeldecay=0.95;
setfig('optimalveldamping');clf;

subplot(311);
h=plot(steps,allfit(:,1));
hold on;
h(2)=plot(steps,allfit(:,2),'r');
h(3)=plot(steps,sum(allfit,2),'k');
h(4)=plot(steps,A(1)*modeldecay.^(steps-1),':');
h(5)=plot(steps,A(2)*modeldecay.^(steps-1),'r:');
legend(h,{'Same','Other','Total','Same (model)','Other (model)'});
xlabel('Step');
ylabel('Weight');

subplot(312)
step=1;
plot(alldelta(step,:,1),alldpred(step,:,1),'r.');
hold on;
plot(alldelta(step,:,2),alldpred(step,:,2),'g.');
xlabel('Observed Delta');
ylabel('Predicted delta');
axis equal
title(sprintf('Step %d',step));

subplot(313)
h=plot(alldeltastd,'r');
hold on;
h(2)=plot(allpredstd,'g');
xlabel('Step');
ylabel('Std Dev of Prediction');
legend({sprintf('No movement (E=%.3f)',alldeltastd(end)),sprintf('Fit (E=%.3f)',allpredstd(end))});
title('Reduction of stddev of delta');


% Test a model
cols='rgbcmyk';

mdls={[0 0 0 0],[1,0,.99,0],[1,0,.9,.09],[1,0,.8,.19],[1,0,.75,.24]};
setfig('optim-err');clf;
h=[];leg={};
for i=1:length(mdls)
  mdl=mdls{i};
  err=[];
  for step=steps
    [ppos,pvel]=model(mdl,p,v,step);
    fprintf('Steps=%d ',step);
    err(step)=errcheck(p,v,ppos,pvel);
  end
  h(i)=plot(diff([0,err]),cols(mod(i-1,length(cols))+1));
  hold on;
  leg{i}=sprintf('Model=%s (E=%.3f)',sprintf('%.2f ',mdl),err(end));
  %  fprintf('%s\n',leg{i});
end
xlabel('Number of step');
ylabel('Error');
legend(h,leg,'Location','SouthEast');
title('Error');


function err=errcheck(pos,vel,ppos,pvel)
diff=ppos-pos;
err=nanstd(diff(:));
fprintf('std(err)=%.3f\n',err);


% Model - using mdl parameters, pos(nframes,2,2), vel(nframes,2,2), predict nstep aheads to give predpos(nframes,2,2),predvel(nframes,2,2)
% predpos(1:nstep-1,:) will be nan
function [pos,vel]=model(mdl,pos,vel,nstep)

for i=1:nstep
  pos=pos+vel*mdl(1)+vel(:,[2,1],:)*mdl(2);
  vel=vel*mdl(3)+vel(:,[2,1],:)*mdl(4);
end
pos(1+nstep:end,:,:)=pos(1:end-nstep,:,:);
pos(1:nstep,:,:)=nan;
vel(1+nstep:end,:,:)=vel(1:end-nstep,:,:);
vel(1:nstep,:,:)=nan;
