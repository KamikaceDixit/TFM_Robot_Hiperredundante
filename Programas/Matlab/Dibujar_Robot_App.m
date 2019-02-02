function [Grafico] = Dibujar_Robot_App(app,Puntos_Moved_Red)

%Los parámetros están definidos en la función de arranque de la app

 cla(app.UIAxes); %%Borrar los puntos anteriores
 %set(app.UIAxes,axis([-400 400 -400 400 0 800]));
%% Dibuja la nube de puntos de todos los discos

for i=1:16
Grafico=scatter3(app.UIAxes, Puntos_Moved_Red(:,1,i),Puntos_Moved_Red(:,2,i),Puntos_Moved_Red(:,3,i),4);
end


%% Crea las superficies entre los discos fijos
for i=2:2:14
X=[Puntos_Moved_Red(:,1,i)' Puntos_Moved_Red(1,1,i);Puntos_Moved_Red(:,1,i+1)' Puntos_Moved_Red(1,1,i+1)];
Y=[Puntos_Moved_Red(:,2,i)' Puntos_Moved_Red(1,2,i);Puntos_Moved_Red(:,2,i+1)' Puntos_Moved_Red(1,2,i+1)];
Z=[Puntos_Moved_Red(:,3,i)' Puntos_Moved_Red(1,3,i);Puntos_Moved_Red(:,3,i+1)' Puntos_Moved_Red(1,3,i+1)];
surf(app.UIAxes,X,Y,Z,'FaceColor',[1 0.84 0],'LineStyle','none');
end

%% Se representa con lineas los cables
for i=1:2:15
    for j=1:36
        line(app.UIAxes,[Puntos_Moved_Red(j,1,i) Puntos_Moved_Red(j,1,i+1)],[Puntos_Moved_Red(j,2,i) Puntos_Moved_Red(j,2,i+1)],[Puntos_Moved_Red(j,3,i) Puntos_Moved_Red(j,3,i+1)]);
    end
end

end


