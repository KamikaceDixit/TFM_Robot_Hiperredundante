function [] = Rep_Tr(Tr1,Tr2,Tr3,t0,t1,t2,t3)
Time1=t0:0.01:t1;
Time2=t1:0.01:t2;
Time3=t2:0.01:t3;
syms t;

if(isempty(Time2))
    plot([Time1 Time3],[subs(Tr1,t,Time1) subs(Tr3,t,Time3-Time3(1))]);
else
    plot([Time1 Time2 Time3],[subs(Tr1,t,Time1) subs(Tr2,t,Time2-Time2(1)) subs(Tr3,t,Time3-Time3(1))]);
end

end