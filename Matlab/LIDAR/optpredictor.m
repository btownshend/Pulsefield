% Find optimal velocity damping predictor
function fit=optpredictor(csnap,nlag,ndelay,noise)
  if nargin<4
    % Use 3mm of noise to improve robustness
    noise=0;
  end
  if nargin<3
    % Delay of first sample used in predictor
    ndelay=1;
  end
  if nargin<2
    % Number of previous values to use for predictor
    nlag=4
  end
  defwts=[1.1 .18
          -0.1 -.18]';
  defwts=defwts(:);
  defwts=defwts/sum(defwts);
  fps=50;
  x=nan(0,2,2);
  v=nan(0,2,2);
  s=nan(0,2);
  pcode=nan(0,2,2);
  frame=[];
  for i=1:length(csnap)
    if length(csnap(i).tracker.tracks) >= 1 &&all(isfinite(csnap(i).tracker.tracks(1).legs(:)))
      x(end+1,:,:)=csnap(i).tracker.tracks(1).legs;
      pcode(end+1,:,:)=csnap(i).tracker.tracks(1).predictedlegs;
      v(end+1,:,:)=csnap(i).tracker.tracks(1).legvelocity;
      s(end+1,:)=[length(csnap(i).tracker.tracks(1).scanpts{1}),length(csnap(i).tracker.tracks(1).scanpts{1})];
      frame(end+1)=csnap(i).vis.frame;
    end
  end
  if noise~=0
    fprintf('Adding %.1f mm of noise to data\n',noise);
  end
  xn=x+randn(size(x))*noise/1000;   % Add noise to data
                                    % Take x velocity
                                    %x=v(:,:,1)';

  % Only used target points with lots of scanpts for training, measurement
  minscanpts=2;
  xgood=x;
  xgood(s(:,1)<minscanpts,1,:)=nan;
  xgood(s(:,2)<minscanpts,2,:)=nan;

  fit=[];
  options=optimset('TolX',1e-6,'TolFun',1e-6,'MaxFunEvals',10000,'MaxIter',10000);
  for lag=0:nlag
    % Try numerical fit
    fprintf('\nNLAG=%d NDELAY=%d\n',lag,ndelay);
    if lag>0
      fit(lag)=0;  % Add another dimension
      fit=fminsearch(@(z) prederror(z,ndelay,xgood,xn), fit,options);
      fprintf('FIT fit=[%s]; sum(fit)=%.4f\n', sprintf('%.4f ',fit),sum(fit));
    end

    fprintf('x1(n)=');
    for i=1:length(fit)
      delay=ceil(i/2)-1;
      fprintf('%.2f*x%d(n-%d) + ',fit(i), 2-mod(i,2), ndelay+delay);
    end
    fprintf('\n');

    xpred=predictor(fit,ndelay,x);
    xpred(1:nlag,:,:)=nan;
    error=xgood-xpred; % prederror(fit,ndelay,x,xn);
    fprintf('RMS residual after looking back %d samples (including other leg interleaved): %.2f mm; 99-th percentile=%.2f\n', length(fit), sqrt(nanmean(error(:).^2))*1000,prctile(abs(error(:)),95)*1000);
  end

  % Use default weights to predict
  deferr=xgood-predictor(defwts(ndelay:end),ndelay,x);
  fprintf('RMS residual using defwts: %.2f mm; 99-th percentile=%.2f\n', sqrt(nanmean(deferr(:).^2))*1000,prctile(abs(deferr(:)),95)*1000);
  
  %   Compare the predicted signal to the original signal
  zoherror=xgood-predictor(1,ndelay,x);

  % Just plot the x-coordinates 
  setfig('XPredError');clf;
  subplot(411);
  plot(frame,squeeze(zoherror(:,1,1))*1000);
  xlabel('Frame'); ylabel('X-Position Error (mm)'); grid;
  title(sprintf('Using zero-order hold (RMSE=%.2f)',sqrt(nanmean(zoherror(:).^2))*1000));
  c=axis;
  
  subplot(412);
  plot(frame,squeeze(error(:,1,1))*1000);
  xlabel('Frame'); ylabel('X-Position Error (mm)'); grid;
  title(sprintf('Using %d-order predictor (RMSE=%.2f)',nlag,sqrt(nanmean(error(:).^2))*1000));
  axis(c);

  subplot(413);
  plot(frame,squeeze(deferr(:,1,1))*1000);
  xlabel('Frame'); ylabel('X-Position Error (mm)'); grid;
  title(sprintf('Using %d-order default predictor (RMSE=%.2f)',length(defwts),sqrt(nanmean(deferr(:).^2))*1000));
  axis(c);
  suptitle('Delta position - Estimate - Leg 1, X-Coord');

  subplot(414);
  obserr=xgood-pcode;
  plot(frame,squeeze(obserr(:,1,1))*1000);
  xlabel('Frame'); ylabel('X-Position Error (mm)'); grid;
  title(sprintf('Using predictor in code (RMSE=%.2f)',sqrt(nanmean(obserr(:).^2))*1000));
  axis(c);
  suptitle('Delta position - Estimate - Leg 1, X-Coord');

  setfig('error');clf;
  plot(abs(xpred(:))*1000,abs(error(:))*1000,'.');
  xlabel('Predicted delta');
  ylabel('Error');
  sel=isfinite(xpred(:))&isfinite(error(:));
  emodel=polyfit(abs(xpred(sel))*1000,abs(error(sel))*1000,1)
  fprintf('RMSE=%.2f*|pred| + %.2f mm (each axis independent)\n', emodel);

  setfig('Fit');clf;
  if mod(length(fit),2)
    fit(end+1)=nan;
  end
  bar(ndelay+(0:length(fit)/2-1),reshape(fit,2,[])');
  xlabel('prior');
  ylabel('Weight of prior deltaxs');
  title('Final fit');
  
  % Circular variance
  predmag=sqrt(xpred(:,:,1).^2+xpred(:,:,2).^2);
  errmag=sqrt(error(:,:,1).^2+error(:,:,2).^2);
  sel=isfinite(predmag(:))&isfinite(errmag(:));
  emodel=polyfit(predmag(sel)*1000,errmag(sel)*1000,1)
  fprintf('RMSE=%.2f*|pred| + %.2f mm (magnitudes)\n', emodel);

  return;
end

% m is predictor of x(n,i,j)=m(1)*x(n-1,i,j)+m(2)*x(n-1,1-i,j)+m(3)*x(n-2,i,j)+...
function pred=predictor(m,ndelay,x)
% Add some noise to the inputs to make the predictor more robust
  pred=zeros(size(x));
  for i=1:2:length(m)
    delay=(i-1)/2+ndelay;
    p=m(i)*x;
    pred(1+delay:end,:,:)=pred(1+delay:end,:,:)+p(1:end-delay,:,:);
  end
  for i=2:2:length(m)
    delay=i/2-1+ndelay;
    p=m(i)*x(:,[2,1],:);
    pred(1+delay:end,:,:)=pred(1+delay:end,:,:)+p(1:end-delay,:,:);
  end
  pred(1:ceil(length(m)/2),:,:)=nan;
end

function e=prederror(m,ndelay,x,xn)
  pred=predictor(m,ndelay,xn);
  diff=x-pred;
  diff(1:ceil(length(m)/2),:,:)=nan;
  e=sqrt(nanmean(diff(:).^2));
end

