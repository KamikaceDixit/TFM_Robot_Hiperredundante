%% Import data from text file.
load("alpha_0.1.mat");
figure
hold on
title('alpha 0.1')
plot(Time, Tr_pos);
plot(Time, Tr_vel);
plot(Time, Pos);
plot(Time, Vel);
legend('Tr-Pos','Tr-vel' ,'Pos','Vel')
xlabel('Time (ms)')
ylabel('Pulses - Pulses/s')

clear
load("alpha_0.2.mat");
figure
title('alpha 0.2')
hold on
plot(Time, Tr_pos);
plot(Time, Tr_vel);
plot(Time, Pos);
plot(Time, Vel);
legend('Tr-Pos','Tr-vel' ,'Pos','Vel')
xlabel('Time (ms)')
ylabel('Pulses - Pulses/s')

clear
load("alpha_0.4.mat");
figure
title('alpha 0.4')
hold on
plot(Time, Tr_pos);
plot(Time, Tr_vel);
plot(Time, Pos);
plot(Time, Vel);
legend('Tr-Pos','Tr-vel' ,'Pos','Vel')
xlabel('Time (ms)')
ylabel('Pulses - Pulses/s')