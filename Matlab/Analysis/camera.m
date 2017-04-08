% Form camera matrix from eye,ref,up
function c=camera(eye,ref,up)
z=eye-ref;z=z/norm(z);
x=cross(up,z);x=x/norm(x);
y=cross(z,x);y=y/norm(y);
c=[x;y;z];
c(4,4)=1;
trans=zeros(4,4);
trans(1,1)=1;trans(2,2)=1;trans(3,3)=1;trans(4,4)=1;
trans(1:3,4)=-eye;
c=c*trans;
