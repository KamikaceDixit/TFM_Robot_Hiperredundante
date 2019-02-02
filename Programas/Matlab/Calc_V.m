function [V_Tr] = Calc_V(AcelT,DcelT,PulsesIni,PulsesFin,Time)
%Poli4_Pos(t0,t1,x_0,v_0,v_1,a_0,a_1)
%Poli1_Pos(t0,x_0,v_0)
%Rep_Tr(Tr1,Tr2,Tr3,t0,t1,t2,t3)
%Pulses(t0,t1,x_0,v_0,v_1,a_0,a_1)
syms v t;
PulsesAcel=Pulses(0,AcelT,0,0,v,0,0);
PulsesDcel=Pulses(0,DcelT,0,v,0,0,0);
Total_Pulses=PulsesAcel+PulsesDcel+(v*(Time-AcelT-DcelT));

V_Tr=solve(Total_Pulses-PulsesFin+PulsesIni,v);
end