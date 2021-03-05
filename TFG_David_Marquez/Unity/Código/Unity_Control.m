clc
clear all

% Parámetros del cliente TCPIP
TCPIP = tcpip('127.0.0.1',55001,'NetworkRole','Client');
set(TCPIP,'Timeout',0.0000001);
TCPIP.OutputBufferSize = 10000;
recibido = 0;

% Parámetros de control de Mach I
global Arduino Vmax AcelT DcelT alpha beta V_Tr Mot_I Time Output;
Mot_I = [ 1,2,4,5,7,8,10,11,13,14,16,17,19,20,22,23,25,26,28,29,31,32,34,35 ];
V_Tr = zeros(1,36);

Vmax = 1000; %Esto estaría bien recibirlo de Unity
AcelT = 0.4; %Esto estaría bien recibirlo de Unity
DcelT = 0.4; %Esto estaría bien recibirlo de Unity

conexion_ok = false;

%% Bucle de conexión TCPIP permanente con Unity.
while(1)
    fopen(TCPIP);
    msg = fgets(TCPIP);
    if isempty(msg) == 0
        msg = regexp(msg,';','split');
        key = msg(1);
        
        %% Conexión con Arduino
        if strcmp(key,'Arduino')
            %AQUÍ TROCEAR MSG (COM Y BAUDRATE) PARA OBTENER LOS VALORES
            msg = 0;
            delete(instrfindall);
            try
                Arduino = serial("COM16",'BaudRate',115200);  %El COM y el baudrate debería sacarlos de Unity
                set(Arduino,'DataBits',8);
                set(Arduino,'StopBits',1);
                set(Arduino,'Parity','none');
                set(Arduino,'Timeout',0.5);
                fopen(Arduino);
                conexion_ok = true;
                pause(2);
            catch
                conexion_ok = false;
                %MANDAR MENSAJE A UNITY DE ERROR DE CONEXIÓN
            end
        end
        
        %% Cálculo de trayectorias y envío al robot
        if strcmp(key,'mover') && conexion_ok == 1
            % Recepción de los valores de Alfa y Beta desde Unity
            msg = regexp(msg,';','split');
            alpha = msg(2:8);
            alpha = str2double(alpha);
            beta = msg(9:15);
            beta = str2double(beta);
            msg = 0;

            %%Se calculan las posiciones de todas las rotulas a partir de los angulos y se calculan los pulsos de destino.
            Puntos_Moved_Red = Calc_Posiciones(alpha,beta);
            Pulsos_Destino = Calc_Pulsos(Puntos_Moved_Red);

            %%Se solicita a la placa Main las posiciones actuales de todos los motores para poder calcular las trayectorias
            PulsesIni=Serial(Arduino,"Pos");
            disp(PulsesIni);

            %%Se analiza la trayectoria mas desfavorable para calcular el tiempo minimo necesario para la trayectoria.
            [M,I] = max(abs(Pulsos_Destino(Mot_I)-PulsesIni(Mot_I)));
            [Time,] = Calc_T_V(Vmax,AcelT,DcelT,PulsesIni(Mot_I(I)),Pulsos_Destino(Mot_I(I)),0,0,0,0);

            %%Se calculan las velocidades maximas de los trapecios de velocidad para cada motor.
            for i=1:24
                %V_Trs(Mot_I(i))=double(Calc_V(AcelT,DcelT,PulsesIni,Pulsos_Destino(Mot_I(i)),Time));
                V_Tr(Mot_I(i))=(2*PulsesIni(Mot_I(i)) - 2*Pulsos_Destino(Mot_I(i)))/(AcelT + DcelT - 2*Time);
            end

            %%Se envian a la placa Main la configuracion y las velocidades maximas de los trapecios.
            Serial(Arduino,"Conf",Time,'Speed',Vmax,'AcelT',AcelT,'DcelT',DcelT);
            Serial(Arduino,"Tr",V_Tr);

            disp(['Tiempo Trayectoria: ' num2str(Time)]);

            for i=1:20
                if Arduino.BytesAvailable
                    Output=fscanf(Arduino,'%s');
                    disp(['Trayectorias Enviadas Correctamente: ' Output]);
                    if Output=="OK"
                        %ENVIAR A UNITY UN OK PARA CAMBIAR ALGÚN BOTÓN O
                        %LUZ
                        break;
                    end
                else
                    pause(0.2);
                end
            end
            disp("FIN");
            
            % ESTE ES EL COMANDO DE INICIO OJITO CON ÉL
            Serial(Arduino,"Ini");

            
        elseif strcmp(key,'mover') && conexion_ok == 0
            % ERROR DE CONEXIÓN
            msg = 0;
        end
    end
    fclose(TCPIP);
end