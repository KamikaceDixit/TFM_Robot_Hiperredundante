clear;
%dejdkajsda
syms t0 t1 a b c d e f x0 x1 v0 v1 a0 a1 t v x a;

A=[t0^5 t0^4 t0^3 t0^2 t0 1;
   t1^5 t1^4 t1^3 t1^2 t1 1;
   5*t0^4 4*t0^3 3*t0^2 2*t0 1 0;
   5*t1^4 4*t1^3 3*t1^2 2*t1 1 0;
   20*t0^3 12*t0^2 6*t0 2 0 0
   20*t1^3 12*t1^2 6*t1 2 0 0];

X=[a; b; c; d; e; f;];
B=[x0; x1; v0; v1; a0; a1;];
detA=det(A);

A1=A; A1(:,1)=B;
a=det(A1)/det(A);
A2=A; A2(:,2)=B;
b=det(A2)/det(A);
A3=A; A3(:,3)=B;
c=det(A3)/det(A);
A4=A; A4(:,4)=B;
d=det(A4)/det(A);
A5=A; A5(:,5)=B;
e=det(A5)/det(A);
A6=A; A6(:,6)=B;
f=det(A6)/det(A);

t0=0;t1=8000;x0=0; x1=30000; v0=0;v1=0;a0=0;a1=0;

x=a*t^5 + b*t^4 + c*t^3 + d*t^2 + e*t0 + f;
v=5*a*t^4 + 4*b*t^3 + 3*c*t^2 + 2*d*t + e;
a=20*a*t^3 + 12*b*t^2 + 6*c*t + 2*d;
t=t0:(t1-t0)/100:t1;
x=subs(x);
v=subs(v);
a=subs(a);
figure
hold on
plot(t,x);
plot(t,v*1000);
plot(t,a*1000000);