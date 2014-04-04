% Get probability of estimate positions legs being separated by any distance using provided position variance
% Assumes normal distribution N(0,possigma) for position estimates
%  log-normal distribution for sep;   log(legsep)~N(log(mu),log((mu+sigma)/mu))
function legsepdist=getlegsepdist(legsepmu,legsepsigma,possigma)
LOGLEGSEPMU=log(legsepmu);
LOGLEGSEPSIGMA=log(1+legsepsigma/legsepmu);
nstd=3;    % Run out this far on the individual distributions
res=.01;
maxlegsep=legsepmu*exp(nstd*LOGLEGSEPSIGMA);
legsepdist=struct('d',0:res:maxlegsep+nstd*possigma);
legsepdist.p=zeros(size(legsepdist.d));
truesep=exp(log(res):res:log(maxlegsep));
pls=normpdf(log(truesep),LOGLEGSEPMU,LOGLEGSEPSIGMA);
spmv=[];
for j=1:length(truesep)
  pmv=normpdf(legsepdist.d-truesep(j),0.0,possigma);
  spmv(j)=sum(pmv);
  legsepdist.p=legsepdist.p+pmv.*pls(j);
end

if false
  setfig('legseptest'); clf;
  legsepdist.p=legsepdist.p*res;
  plot(legsepdist.d,legsepdist.p,'r');
end



  
