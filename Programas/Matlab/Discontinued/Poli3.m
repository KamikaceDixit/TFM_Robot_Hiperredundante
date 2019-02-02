clear;
%dejdkajsda
syms t0 t1 a b c d v0 v1 a0 a1 t v e;

A=[t0^3 t0^2 t0 1;
    t1^3 t1^2 t1 1;
    3*t0^2 2*t0 1 0;
    3*t1^2 2*t1 1 0;];
X=[a; b; c; d;];
B=[v0; v1; a0; a1;];
detA=det(A);

A1=A;
A1(:,1)=B;
A2=A;
A2(:,2)=B;
A3=A;
A3(:,3)=B;
A4=A;
A4(:,4)=B;
a=det(A1)/det(A); %subs(det(A1)/det(A))
b=det(A2)/det(A);
c=det(A3)/det(A);
d=det(A4)/det(A);

t0=0.7;t1=1;v0=6000;v1=0;a0=0;a1=0;x0=200;
%%Antonio HELP
a=(2*t0*v1 - 2*t0*v0 + 2*t1*v0 - 2*t1*v1 + a0*t0^2 + a0*t1^2 + a1*t0^2 ...
    + a1*t1^2 - 2*a0*t0*t1 - 2*a1*t0*t1)/(t0^4 - 4*t0^3*t1 + 6*t0^2*t1^2 ...
    - 4*t0*t1^3 + t1^4);

b=-(a0*t0^3 + 2*a0*t1^3 + 2*a1*t0^3 + a1*t1^3 - 3*t0^2*v0 + 3*t0^2*v1 + ...
    3*t1^2*v0 - 3*t1^2*v1 - 3*a0*t0*t1^2 - 3*a1*t0^2*t1)/(t0^4 - ...
    4*t0^3*t1 + 6*t0^2*t1^2 - 4*t0*t1^3 + t1^4);

c=(a0*t1^4 + a1*t0^4 + 2*a0*t0^3*t1 + 2*a1*t0*t1^3 + 6*t0*t1^2*v0 - ...
    6*t0^2*t1*v0 - 6*t0*t1^2*v1 + 6*t0^2*t1*v1 - 3*a0*t0^2*t1^2 - ...
    3*a1*t0^2*t1^2)/(t0^4 - 4*t0^3*t1 + 6*t0^2*t1^2 - 4*t0*t1^3 + t1^4);

d=(t0^4*v1 + t1^4*v0 - a0*t0*t1^4 - a1*t0^4*t1 - 4*t0*t1^3*v0 - ...
    4*t0^3*t1*v1 + 2*a0*t0^2*t1^3 - a0*t0^3*t1^2 - a1*t0^2*t1^3 + ...
    2*a1*t0^3*t1^2 + 3*t0^2*t1^2*v0 + 3*t0^2*t1^2*v1)/(t0^4 - ...
    4*t0^3*t1 + 6*t0^2*t1^2 - 4*t0*t1^3 + t1^4);

e=x0-(1/4)*a*t0^4-(1/3)*b*t0^3-(1/2)*c*t0^2-d*t0


x=(1/4)*a*t^4+(1/3)*b*t^3+(1/2)*c*t^2+d*t + e
v=a*t^3+b*t^2+c*t+d;
acel=3*a*t^2+2*b*t+c;


t=t0:(t1-t0)/100:t1;
v=subs(v);
x=subs(x);
acel=subs(acel);
figure
hold on

plot(t,v);
plot(t,x);
plot(t,acel);