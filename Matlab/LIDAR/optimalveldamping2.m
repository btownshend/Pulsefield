% Find optimal velocity damping predictor
function fit=optimalveldamping2(csnap,nlag,ndelay,noise)
  if nargin<4
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
  fps=50;
  p=nan(0,2,2);
  v=nan(0,2,2);
  s=nan(0,2);
  frame=[];
  for i=1:length(csnap)
    if length(csnap(i).tracker.tracks) >= 1 &&all(isfinite(csnap(i).tracker.tracks(1).legs(:)))
      p(end+1,:,:)=csnap(i).tracker.tracks(1).legs;
      v(end+1,:,:)=csnap(i).tracker.tracks(1).legvelocity;
      s(end+1,:)=[length(csnap(i).tracker.tracks(1).scanpts{1}),length(csnap(i).tracker.tracks(1).scanpts{1})];
      frame(end+1)=csnap(i).vis.frame;
    end
  end
  % Only with lots of scanpts
  minscanpts=2;
  p(s(:,1)<minscanpts,1,:)=nan;
  p(s(:,2)<minscanpts,2,:)=nan;
  % Take only x delta movement
  x=diff(p,1);
  if noise~=0
    fprintf('Adding %.1f mm of noise to data\n',noise);
  end
  xn=x+randn(size(x))*noise/1000;   % 3mm of noise to improve robustness
                                    % Take x velocity
                                    %x=v(:,:,1)';
  frame=frame(2:end);
  fit=[];
  options=optimset('TolX',1e-5,'TolFun',1e-5,'MaxFunEvals',10000,'MaxIter',10000);
  for lag=0:nlag
    % Try numerical fit
    fprintf('\nNLAG=%d NDELAY=%d\n',lag,ndelay);
    if lag>0
      fit(lag)=0;  % Add another dimension
      fit=fminsearch(@(z) prederror(z,ndelay,x,xn), fit,options);
      fprintf('FIT fit=[%s]; sum(fit)=%.4f\n', sprintf('%.4f ',fit),sum(fit));
    end

    fprintf('x1(n)=');
    for i=1:length(fit)
      delay=ceil(i/2)-1;
      fprintf('%.2f*x%d(n-%d) + ',fit(i), 2-mod(i,2), ndelay+delay);
    end
    fprintf('\n');

    xpred=predictor(fit,ndelay,x);
    xpred(1:nlag)=nan;
    error=xpred-x;
    fprintf('RMS residual after looking back %d samples (including other leg interleaved): %.2f mm; 99-th percentile=%.2f\n', length(fit), sqrt(nanmean(error(:).^2))*1000,prctile(abs(error(:)),95)*1000);
  end

  %   Compare the predicted signal to the original signal
  zoh=0*x;
  zoherror=zoh-x;

  setfig('XPredError');clf;
  subplot(311);
  plot(frame,squeeze(zoherror(:,1,1))*1000);
  xlabel('Frame'); ylabel('X-Position (mm)'); grid;
  title(sprintf('Using zero-order hold (RMSE=%.2f)',sqrt(nanmean(zoherror(:).^2))*1000));
  c=axis;

  subplot(312);
  plot(frame,squeeze(error(:,1,1))*1000);
  xlabel('Frame'); ylabel('X-Position (mm)'); grid;
  title(sprintf('Using %d-order predictor (RMSE=%.2f)',nlag,sqrt(nanmean(error(:).^2))*1000));
  axis(c);
  suptitle('Delta position - Estimate - Leg 1, X-Coord');

  subplot(313);
  plot(abs(xpred(:))*1000,abs(error(:))*1000,'.');
  xlabel('Predicted delta');
  ylabel('Error');
  sel=isfinite(xpred(:))&isfinite(error(:));
  emodel=polyfit(abs(xpred(sel))*1000,abs(error(sel))*1000,1)
  fprintf('RMSE=%.2f*|pred| + %.2f mm (each axis independent)\n', emodel);

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
end

function e=prederror(m,ndelay,x,xn)
  pred=predictor(m,ndelay,xn);
  diff=x-pred;
  diff(1:ceil(length(m)/2),:,:)=nan;
  e=sqrt(nanmean(diff(:).^2));
end

